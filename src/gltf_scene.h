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
#define MAX_MESH_COUNT 3000
#define MAX_NODE_COUNT 2000

enum eVBO : uint8_t {
    VERTEX_BUFFER = 0,
    NORMAL_BUFFER,
    UV_BUFFER,
    MAX_VBO_COUNT
};

struct sSubMesh {
    uint8_t   used_vbos = 0;
    uint32_t  VBOs[MAX_VBO_COUNT] = { 0 };
    uint32_t  EBO = 0;

    void rendering_bind();
    void rendering_unbind();
    void clean();
};

// Tuple type, for child meshes
struct sSubMeshChild {
    bool      has_child = false;
    uint16_t  child_index = 0;
};

struct sScene {
    // Scene elements
    bool            enabled[MAX_NODE_COUNT] = {};
    sMat44          models[MAX_NODE_COUNT] = {};
    uint16_t        VAOs[MAX_NODE_COUNT] = {};
    uint16_t        mesh_of_object[MAX_NODE_COUNT] = {};

    // SubMeshes's elements
    uint16_t        submesh_material[MAX_MESH_COUNT] = {};
    uint16_t        submesh_id[MAX_MESH_COUNT] = {};
    sSubMeshChild   submesh_child[MAX_MESH_COUNT] = {};

    // Scene composition
    // NOTE: Maybe, its better for data locality to include the is_full/used on the
    //       Submesh/material struct
    sMaterial       materials[MAX_MATERIAL_COUNT] = {};
    bool            is_material_full[MAX_MATERIAL_COUNT] = {};

    sSubMesh        meshes[MAX_MESH_COUNT] = {};
    bool            is_mesh_full[MAX_MESH_COUNT] = {};

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

        // Create, and fill VBO's data
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

        // For each primitive/submesh
        for(size_t mesh_i = 0; mesh_i <  model.meshes.size(); mesh_i++) {
            tinygltf::Mesh *mesh = &model.meshes[mesh_i];

            for(size_t primitive_i = 0; primitive_i < mesh->primitives.size(); primitive_i++) {
                tinygltf::Primitive *prim = &mesh->primitives[primitive_i];
                uint16_t submesh_index = 0;
                for(; submesh_index < MAX_MESH_COUNT; submesh_index++) {
                    if (!is_mesh_full[submesh_index]) {
                        break;
                    }
                }
                sSubMesh *curr_submesh = &meshes[submesh_index];

                curr_submesh->EBO = prim->indices;

                if (prim->attributes.find("POSITION") == prim->attributes.end()) {
                    uint32_t accesor_i = prim->attributes["POSITION"];
                    tinygltf::Accessor *accesor = &model.accessors[accesor_i];

                    glBindBuffer(GL_ARRAY_BUFFER, total_VBOs[accesor->bufferView]);
                    curr_submesh->VBOs[VERTEX_BUFFER] = accesor->bufferView;

                    glEnableVertexAttribArray(VERTEX_BUFFER);
                    glVertexAttribPointer(VERTEX_BUFFER)
                }

                if (prim->attributes.find("NORMAL") == prim->attributes.end()) {
                    uint32_t accesor_i = prim->attributes["NORMAL"];
                    tinygltf::Accessor *accesor = &model.accessors[accesor_i];

                    glBindBuffer(GL_ARRAY_BUFFER, total_VBOs[accesor->bufferView]);
                    curr_submesh->VBOs[NORMAL_BUFFER] = accesor->bufferView;

                    glEnableVertexAttribArray(NORMAL_BUFFER);
                    // glVertexAttribPointer
                }

                if (prim->attributes.find("TEXCOORD_0") == prim->attributes.end()) {
                    uint32_t accesor_i = prim->attributes["TEXCOORD_0"];
                    tinygltf::Accessor *accesor = &model.accessors[accesor_i];

                    glBindBuffer(GL_ARRAY_BUFFER, total_VBOs[accesor->bufferView]);
                    curr_submesh->VBOs[UV_BUFFER] = accesor->bufferView;

                    glEnableVertexAttribArray(UV_BUFFER);
                    // glVertexAttribPointer
                }

            }
        }
    }

    void clean();
};

#endif // GLTF_SCENE_H_
