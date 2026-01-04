#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>

/// Thin shader wrapper (file-based).
/// - Loads .vert/.frag files
/// - Compiles + links program
/// - Uniform setters with cached locations
class Shader
{
public:
    Shader() = default;
    Shader(const char* vertPath, const char* fragPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool IsValid() const { return program != 0; }
    void Use() const;

    void SetVec2(const char* name, float x, float y);
    void SetVec3(const char* name, float x, float y, float z);

private:
    GLuint program = 0;
    std::unordered_map<std::string, GLint> locCache;

    static std::string LoadTextFile(const char* path);
    static GLuint Compile(GLenum type, const char* src, const char* label);
    static bool PrintShaderLogIfFailed(GLuint shader, const char* label);
    static bool PrintProgramLogIfFailed(GLuint program);

    GLint Loc(const char* name);
    void Destroy();
};
