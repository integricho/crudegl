#pragma once

#include <assimp/mesh.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <tuple>
#include <utility>


namespace crudegl
{


namespace models
{


namespace attributes
{


struct Position
{
    enum
    {
        size = 3,
        type = GL_FLOAT,
        normalized = GL_FALSE
    };
    glm::vec3 position;

    Position(aiMesh* mesh, GLuint index)
    {
        position.x = mesh->mVertices[index].x;
        position.y = mesh->mVertices[index].y;
        position.z = mesh->mVertices[index].z;
    }
};


struct Normal
{
    enum
    {
        size = 3,
        type = GL_FLOAT,
        normalized = GL_FALSE
    };
    glm::vec3 normal;

    Normal(aiMesh* mesh, GLuint index)
    {
        normal.x = mesh->mNormals[index].x;
        normal.y = mesh->mNormals[index].y;
        normal.z = mesh->mNormals[index].z;
    }
};


struct TextureCoordinate
{
    enum
    {
        size = 2,
        type = GL_FLOAT,
        normalized = GL_FALSE
    };
    glm::vec2 texture_coordinate;

    TextureCoordinate(aiMesh* mesh, GLuint index)
    {
        if (mesh->HasTextureCoords(0))
        {
            texture_coordinate.x = mesh->mTextureCoords[0][index].x;
            texture_coordinate.y = mesh->mTextureCoords[0][index].y;
        }
        else
        {
            texture_coordinate.x = 0.0f;
            texture_coordinate.y = 0.0f;
        }
    }
};


}  // namespace attributes


template <typename... Attrs>
struct Vertex : public Attrs...
{
    enum
    {
        attribute_count = sizeof...(Attrs)
    };
    // Get the vertex attribute type for the given `layout_position`
    template <std::size_t layout_position>
    using attribute = std::tuple_element_t<layout_position, std::tuple<Attrs...>>;
    /**
    * Constructor
    * Invoke each base class constructor, essentially delegating data
    * extraction to be performed by each one on it's own. Instead of having a
    * fixed order and number of vertex attributes, user-defined vertex types
    * can be used in any form, provided the user follows the attribute
    * definition conventions, so that attribute metadata can be extracted
    * from them.
    *
    * @param mesh is the assimp provided raw mesh
    * @param index is the vertex index within the mesh that should be processed
    */
    Vertex(aiMesh* mesh, GLuint index) : Attrs(mesh, index)...
    {
    }
};


template <class TVertex,
          template <class> class TInstaller,
          std::size_t... layout_position>
void add_each(std::size_t& offset, std::index_sequence<layout_position...>)
{
    using expander = int[];
    const std::size_t stride = sizeof(TVertex);
    // Instantiate and invoke vertex attributes, passing the stride, current
    // buffer offset and their respective layout_position to them.
    static_cast<void>(expander{0, (static_cast<void>(TInstaller<TVertex::attribute<layout_position>>()(stride, offset, layout_position)), 0)...});
}


/**
 * Prepare and start expansion of `TVertex`, instantiating each of it's
 * attributes and invoking them.
 */
template <class TVertex, template <class> class TInstaller>
void add_each()
{
    std::size_t offset = 0;
    add_each<TVertex, TInstaller>(offset, std::make_index_sequence<TVertex::attribute_count>{});
}


template <class TAttribute>
struct VertexAttributeInstaller
{
    /**
     * Set the vertex shader layout with vertex attribute pointers.
     *
     * Used in conjunction with the above defined `add_each` template. On each
     * invocation, it will create a vertex attribute pointer for the given
     * `layout_position`, using vextex attribute characteristics obtained from
     * the template parameter `TAttribute` that it was instantiated with.
     *
     * @param stride is the size of a single complete vertex type
     * @param offset indicates the beginning of the data within the buffer for
     *        the current parameter
     * @param layout_position is the vertex attribute position as defined in
     *        the vertex shader itself
     */
    void operator()(std::size_t stride, std::size_t& offset, std::size_t layout_position)
    {
        glVertexAttribPointer(layout_position,
                              TAttribute::size,
                              TAttribute::type,
                              TAttribute::normalized,
                              stride,
                              (GLvoid*)offset);
        glEnableVertexAttribArray(layout_position);
        const std::size_t data_size = TAttribute::size * sizeof(TAttribute::type);
        offset += data_size;
    }
};


using DefaultVertex = Vertex<attributes::Position,
                             attributes::Normal,
                             attributes::TextureCoordinate>;


}  // namespace models


}  //namespace crudegl
