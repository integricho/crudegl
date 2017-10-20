#pragma once

#include "utils.h"

#include <glad/glad.h>
#include <SOIL.h>


namespace crudegl
{


namespace textures
{


class Texture2D
{
public:
    /**
    * Constructor
    * Create a new texture
    *
    * @param path is an absolute path to the texture image file
    * @param name under which the texture is referenced in shaders
    * @param min_filter is a texture filter value
    * @param mag_filter is a texture filter value
    * @param wrap_s is a texture parameter value
    * @param wrap_t is a texture parameter value
    * @param generate_mipmap indicates whether to generate mipmap or not
    */
    Texture2D(const std::string& path,
              const std::string& name = "",
              GLenum min_filter = GL_NEAREST_MIPMAP_LINEAR,
              GLenum mag_filter = GL_LINEAR,
              GLenum wrap_s = GL_REPEAT,
              GLenum wrap_t = GL_REPEAT,
              bool generate_mipmap = true): m_path{path},
                                            m_name{name},
                                            m_min_filter{min_filter},
                                            m_mag_filter{mag_filter},
                                            m_wrap_s{wrap_s},
                                            m_wrap_t{wrap_t},
                                            m_generate_mipmap{m_generate_mipmap},
                                            m_handle{0}
    {
        if (m_name.empty())
        {
            m_name = utils::fs::noextension(utils::fs::basename(path));
        }
    }

    virtual ~Texture2D() noexcept
    {
        glDeleteTextures(1, &m_handle);
    }

    // Move-only semantics
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& rhs) = default;
    Texture2D& operator=(Texture2D&&) = default;

    /**
    * Load the texture data into the currently active texture unit
    */
    void load()
    {
        glGenTextures(1, &m_handle);
        glBindTexture(GL_TEXTURE_2D, m_handle);
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap_t);
        // Set texture filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
        // Load texture data
        int width, height;
        unsigned char* bytes = SOIL_load_image(m_path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);
        if (m_generate_mipmap)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        SOIL_free_image_data(bytes);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    /**
    * Bind the texture to the specified texture unit
    *
    * @param unit specifies to which texture unit to bind
    */
    void bind(GLenum unit) const noexcept
    {
        glActiveTexture(unit);
        glBindTexture(GL_TEXTURE_2D, m_handle);
    }
    /**
    * Unbind the texture
    *
    * @param unit specifies from which texture unit to unbind
    */
    void unbind(GLenum unit) const noexcept
    {
        glActiveTexture(unit);
        glBindTexture(GL_TEXTURE_2D, 0);
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
    * Return the identifier by which the texture is referenced in the shaders
    *
    * @return unique string identifier
    */
    std::string get_name() const noexcept
    {
        return m_name;
    }
private:
    std::string m_name;
    std::string m_path;
    GLenum m_min_filter;
    GLenum m_mag_filter;
    GLenum m_wrap_s;
    GLenum m_wrap_t;
    bool m_generate_mipmap;

    GLuint m_handle;
};


}  // namespace textures


}  // namespace crudegl
