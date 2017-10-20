#pragma once

#include <glad/glad.h>

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace crudegl
{


namespace shaders
{


class shader_load_error : public std::runtime_error
{
public:
    shader_load_error(const std::string& path) : path_(path),
                                                 std::runtime_error("Error loading shader source file: " + path)
    {
    }
    const std::string getpath() const
    {
        return path_;
    }
private:
    const std::string path_;
};


class shader_compile_error : public std::runtime_error
{
public:
    shader_compile_error(const std::string& traceback) : std::runtime_error(traceback)
    {
    }
};


template <GLenum type>
class Shader
{
public:
    friend class GLSLProgram;
    enum
    {
        type = type
    };
    /**
    * Constructor
    * Load the given shader path and attempt to compile it
    *
    * @param path an absolute path to the shader file
    */
    Shader(const std::string& path): m_path(path),
                                     m_data(),
                                     m_handle(0)
    {
        load_source();
        compile();
    }

    virtual ~Shader()
    {
        glDeleteShader(m_handle);
    }

    // Move-only semantics
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&&) = default;
    Shader& operator=(Shader&&) = default;

private:
    /**
    * Retrieve the underlying OpenGL handle for this shader
    *
    * @return GLuint reference
    */
    GLuint get_handle() const noexcept
    {
        return m_handle;
    }
    /**
    * Load the shader source file and read it's contents
    */
    void load_source()
    {
        std::ifstream fptr;
        fptr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            fptr.open(m_path);
            m_data.assign(std::istreambuf_iterator<char>(fptr),
                         std::istreambuf_iterator<char>());
        }
        catch (const std::ifstream::failure&)
        {
            if (fptr.is_open())
            {
                fptr.close();
            }
            throw shader_load_error(m_path);
        }
        fptr.close();
    }
    /**
    * Compile the loaded shader source data
    */
    void compile()
    {
        m_handle = glCreateShader(type);
        const char* source = m_data.c_str();
        glShaderSource(m_handle, 1, &source, NULL);
        glCompileShader(m_handle);
        // Check if compilation succeeded
        GLint success = 0;
        glGetShaderiv(m_handle, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLint infolog_size = 0;
            glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &infolog_size);

            std::vector<GLchar> infolog;
            infolog.resize(infolog_size);
            auto infolog_ptr = &infolog[0];

            glGetShaderInfoLog(m_handle, infolog_size, NULL, infolog_ptr);
            throw shader_compile_error(infolog_ptr);
        }
    }
private:
    const std::string m_path;
    std::string m_data;
    GLuint m_handle;
};


using VertexShader = Shader<GL_VERTEX_SHADER>;
using GeometryShader = Shader<GL_GEOMETRY_SHADER>;
using FragmentShader = Shader<GL_FRAGMENT_SHADER>;


}  // namespace shaders


}  // namespace crudegl
