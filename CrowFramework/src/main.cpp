#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "gl2d/gl2d.h"
#include "engine/debug/openglErrorReporting.h"
#include "engine/graphics/Shader.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imguiThemes.h"

#pragma region CrowFramework_Config
/// ============================================================================
/// CrowFramework Configuration
/// ----------------------------------------------------------------------------
/// - Global engine configuration values.
/// - Window size, title, debug flags, etc.
/// - Safe to edit.
/// ============================================================================

static constexpr int kDefaultWidth = 640;
static constexpr int kDefaultHeight = 480;
static constexpr const char* kWindowTitle = "CrowFramework Sandbox";
#pragma endregion

#pragma region Platform_Callbacks
/// ============================================================================
/// Platform Callbacks
/// ----------------------------------------------------------------------------
/// - Low-level platform callbacks (GLFW).
/// - You usually DO NOT need to modify this.
/// ============================================================================

static void error_callback(int error, const char* description)
{
    std::cout << "GLFW Error(" << error << "): " << description << "\n";
}
#pragma endregion

#pragma region Game_TemporaryData
/// ============================================================================
/// Temporary Game Data
/// ----------------------------------------------------------------------------
/// - Quick test data used during prototyping.
/// - Replace with Mesh / Resource system later.
/// ============================================================================

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
    /// ========================================================================
    /// Engine Startup
    /// ------------------------------------------------------------------------
    /// - Initializes the platform layer.
    /// - Creates the window and OpenGL context.
    /// - Do NOT put game logic here.
    /// ========================================================================
    
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
    /// ========================================================================
    /// Graphics Startup
    /// ------------------------------------------------------------------------
    /// - Loads OpenGL function pointers (GLAD).
    /// - Sets up debug and error reporting.
    /// ========================================================================
    
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
    /// ========================================================================
    /// Resource Loading
    /// ------------------------------------------------------------------------
    /// - Load shaders, meshes, textures, and other GPU resources here.
    /// - This section will later be replaced by a ResourceManager.
    /// ========================================================================

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
    /// ========================================================================
    /// Game Initialization
    /// ------------------------------------------------------------------------
    /// - Initialize game state here.
    /// - Player, enemies, levels, scores, etc.
    /// ========================================================================
    
#pragma endregion

#pragma region Main_Loop
    /// ========================================================================
    /// Main Game Loop
    /// ------------------------------------------------------------------------
    /// - Runs until the window is closed.
    /// - Update ¡æ Render ¡æ Present
    /// ========================================================================
    
    while (!glfwWindowShouldClose(window))
    {
#pragma region Frame_Begin
        /// Per-frame setup (viewport, clear, timing).
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
#pragma endregion

#pragma region Input_Update
        /// --------------------------------------------------------------------
        /// Input Update
        /// - Handle keyboard / mouse input here.
        /// --------------------------------------------------------------------
        
#pragma endregion

#pragma region Game_Update
        /// --------------------------------------------------------------------
        /// Game Update
        /// - Update game logic.
        /// - Movement, collision, AI, scoring, etc.
        /// --------------------------------------------------------------------
        
#pragma endregion

#pragma region World_Render
        /// --------------------------------------------------------------------
        /// World Rendering
        /// - Draw game objects here.
        /// - Do NOT update game logic in this section.
        /// --------------------------------------------------------------------
        
        shader.Use();
        shader.SetVec3("uColor", 0.0f, 1.0f, 0.0f);
        shader.SetVec2("uScale", 0.5f, 0.5f);
        shader.SetVec2("uOffset", 0.0f, 0.0f);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
#pragma endregion

#pragma region UI_Render
        /// --------------------------------------------------------------------
        /// UI Rendering
        /// - ImGui or HUD rendering goes here.
        /// --------------------------------------------------------------------
        
#pragma endregion

#pragma region Frame_End
        /// Present the frame and poll events.
        glfwSwapBuffers(window);
        glfwPollEvents();
#pragma endregion
    }
#pragma endregion

#pragma region Engine_Shutdown
    /// ========================================================================
    /// Engine Shutdown
    /// ------------------------------------------------------------------------
    /// - Clean up all resources.
    /// - Called once before application exit.
    /// ========================================================================
    
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
#pragma endregion
}
#pragma endregion
