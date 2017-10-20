#pragma once

#include "shaders.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <vector>


namespace crudegl
{


namespace shaders
{


class program_link_error : public std::runtime_error
{
public:
    program_link_error(const std::string& traceback) : std::runtime_error(traceback)
    {
    }
};


class GLSLProgram
{
public:
    /**
    * Constructor
    * Create a new OpenGL program
    */
    GLSLProgram(): m_handle(0)
    {
        m_handle = glCreateProgram();
    }

    virtual ~GLSLProgram()
    {
        glDeleteProgram(m_handle);
    }

    // Move-only semantics
    GLSLProgram(const GLSLProgram&) = delete;
    GLSLProgram& operator=(const GLSLProgram&) = delete;
    
    GLSLProgram(GLSLProgram&&) = default;
    GLSLProgram& operator=(GLSLProgram&&) = default;

    /**
    * Attach the passed in shader to this program
    *
    * @param shader a valid / compiled OpenGL shader
    * @return whether the shader was successfully attached to the program or not
    */
    bool attach(GLuint shader)
    {
        glAttachShader(m_handle, shader);
        GLenum error = glGetError();
        return error != GL_INVALID_VALUE && error != GL_INVALID_OPERATION;
    }
    /**
    * Overloaded version of the above attach method for vertex shaders
    */
    bool attach(shaders::VertexShader& shader)
    {
        return attach(shader.get_handle());
    }
    /**
    * Overloaded version of the above attach method for geometry shaders
    */
    bool attach(shaders::GeometryShader& shader)
    {
        return attach(shader.get_handle());
    }
    /**
    * Overloaded version of the above attach method for fragment shaders
    */
    bool attach(shaders::FragmentShader& shader)
    {
        return attach(shader.get_handle());
    }
    /**
    * Attempt linking the attached shaders of the program
    */
    void link()
    {
        glLinkProgram(m_handle);
        GLint success = 0;
        glGetProgramiv(m_handle, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint infolog_size = 0;
            glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &infolog_size);

            std::vector<GLchar> infolog;
            infolog.resize(infolog_size);
            auto infolog_ptr = &infolog[0];

            glGetProgramInfoLog(m_handle, infolog_size, NULL, infolog_ptr);
            throw program_link_error(infolog_ptr);
        }
    }
    /**
    * Set the program as the currently active one
    */
    void use() const
    {
        if (m_handle)
        {
            glUseProgram(m_handle);
        }
    }
    /**
    * Retrieve the underlying OpenGL handle for this program
    *
    * @return GLuint reference
    */
    GLuint get_handle() const noexcept
    {
        return m_handle;
    }
    /**
    * Return the location of the passed in uniform variable name
    */
    GLuint get_uniform_location(const std::string& name) const noexcept
    {
        return glGetUniformLocation(m_handle, name.c_str());
    }
    /**
    * Bind data to the specified uniform variable
    */
    void set_uniform(const std::string& name, GLfloat v0)
    {
        glUniform1f(get_uniform_location(name), v0);
    }
    void set_uniform(const std::string& name, GLfloat v0, GLfloat v1)
    {
        glUniform2f(get_uniform_location(name), v0, v1);
    }
    void set_uniform(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
    {
        glUniform3f(get_uniform_location(name), v0, v1, v2);
    }
    void set_uniform(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
    {
        glUniform4f(get_uniform_location(name), v0, v1, v2, v3);
    }
    void set_uniform(const std::string& name, GLint v0)
    {
        glUniform1i(get_uniform_location(name), v0);
    }
    void set_uniform(const std::string& name, GLint v0, GLint v1)
    {
        glUniform2i(get_uniform_location(name), v0, v1);
    }
    void set_uniform(const std::string& name, GLint v0, GLint v1, GLint v2)
    {
        glUniform3i(get_uniform_location(name), v0, v1, v2);
    }
    void set_uniform(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
    {
        glUniform4i(get_uniform_location(name), v0, v1, v2, v3);
    }
    void set_uniform(const std::string& name, const std::vector<GLfloat>& values)
    {
        glUniform1fv(get_uniform_location(name), values.size(), glm::value_ptr(values));
    }
    void set_uniform(const std::string& name, const glm::vec2& value)
    {
        glUniform2fv(get_uniform_location(name), 1, glm::value_ptr(value));
    }
    void set_uniform(const std::string& name, const std::vector<glm::vec2>& values)
    {
        glUniform2fv(get_uniform_location(name), values.size(), glm::value_ptr(values.front()));
    }
    void set_uniform(const std::string& name, const glm::vec3& value)
    {
        glUniform3fv(get_uniform_location(name), 1, glm::value_ptr(value));
    }
    void set_uniform(const std::string& name, const std::vector<glm::vec3>& values)
    {
        glUniform3fv(get_uniform_location(name), values.size(), glm::value_ptr(values.front()));
    }
    void set_uniform(const std::string& name, const glm::vec4& value)
    {
        glUniform4fv(get_uniform_location(name), 1, glm::value_ptr(value));
    }
    void set_uniform(const std::string& name, const std::vector<glm::vec4>& values)
    {
        glUniform4fv(get_uniform_location(name), values.size(), glm::value_ptr(values.front()));
    }
    void set_uniform(const std::string& name, const std::vector<GLint>& values)
    {
        glUniform1iv(get_uniform_location(name), values.size(), glm::value_ptr(values));
    }
    void set_uniform(const std::string& name, const glm::ivec2& value)
    {
        glUniform2iv(get_uniform_location(name), 1, glm::value_ptr(value));
    }
    void set_uniform(const std::string& name, const std::vector<glm::ivec2>& values)
    {
        glUniform2iv(get_uniform_location(name), values.size(), glm::value_ptr(values.front()));
    }
    void set_uniform(const std::string& name, const glm::ivec3& value)
    {
        glUniform3iv(get_uniform_location(name), 1, glm::value_ptr(value));
    }
    void set_uniform(const std::string& name, const std::vector<glm::ivec3>& values)
    {
        glUniform3iv(get_uniform_location(name), values.size(), glm::value_ptr(values.front()));
    }
    void set_uniform(const std::string& name, const glm::ivec4& value)
    {
        glUniform4iv(get_uniform_location(name), 1, glm::value_ptr(value));
    }
    void set_uniform(const std::string& name, const std::vector<glm::ivec4>& values)
    {
        glUniform4iv(get_uniform_location(name), values.size(), glm::value_ptr(values.front()));
    }
    void set_uniform(const std::string& name, const glm::mat2& value, GLboolean transpose = GL_FALSE)
    {
        glUniformMatrix2fv(get_uniform_location(name), 1, transpose, glm::value_ptr(value));
    }
    void set_uniform(const std::string& name, const glm::mat3& value, GLboolean transpose = GL_FALSE)
    {
        glUniformMatrix3fv(get_uniform_location(name), 1, transpose, glm::value_ptr(value));
    }
    void set_uniform(const std::string& name, const glm::mat4& value, GLboolean transpose = GL_FALSE)
    {
        glUniformMatrix4fv(get_uniform_location(name), 1, transpose, glm::value_ptr(value));
    }
private:
    GLuint m_handle;
};


}  // namespace shaders


}  // namespace crudegl
