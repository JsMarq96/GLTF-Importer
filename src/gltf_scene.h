#ifndef GLTF_SCENE_H_
#define GLTF_SCENE_H_

#include "gl3w.h"
#include "glcorearb.h"
#include "shader.h"
#include "raw_shaders.h"
#include "material.h"
#include "camera.h"

#include "tiny_gltf.h"
#include <cstdint>
#include <stdint.h>
#include <string.h>
#include <cassert>

#define MAX_MATERIAL_COUNT 2000
#define MAX_MESH_COUNT 2000
#define MAX_SUBMESH_COUNT 3000
#define MAX_NODE_COUNT 2000

enum eVBO : uint8_t {
    VERTEX_BUFFER = 0,
    NORMAL_BUFFER,
    UV_BUFFER,
    MAX_VBO_COUNT
};

struct sSubMesh {
    bool      used_VBOs[MAX_VBO_COUNT] = { false, false, false };
    uint32_t  VBOs[MAX_VBO_COUNT] = { 0 };
    uint32_t  EBO = 0;

    void rendering_bind();
    void render();
    void rendering_unbind();
    void clean();
};

// Tuple type, for child meshes
struct sSubMeshChild {
    bool      has_child = false;
    uint16_t  child_index = 0;
};

struct sMesh {
    uint32_t VAO = 0;
    uint16_t first_submesh = 0;
};
 /**
  * Scene's structure:
  * A scene is made out of nodes, and each node has:
  *  -A transformation
  *  -A mesh
  *
  * And a mesh is made by a VAO and a series of submeshes
  * (primitives of gltf).
  * Each submesh is also associated with a materials, and has
  * a series of VBOs
  * */
struct sScene {
    // Scene nodes
    bool            enabled[MAX_NODE_COUNT] = {};
    sMat44          models[MAX_NODE_COUNT] = {};
    uint16_t        mesh_of_object[MAX_NODE_COUNT] = {};

     // Scene composition
    // NOTE: Maybe, its better for data locality to include the is_full/used on the
    //       Submesh/material struct
    sMaterial       materials[MAX_MATERIAL_COUNT] = {};
    bool            is_material_full[MAX_MATERIAL_COUNT] = {};

    sMesh           meshes[MAX_MESH_COUNT] = {};
    bool            is_mesh_full[MAX_MESH_COUNT] = {};

    // SubMeshes's elements
    // TODO: double check the data locality on these
    bool            is_submesh_empty[MAX_SUBMESH_COUNT] = {};
    sSubMesh        submeshes[MAX_SUBMESH_COUNT] = {};
    uint16_t        submesh_material[MAX_SUBMESH_COUNT] = {};
    sSubMeshChild   submesh_child[MAX_SUBMESH_COUNT] = {};

    void init() {
        memset(enabled, false, sizeof(sScene::enabled));
        memset(submesh_child, 0, sizeof(sScene::submesh_child));
    };

    void load_gltf_model(const char* gltf_root_dir,
                         const char* gltf_file) {
        // Concat the files dir
        int root_dir_str_size = strlen(gltf_root_dir);
        int file_name_size = strlen(gltf_file);
        char* gltf_file_dir = (char*) malloc((root_dir_str_size + file_name_size) * sizeof(char));
        memcpy(gltf_file_dir, gltf_root_dir, root_dir_str_size);
        strcat(gltf_file_dir, gltf_file);

        std::cout << "Gltf file: " << gltf_file_dir << std::endl;



        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string error, warn;

        bool parse_result = loader.LoadASCIIFromFile(&model, &error, &warn, gltf_file_dir);
        assert(parse_result && "Error parsing GLTF model");

        // 1) Create, and fill VBO's data ===================
        // NOTE: maybe concatenate all the data on a single VBO, in order
        // to have less bidings on runtime
        uint32_t *total_VBOs = (uint32_t*) malloc(sizeof(uint32_t) * model.bufferViews.size());
        glGenBuffers(model.bufferViews.size(), total_VBOs);

        for(size_t bufferview_i = 0; bufferview_i < model.bufferViews.size(); bufferview_i++) {
            tinygltf::BufferView *buffer_view = &model.bufferViews[bufferview_i];

            glBindBuffer(buffer_view->target,
                         total_VBOs[bufferview_i]);
            glBufferData(buffer_view->target,
                         buffer_view->byteLength,
                         &model.buffers[buffer_view->buffer].data.at(0) + buffer_view->byteOffset,
                         GL_STATIC_DRAW);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // 2) Load meshes ===================================
        // For each mesh AKA group of submeshes/primitives
        uint32_t *total_VAOs = (uint32_t*) malloc(sizeof(uint32_t) * model.meshes.size());
        glGenVertexArrays(model.meshes.size(), total_VAOs);

        for(size_t mesh_i = 0; mesh_i <  model.meshes.size(); mesh_i++) {
            tinygltf::Mesh *gltf_mesh = &model.meshes[mesh_i];

            // Get the first availabe mesh spot on the scene
            uint16_t mesh_index = 0;
            for(;mesh_index < MAX_MESH_COUNT; mesh_index++) {
                if (!is_mesh_full[mesh_index]) {
                    break;
                }
            }
            is_mesh_full[mesh_index] = true;
            sMesh *mesh = &meshes[mesh_index];

            mesh->VAO = total_VAOs[mesh_index];
            glBindVertexArray(mesh->VAO);

            // for each submesh /  primitive
            for(size_t primitive_i = 0; primitive_i < gltf_mesh->primitives.size(); primitive_i++) {
                tinygltf::Primitive *prim = &gltf_mesh->primitives[primitive_i];
                // Get the first available submesh spot on memmory
                uint16_t submesh_index = 0;
                for(; submesh_index < MAX_SUBMESH_COUNT; submesh_index++) {
                    if (is_submesh_empty[submesh_index]) {
                        break;
                    }
                }
                sSubMesh *curr_submesh = &submeshes[submesh_index];

                curr_submesh->EBO = prim->indices;

                //TODO: WARNING: This is only for a very strict format, so it may be a VERY bad idea
                if (prim->attributes.find("POSITION") == prim->attributes.end()) {
                    uint32_t accesor_i = prim->attributes["POSITION"];
                    tinygltf::Accessor *accesor = &model.accessors[accesor_i];

                    curr_submesh->VBOs[VERTEX_BUFFER] = total_VBOs[accesor->bufferView];
                    glBindBuffer(GL_ARRAY_BUFFER, curr_submesh->VBOs[VERTEX_BUFFER]);

                    glEnableVertexAttribArray(VERTEX_BUFFER);
                    glVertexAttribPointer(VERTEX_BUFFER,
                                          3,
                                          GL_FLOAT,
                                          accesor->normalized ? GL_TRUE : GL_FALSE,
                                          3 * sizeof(float),
                                          (void*) 0);
                }

                if (prim->attributes.find("NORMAL") == prim->attributes.end()) {
                    uint32_t accesor_i = prim->attributes["NORMAL"];
                    tinygltf::Accessor *accesor = &model.accessors[accesor_i];

                    curr_submesh->VBOs[NORMAL_BUFFER] = total_VBOs[accesor->bufferView];
                    glBindBuffer(GL_ARRAY_BUFFER, curr_submesh->VBOs[NORMAL_BUFFER]);

                    glEnableVertexAttribArray(NORMAL_BUFFER);
                    glVertexAttribPointer(NORMAL_BUFFER,
                                          3,
                                          GL_FLOAT,
                                          accesor->normalized ? GL_TRUE : GL_FALSE,
                                          3 * sizeof(float),
                                          (void*) 0);

                }

                if (prim->attributes.find("TEXCOORD_0") == prim->attributes.end()) {
                    uint32_t accesor_i = prim->attributes["TEXCOORD_0"];
                    tinygltf::Accessor *accesor = &model.accessors[accesor_i];

                    curr_submesh->VBOs[UV_BUFFER] = total_VBOs[accesor->bufferView];
                    glBindBuffer(GL_ARRAY_BUFFER, curr_submesh->VBOs[UV_BUFFER]);

                    glEnableVertexAttribArray(UV_BUFFER);
                    glVertexAttribPointer(UV_BUFFER,
                                          2,
                                          GL_FLOAT,
                                          accesor->normalized ? GL_TRUE : GL_FALSE,
                                          2 * sizeof(float),
                                          (void*) 0);

                }

            }

            glBindVertexArray(0);
        }

        // 3) Load Textures =========================================
        // 4) Load Materials ========================================
        // 5) Load Models ===========================================
        // Cleanup
        free(total_VBOs);
        // free gltf
    }

    void render() {
        for(uint16_t node_i = 0; node_i < MAX_NODE_COUNT; node_i++) {
            if (!enabled[node_i]) {
                continue;
            }

            sMesh *curr_mesh = &meshes[mesh_of_object[node_i]];

            glBindVertexArray(curr_mesh->VAO);

            for(uint16_t submesh_id = curr_mesh->first_submesh; submesh_id < MAX_SUBMESH_COUNT ;) {
                // Bind material
                materials[submesh_material[submesh_id]].enable();
                // Bind & render submesh
                submeshes[submesh_id].rendering_bind();
                // TODO: add render uniforms
                submeshes[submesh_id].render();
                submeshes[submesh_id].rendering_unbind();

                // Render all the child/ associated submeshes with the current mesh
                if (submesh_child[submesh_id].has_child) {
                    submesh_id = submesh_child[submesh_id].child_index;
                } else {
                    break;
                }
            }
        }
    }
    void clean();
};

#endif // GLTF_SCENE_H_
