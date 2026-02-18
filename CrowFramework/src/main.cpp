#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm> // std::clamp

#include "gl2d/gl2d.h"
#include "engine/debug/openglErrorReporting.h"
#include "engine/graphics/Shader.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imguiThemes.h"

#pragma region CrowFramework_Config
static constexpr int kDefaultWidth = 640;
static constexpr int kDefaultHeight = 480;
static constexpr const char* kWindowTitle = "Breakout";
#pragma endregion

#pragma region Platform_Callbacks
static void error_callback(int error, const char* description)
{
    std::cout << "GLFW Error(" << error << "): " << description << "\n";
}
#pragma endregion

#pragma region Game_TemporaryData
static float rect[] = {
    -0.5f,-0.5f,0.0f,
     0.5f,-0.5f,0.0f,
     0.5f, 0.5f,0.0f,

    -0.5f,-0.5f,0.0f,
     0.5f, 0.5f,0.0f,
    -0.5f, 0.5f,0.0f
};
#pragma endregion

#pragma region Main
int main()
{
#pragma region Engine_Startup
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window =
        glfwCreateWindow(kDefaultWidth, kDefaultHeight, kWindowTitle, nullptr, nullptr);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
#pragma endregion

#pragma region Graphics_Startup
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    enableReportGlErrors();
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
#pragma endregion

#pragma region Resource_Loading
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);

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
#pragma endregion

#pragma region Game_Initialization
    float paddleX = 0.0f;
    const float paddleY = -0.88f;
    const float paddleW = 0.25f;
    const float paddleH = 0.06f;

    const float paddleSpeed = 1.6f;

    double lastTime = glfwGetTime();

	float ballX = 0.0f;
    float ballY = -0.2f;
    float ballSize = 0.04f;

    float ballVX = 0.7f;
	float ballVY = 1.0f;
#pragma endregion

#pragma region Main_Loop
    while (!glfwWindowShouldClose(window))
    {
#pragma region Frame_Begin
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        // delta time
        double now = glfwGetTime();
        float dt = static_cast<float>(now - lastTime);
        lastTime = now;

        if (dt > 0.05f) dt = 0.05f;
#pragma endregion

#pragma region Input_Update
        float dir = 0.0f;

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            dir -= 1.0f;
		}
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            dir += 1.0f;
        }
        paddleX += dir * paddleSpeed * dt;

        float halfPaddleW = paddleW / 2.0f;
		paddleX = std::clamp(paddleX, -1.0f + halfPaddleW, 1.0f - halfPaddleW);
#pragma endregion

#pragma region Game_Update

        float half = ballSize * 0.5f;

        // left/right
        if (ballX < -1.0f + half) { ballX = -1.0f + half; ballVX *= -1.0f; }
        if (ballX > 1.0f - half) { ballX = 1.0f - half; ballVX *= -1.0f; }

#pragma endregion

#pragma region World_Render
        glBindVertexArray(vao);

        // Paddle
        shader.Use();
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
#pragma endregion


#pragma region UI_Render
#pragma endregion

#pragma region Frame_End
        glfwSwapBuffers(window);
        glfwPollEvents();
#pragma endregion
    }
#pragma endregion

#pragma region Engine_Shutdown
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
#pragma endregion
}
#pragma endregion