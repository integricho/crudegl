#pragma once

#include "meshes.h"
#include "programs.h"
#include "textures.h"
#include "vertices.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>


namespace crudegl
{


namespace models
{


class model_error : public std::runtime_error
{
public:
    model_error(const std::string& path,
                const std::string& message) : path_{path},
                                              std::runtime_error(message)
    {
    }
    const std::string getpath() const
    {
        return path_;
    }
private:
    const std::string path_;
};


class Model
{
public:
    using program_type = shaders::GLSLProgram;

    Model() = default;

    virtual ~Model() = default;

    // Move-only semantics
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    Model(Model&&) = default;
    Model& operator=(Model&&) = default;

    /**
    * Load the model data in case it's not already loaded
    */
    virtual void load() = 0;
    /**
    * Render the current model
    * @param program is a compiled and linked OpenGL program with shaders
    */
    virtual void render(program_type& program) const = 0;
};


template <class TVertexData = GLfloat,
          class TVertexLayout = DefaultVertex,
          class TTexture = textures::Texture2D>
class RawModel : public Model
{
public:
    using vertex_data_type = TVertexData;
    using vertex_layout = TVertexLayout;
    using texture_type = TTexture;
    using texture_vec = std::vector<std::shared_ptr<texture_type>>;
    using mesh_type = Mesh<vertex_data_type, vertex_layout, texture_type, program_type>;
    /**
    * Constructor
    * Create a model instance from raw data
    * @param path is an absolute path to a model file
    */
    RawModel(const std::vector<vertex_data_type>& vertices,
             const std::vector<GLuint>& indices,
             const std::vector<std::string>& texture_paths)
    {
        texture_vec textures;
        for (const auto& path : texture_paths)
        {
            auto instance = std::make_shared<texture_type>(path);
            instance->load();
            textures.push_back(instance);
        }
        m_meshes.emplace_back(vertices, indices, std::move(textures));
    }
    /**
    * Load the model data in case it's not already loaded
    */
    virtual void load() override
    {
    }
    /**
    * Render the current model
    * @param program is a compiled and linked OpenGL program with shaders
    */
    void render(program_type& program) const override
    {
        for (const auto& mesh : m_meshes)
        {
            mesh.render(program);
        }
    }
private:
    std::vector<mesh_type> m_meshes;
    std::unordered_map<std::string, std::shared_ptr<texture_type>> m_loaded_textures;
};


template <class TVertexData = DefaultVertex,
          class TVertexLayout = DefaultVertex,
          class TTexture = textures::Texture2D>
class AssetModel : public Model
{
public:
    using vertex_data_type = TVertexData;
    using vertex_layout = TVertexLayout;
    using texture_type = TTexture;
    using texture_vec = std::vector<std::shared_ptr<texture_type>>;
    using mesh_type = Mesh<vertex_data_type, vertex_layout, texture_type, program_type>;
    /**
    * Constructor
    * Create a model instance and load all of it's resources
    * @param path is an absolute path to a model file
    */
    explicit AssetModel(const std::string& path) : m_path{path},
                                                   m_parentdir{utils::fs::dirname(path)},
                                                   m_loaded{false}
    {
    }
    /**
    * Load the model data in case it's not already loaded
    */
    void load() override
    {
        if (!m_loaded)
        {
            load_model();
            m_loaded = true;
        }
    }
    /**
    * Render the current model
    * @param program is a compiled and linked OpenGL program with shaders
    */
    void render(program_type& program) const override
    {
        if (!m_loaded)
        {
            throw model_error(m_path, "Model not loaded before rendering.");
        }
        for (const auto& mesh : m_meshes)
        {
            mesh.render(program);
        }
    }
private:
    /**
    * Load model file and initiate recursive model processing
    */
    void load_model()
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(m_path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw model_error(m_path, "Cannot load model.");
        }
        process_node(scene->mRootNode, scene);
    }
    /**
    * Recursively process each mesh within the model
    * @param node is the current node within the scene data structure
    * @param scene is the model / scene containing all the meshes
    */
    void process_node(aiNode* node, const aiScene* scene)
    {
        // Process all meshes of the current node
        for (GLuint i = 0; i < node->mNumMeshes; ++i)
        {
            GLuint mesh_index = node->mMeshes[i];
            aiMesh* raw_mesh = scene->mMeshes[mesh_index];
            m_meshes.push_back(process_mesh(raw_mesh, scene));
        }
        // Process all children of the current mesh
        for (GLuint i = 0; i < node->mNumChildren; ++i)
        {
            process_node(node->mChildren[i], scene);
        }
    }
    /**
    * Create a new `Mesh` instance from a raw mesh and load all it's resources
    * @param raw_mesh is the raw assimp data structure describing the mesh
    * @param scene is the model / scene containing all the meshes
    */
    mesh_type process_mesh(aiMesh* raw_mesh, const aiScene* scene)
    {
        auto vertices = collect_vertices(raw_mesh);
        auto indices = collect_indices(raw_mesh);
        auto textures = collect_textures(raw_mesh, scene);
        return mesh_type(vertices, indices, std::move(textures));
    }
    /**
    * Collect and return all vertices from the passed in mesh
    * @param raw_mesh is the raw assimp data structure describing the mesh
    */
    std::vector<vertex_data_type> collect_vertices(aiMesh* raw_mesh)
    {
        std::vector<vertex_data_type> vertices;
        for (GLuint i = 0; i < raw_mesh->mNumVertices; ++i)
        {
            vertices.emplace_back(raw_mesh, i);
        }
        return vertices;
    }
    /**
    * Collect and return all indices of all faces from the passed in mesh
    * @param raw_mesh is the raw assimp data structure describing the mesh
    */
    std::vector<GLuint> collect_indices(aiMesh* raw_mesh)
    {
        std::vector<GLuint> indices;
        for (GLuint i = 0; i < raw_mesh->mNumFaces; ++i)
        {
            aiFace face = raw_mesh->mFaces[i];
            for (GLuint j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }
        return indices;
    }
    /**
    * Collect and return all textures from the passed in mesh
    * @param raw_mesh is the raw assimp data structure describing the mesh
    * @param scene is the model / scene containing all the meshes
    */
    texture_vec collect_textures(aiMesh* raw_mesh, const aiScene* scene)
    {
        texture_vec textures;
        if (raw_mesh->mMaterialIndex > 0)
        {
            aiMaterial* material = scene->mMaterials[raw_mesh->mMaterialIndex];
            load_textures(material, aiTextureType_DIFFUSE, textures);
            load_textures(material, aiTextureType_SPECULAR, textures);
        }
        return textures;
    }
    /**
    * Load all the requested texture types from the passed in material into
    * the given vector
    * @param material is the source material from which to load textures
    * @param type is the type of textures to load from the material
    * @param textures is the vector into which textures are inserted
    */
    void load_textures(aiMaterial* material, aiTextureType type, texture_vec& textures)
    {
        for (GLuint i = 0; i < material->GetTextureCount(type); ++i)
        {
            aiString path;
            material->GetTexture(type, i, &path);
            auto found = m_loaded_textures.find(path.C_Str());
            if (found == m_loaded_textures.end())
            {
                // Not yet loaded, load texture
                auto instance = std::make_shared<texture_type>(utils::fs::join(m_parentdir, path.C_Str()));
                instance->load();
                textures.push_back(instance);
                m_loaded_textures.insert({path.C_Str(), instance});
            }
            else
            {
                // If already loaded, use the existing instance
                textures.push_back(found->second);
            }
        }
    }
private:
    bool m_loaded;
    std::string m_path;
    std::string m_parentdir;
    std::vector<mesh_type> m_meshes;
    std::unordered_map<std::string, std::shared_ptr<texture_type>> m_loaded_textures;
};


}  // namespace models


}  // namespace crudegl
