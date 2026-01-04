#include "engine/graphics/Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

std::string Shader::LoadTextFile(const char* path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "Failed to open: " << path << "\n";
        return {};
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool Shader::PrintShaderLogIfFailed(GLuint shader, const char* label)
{
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok) return true;

    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    std::string log((size_t)len, '\0');
    glGetShaderInfoLog(shader, len, nullptr, log.data());

    std::cout << "[Shader Compile Failed] " << label << "\n" << log << "\n";
    return false;
}

bool Shader::PrintProgramLogIfFailed(GLuint p)
{
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (ok) return true;

    GLint len = 0;
    glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
    std::string log((size_t)len, '\0');
    glGetProgramInfoLog(p, len, nullptr, log.data());

    std::cout << "[Program Link Failed]\n" << log << "\n";
    return false;
}

GLuint Shader::Compile(GLenum type, const char* src, const char* label)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    if (!PrintShaderLogIfFailed(s, label))
    {
        glDeleteShader(s);
        return 0;
    }
    return s;
}

Shader::Shader(const char* vertPath, const char* fragPath)
{
    std::string vertSrc = LoadTextFile(vertPath);
    std::string fragSrc = LoadTextFile(fragPath);

    if (vertSrc.empty() || fragSrc.empty())
    {
        std::cout << "Shader source empty. Check paths / working directory.\n";
        return;
    }

    const char* vsrc = vertSrc.c_str();
    const char* fsrc = fragSrc.c_str();

    GLuint vs = Compile(GL_VERTEX_SHADER, vsrc, "VERTEX");
    GLuint fs = Compile(GL_FRAGMENT_SHADER, fsrc, "FRAGMENT");
    if (!vs || !fs)
    {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return;
    }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!PrintProgramLogIfFailed(program))
    {
        glDeleteProgram(program);
        program = 0;
    }
}

Shader::~Shader()
{
    Destroy();
}

Shader::Shader(Shader&& other) noexcept
{
    program = other.program;
    locCache = std::move(other.locCache);
    other.program = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this == &other) return *this;
    Destroy();
    program = other.program;
    locCache = std::move(other.locCache);
    other.program = 0;
    return *this;
}

void Shader::Destroy()
{
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }
    locCache.clear();
}

void Shader::Use() const
{
    glUseProgram(program);
}

GLint Shader::Loc(const char* name)
{
    auto it = locCache.find(name);
    if (it != locCache.end()) return it->second;

    GLint loc = glGetUniformLocation(program, name);
    locCache.emplace(name, loc);
    return loc;
}

void Shader::SetVec2(const char* name, float x, float y)
{
    GLint loc = Loc(name);
    if (loc != -1) glUniform2f(loc, x, y);
}

void Shader::SetVec3(const char* name, float x, float y, float z)
{
    GLint loc = Loc(name);
    if (loc != -1) glUniform3f(loc, x, y, z);
}
