#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>

#include "engine/debug/openglErrorReporting.h"
#include "engine/graphics/Shader.h"

static constexpr int kDefaultWidth = 640;
static constexpr int kDefaultHeight = 480;
static constexpr const char* kWindowTitle = "Breakout";

static void error_callback(int error, const char* description)
{
    std::cout << "GLFW Error(" << error << "): " << description << "\n";
}

// Unit rect centered at origin (NDC-space transform via uScale/uOffset in vertex shader)
static float rectVerts[] = {
    -0.5f,-0.5f,0.0f,
     0.5f,-0.5f,0.0f,
     0.5f, 0.5f,0.0f,
    -0.5f,-0.5f,0.0f,
     0.5f, 0.5f,0.0f,
    -0.5f, 0.5f,0.0f
};

struct Brick
{
    float x, y;
    float w, h;
    float r, g, b;
    bool destroyed = false;
};

static void DrawRect(GLuint vao, Shader& shader,
    float x, float y, float w, float h,
    float r, float g, float b)
{
    shader.Use();
    shader.SetVec3("uColor", r, g, b);
    shader.SetVec2("uScale", w, h);
    shader.SetVec2("uOffset", x, y);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

static void BuildBricks(std::vector<Brick>& bricks,
    float playX, float playY, float playW, float playH)
{
    bricks.clear();

    const int cols = 14;
    const int rowsPerColor = 2;
    const int bands = 4;
    const int rows = rowsPerColor * bands; // 8 rows

    // playfield bounds
    const float left = playX - playW * 0.5f;
    const float right = playX + playW * 0.5f;
    const float top = playY + playH * 0.5f;

    // Atari-ish: bricks start below top, leaving a top "score band" area
    const float scoreBandH = 0.18f;
    const float bricksTop = top - scoreBandH;

    // Make bricks fill width nicely: small side margin, tiny gaps
    const float marginX = 0.02f;
    const float marginTop = 0.06f;

    const float gapX = 0.006f;
    const float gapY = 0.012f;

    const float areaW = (right - left) - marginX * 2.0f;
    const float areaH = 0.42f; // tweak: taller brick field

    const float brickW = (areaW - gapX * (cols - 1)) / cols;
    const float brickH = (areaH - gapY * (rows - 1)) / rows;

    const float startX = left + marginX + brickW * 0.5f;
    const float startY = bricksTop - marginTop - brickH * 0.5f;

    // Colors (Atari-ish order from TOP: red/orange/green/yellow)
    const float colors[bands][3] = {
        { 0.86f, 0.10f, 0.10f }, // red
        { 0.92f, 0.55f, 0.10f }, // orange
        { 0.10f, 0.70f, 0.20f }, // green
        { 0.90f, 0.85f, 0.15f }, // yellow
    };

    bricks.reserve(cols * rows);

    for (int r = 0; r < rows; ++r)
    {
        const int band = r / rowsPerColor; // 0..3
        const float rr = colors[band][0];
        const float gg = colors[band][1];
        const float bb = colors[band][2];

        const float y = startY - r * (brickH + gapY);

        for (int c = 0; c < cols; ++c)
        {
            const float x = startX + c * (brickW + gapX);

            Brick b;
            b.x = x; b.y = y;
            b.w = brickW; b.h = brickH;
            b.r = rr; b.g = gg; b.b = bb;
            b.destroyed = false;
            bricks.push_back(b);
        }
    }
}

// returns true if hit; also resolves by pushing ball out + reflecting on minimal penetration axis
static bool BallVsAABB(float& ballX, float& ballY, float ballHalf,
    float& ballVX, float& ballVY,
    const Brick& box)
{
    const float halfW = box.w * 0.5f;
    const float halfH = box.h * 0.5f;

    const bool overlapX = (ballX + ballHalf) >= (box.x - halfW) &&
        (ballX - ballHalf) <= (box.x + halfW);
    const bool overlapY = (ballY + ballHalf) >= (box.y - halfH) &&
        (ballY - ballHalf) <= (box.y + halfH);

    if (!(overlapX && overlapY)) return false;

    // penetration
    const float dx = ballX - box.x;
    const float px = (halfW + ballHalf) - std::abs(dx);

    const float dy = ballY - box.y;
    const float py = (halfH + ballHalf) - std::abs(dy);

    if (px < py)
    {
        // resolve X
        ballVX *= -1.0f;
        ballX += (dx > 0.0f) ? px : -px;
    }
    else
    {
        // resolve Y
        ballVY *= -1.0f;
        ballY += (dy > 0.0f) ? py : -py;
    }
    return true;
}

int main()
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(kDefaultWidth, kDefaultHeight, kWindowTitle, nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    enableReportGlErrors();
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // Geometry
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVerts), rectVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    Shader shader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    if (!shader.IsValid())
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // ===== Game constants =====
    // White background + slightly inset black playfield (thin white "wall")
    const float playW = 1.94f;
    const float playH = 1.98f;
    const float playX = 0.0f;
    const float playY = -0.01f;

    const float leftWall = playX - playW * 0.5f;
    const float rightWall = playX + playW * 0.5f;
    const float topWall = playY + playH * 0.5f;

    // Paddle
    float paddleX = 0.0f;
    const float paddleY = -0.88f;
    const float paddleW = 0.25f;
    const float paddleH = 0.06f;
    const float paddleSpeed = 1.6f;

    // Ball
    float ballX = 0.0f;
    float ballY = -0.2f;
    float ballSize = 0.04f;
    float ballVX = 0.7f;
    float ballVY = 1.0f;

    // Bricks
    std::vector<Brick> bricks;
    BuildBricks(bricks, playX, playY, playW, playH);

    int score = 0;

    auto UpdateTitle = [&]()
        {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "Breakout  |  Score: %d  |  Bricks: %d",
                score, (int)bricks.size());
            glfwSetWindowTitle(window, buf);
        };
    UpdateTitle();

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        // ----- frame begin -----
        int fbw = 0, fbh = 0;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        glViewport(0, 0, fbw, fbh);
        glClear(GL_COLOR_BUFFER_BIT);

        const double now = glfwGetTime();
        float dt = (float)(now - lastTime);
        lastTime = now;
        if (dt > 0.05f) dt = 0.05f;

        // ----- input -----
        float dir = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)  dir -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir += 1.0f;

        paddleX += dir * paddleSpeed * dt;

        const float halfPW = paddleW * 0.5f;
        paddleX = std::clamp(paddleX, leftWall + halfPW, rightWall - halfPW);

        // ----- update -----
        const float halfBall = ballSize * 0.5f;

        // move
        ballX += ballVX * dt;
        ballY += ballVY * dt;

        // walls (no bottom wall)
        if (ballY + halfBall > topWall) { ballY = topWall - halfBall;   ballVY *= -1.0f; }
        if (ballX - halfBall < leftWall) { ballX = leftWall + halfBall;  ballVX *= -1.0f; }
        if (ballX + halfBall > rightWall) { ballX = rightWall - halfBall; ballVX *= -1.0f; }

        // paddle AABB (only when falling)
        const float halfPH = paddleH * 0.5f;
        const bool pOverlapX = (ballX + halfBall) >= (paddleX - halfPW) &&
            (ballX - halfBall) <= (paddleX + halfPW);
        const bool pOverlapY = (ballY - halfBall) <= (paddleY + halfPH) &&
            (ballY + halfBall) >= (paddleY - halfPH);

        if (pOverlapX && pOverlapY && ballVY < 0.0f)
        {
            // snap to top of paddle to avoid "sticky gap" feeling
            ballY = paddleY + halfPH + halfBall;

            ballVY *= -1.0f;

            // angle control (Atari-ish)
            float offset = (ballX - paddleX) / halfPW;  // -1..1
            offset = std::clamp(offset, -1.0f, 1.0f);

            ballVX = offset * 1.2f;

            // prevent too-straight vertical
            if (std::abs(ballVX) < 0.2f) ballVX = (ballVX < 0.0f) ? -0.2f : 0.2f;
        }

        // brick collision: first hit only per frame (simple + stable)
        bool hitBrick = false;
        for (auto& b : bricks)
        {
            if (b.destroyed) continue;

            if (BallVsAABB(ballX, ballY, halfBall, ballVX, ballVY, b))
            {
                b.destroyed = true;
                score += 10;
                hitBrick = true;
                break;
            }
        }

        if (hitBrick)
        {
            bricks.erase(std::remove_if(bricks.begin(), bricks.end(),
                [](const Brick& b) { return b.destroyed; }),
                bricks.end());
            UpdateTitle();
        }

        // reset if ball falls below screen
        if (ballY < -1.0f - halfBall)
        {
            ballX = 0.0f;
            ballY = -0.2f;
            ballVX = 0.7f;
            ballVY = 1.0f;
        }

        // ----- render -----
        glBindVertexArray(vao);

        // Background (white)
        shader.Use();
        shader.SetVec3("uColor", 0.95f, 0.95f, 0.95f);
        shader.SetVec2("uScale", 2.0f, 2.0f);
        shader.SetVec2("uOffset", 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Playfield (black)
        shader.SetVec3("uColor", 0.02f, 0.02f, 0.02f);
        shader.SetVec2("uScale", playW, playH);
        shader.SetVec2("uOffset", playX, playY);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Bricks (render from vector so destruction works)
        shader.Use();
        for (const auto& b : bricks)
        {
            shader.SetVec3("uColor", b.r, b.g, b.b);
            shader.SetVec2("uScale", b.w, b.h);
            shader.SetVec2("uOffset", b.x, b.y);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Paddle
        shader.SetVec3("uColor", 0.20f, 0.70f, 1.00f);
        shader.SetVec2("uScale", paddleW, paddleH);
        shader.SetVec2("uOffset", paddleX, paddleY);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Ball
        shader.SetVec3("uColor", 1.0f, 1.0f, 1.0f);
        shader.SetVec2("uScale", ballSize, ballSize);
        shader.SetVec2("uOffset", ballX, ballY);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);

        // ----- end frame -----
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
