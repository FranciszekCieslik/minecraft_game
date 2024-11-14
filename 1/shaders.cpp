#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

GLuint CreateShader(const std::string &shaderSource, GLenum shaderType)
{
    const GLuint shaderId = glCreateShader(shaderType);
    if (!shaderId)
    {
        std::cerr << "Error creating shader!" << std::endl;
        return 0;
    }
    const char *source = shaderSource.c_str();
    glShaderSource(shaderId, 1, &source, nullptr);
    glCompileShader(shaderId);

    GLint success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shaderId, 512, nullptr, infoLog);
        std::cerr << "Error compiling shader: " << infoLog << std::endl;
    }
    return shaderId;
}

GLuint CreateProgram(GLuint vertexShader, GLuint fragmentShader, GLuint geometryShader = 0)
{
    const GLuint programId = glCreateProgram();
    if (!programId)
    {
        std::cerr << "Error creating program!" << std::endl;
        return 0;
    }
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);
    if (geometryShader)
        glAttachShader(programId, geometryShader);
    glLinkProgram(programId);

    GLint success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        std::cerr << "Error linking program: " << infoLog << std::endl;
    }

    glDetachShader(programId, vertexShader);
    glDetachShader(programId, fragmentShader);
    if (geometryShader) glDetachShader(programId, geometryShader);

    return programId;
}

std::pair<GLuint, GLuint> CreateVertexBufferObject()
{
    const float triangle[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return std::make_pair(vbo, vao);
}

std::string LoadShaderSource(const char* filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}