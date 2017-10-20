#pragma once

#include "programs.h"
#include "shaders.h"
#include "textures.h"
#include "vertices.h"

#include <glad/glad.h>

#include <memory>
#include <tuple>
#include <vector>


namespace crudegl
{


namespace models
{


template <class TVertexData = DefaultVertex,
          class TVertexLayout = DefaultVertex,
          class TTexture = textures::Texture2D,
          class TProgram = shaders::GLSLProgram>
class Mesh
{
public:
    using vertex_data_type = TVertexData;
    using vertex_layout = TVertexLayout;
    using texture_type = TTexture;
    using texture_vec = std::vector<std::shared_ptr<texture_type>>;
    using program_type = TProgram;
    /**
    * Constructor
    * Create a mesh instance
    * @param vertices is a list of vertices
    * @param indices is a list of indices used for indexed draw
    * @param textures is a list of loaded texture instances
    */
    Mesh(const std::vector<vertex_data_type>& vertices,
         const std::vector<GLuint>& indices,
         texture_vec&& textures) : m_vao(0),
                                   m_vbo(0),
                                   m_ebo(0),
                                   m_vertex_count(vertices.size()),
                                   m_index_count(indices.size()),
                                   m_textures(std::move(textures))
    {
        // Create and bind vertex array object
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        // Create and bind vertex buffer object
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertex_count * sizeof(vertex_data_type), vertices.data(), GL_STATIC_DRAW);

        // Create and bind element buffer object
        if (m_index_count > 0)
        {
            glGenBuffers(1, &m_ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_index_count * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
        }

        // Set vertex attributes
        add_each<vertex_layout, VertexAttributeInstaller>();

        // After vertex attributes are bound, unbind the vertex array object
        // to avoid other code accidentally stepping over the setup
        glBindVertexArray(0);
    }

    virtual ~Mesh() = default;

    // Move-only semantics
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;

    void render(program_type& program) const
    {
        bind_textures(program);
        draw_mesh();
        // Unbind all textures to avoid accidents
        unbind_textures();
    }
private:
    void bind_textures(program_type& program) const
    {
        for (decltype(m_textures.size()) i = 0; i < m_textures.size(); ++i)
        {
            const auto& texture = m_textures[i];
            texture->bind(GL_TEXTURE0 + i);
            program.set_uniform(texture->get_name(), static_cast<GLint>(i));
        }
    }
    void draw_mesh() const
    {
        glBindVertexArray(m_vao);
        if (m_index_count > 0)
        {
            glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, m_vertex_count);
        }
        glBindVertexArray(0);
    }
    void unbind_textures() const
    {
        for (decltype(m_textures.size()) i = 0; i < m_textures.size(); ++i)
        {
            const auto& texture = m_textures[i];
            texture->unbind(GL_TEXTURE0 + i);
        }
    }
private:
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;

    std::size_t m_vertex_count;
    std::size_t m_index_count;
    texture_vec m_textures;
};


}  // namespace models


}  // namespace crudegl
