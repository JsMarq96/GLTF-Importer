#include "gltf_scene.h"
#include "gl3w.h"
#include "glcorearb.h"
#include <cstdint>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#include "tiny_gltf.h"

void sScene::init() {
        memset(enabled, false, sizeof(sScene::enabled));
        memset(submesh_child, 0, sizeof(sScene::submesh_child));
    };

    void sScene::load_gltf_model(const char* gltf_root_dir) {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string error, warn;

        bool parse_result = loader.LoadASCIIFromFile(&model, &error, &warn, gltf_root_dir);
        assert(parse_result && "Error parsing GLTF model");

        // 1) Create, and fill VBO's data ===================
        // NOTE: maybe concatenate all the data on a single VBO, in order
        // to have less bidings on runtime
        std::cout << model.bufferViews.size() << std::endl;
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
        // Temp storage for building the nodes,
        // relates the GLTF's order to the in-engine order
        uint32_t *mesh_submesh_index = (uint32_t*) malloc(model.meshes.size() * sizeof(uint32_t));

        // Count the number of primitives, and create VAOs
        uint32_t primitive_count = 0;
        for(size_t mesh_i = 0; mesh_i <  model.meshes.size(); mesh_i++) {
            primitive_count += model.meshes[mesh_i].primitives.size();
        }
        uint32_t *total_VAOs = (uint32_t*) malloc(sizeof(uint32_t) * primitive_count);
        glGenVertexArrays(primitive_count, total_VAOs);
        uint16_t current_VAO_index = 0;

        for(size_t mesh_i = 0; mesh_i < model.meshes.size(); mesh_i++) {
            // Store the first submesh for this mesh
            uint16_t submesh_first_spot = 0;
            for(;submesh_first_spot < MAX_SUBMESH_COUNT; submesh_first_spot++) {
                if (is_submesh_empty[submesh_first_spot]) {
                    break;
                }
            }
            mesh_submesh_index[mesh_i] = submesh_first_spot;

            // Iterate primitives
            uint16_t last_submesh = submesh_first_spot;
            tinygltf::Mesh *gltf_mesh = &model.meshes[mesh_i];

            for(size_t primitive_i = 0; primitive_i < gltf_mesh->primitives.size(); primitive_i++) {
                tinygltf::Primitive *prim = &gltf_mesh->primitives[primitive_i];
                // Get the first available submesh spot on memmory
                uint16_t submesh_index = 0;
                for(; submesh_index < MAX_SUBMESH_COUNT; submesh_index++) {
                    if (is_submesh_empty[submesh_index]) {
                        break;
                    }
                }
                sSubMeshRenderData *curr_render_data = &submeshes_render[submesh_index];
                sSubMeshRenderBuffers *curr_render_buffers = &submeshes_buffers[submesh_index];

                // Set rendering mode TODO: a LUT for this??
                uint32_t mode = 0;
                switch(prim->mode) {
                    case TINYGLTF_MODE_TRIANGLES: mode = GL_TRIANGLES; break;
                    case TINYGLTF_MODE_TRIANGLE_FAN: mode = GL_TRIANGLE_FAN; break;
                    case TINYGLTF_MODE_TRIANGLE_STRIP: mode = GL_TRIANGLE_STRIP; break;
                    case TINYGLTF_MODE_LINE: mode = GL_LINES; break;
                    case TINYGLTF_MODE_LINE_LOOP: mode = GL_LINE_LOOP; break;
                    case TINYGLTF_MODE_POINTS: mode = GL_POINTS; break;
                    default: assert("Not implemented rendering mode");
                }
                curr_render_data->render_mode = mode;

                // Bind VAO
                curr_render_data->VAO = total_VAOs[current_VAO_index];
                glBindVertexArray(curr_render_data->VAO);

                // Store and bind EBO
                curr_render_buffers->EBO = prim->indices;
                curr_render_data->indices_size = model.accessors[prim->indices].count;
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr_render_buffers->EBO);

                // Bind VBOs
                //TODO: WARNING: This is only for a very strict format, so it may be a VERY bad idea
                if (prim->attributes.find("POSITION") == prim->attributes.end()) {
                    uint32_t accesor_i = prim->attributes["POSITION"];
                    tinygltf::Accessor *accesor = &model.accessors[accesor_i];

                    curr_render_buffers->VBOs[VERTEX_BUFFER] = total_VBOs[accesor->bufferView];
                    glBindBuffer(GL_ARRAY_BUFFER, curr_render_buffers->VBOs[VERTEX_BUFFER]);

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

                    curr_render_buffers->VBOs[NORMAL_BUFFER] = total_VBOs[accesor->bufferView];
                    glBindBuffer(GL_ARRAY_BUFFER, curr_render_buffers->VBOs[NORMAL_BUFFER]);

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

                    curr_render_buffers->VBOs[UV_BUFFER] = total_VBOs[accesor->bufferView];
                    glBindBuffer(GL_ARRAY_BUFFER, curr_render_buffers->VBOs[UV_BUFFER]);

                    glEnableVertexAttribArray(UV_BUFFER);
                    glVertexAttribPointer(UV_BUFFER,
                                          2,
                                          GL_FLOAT,
                                          accesor->normalized ? GL_TRUE : GL_FALSE,
                                          2 * sizeof(float),
                                          (void*) 0);

                }

                // Add to previus submesh as a child
                submesh_child[last_submesh].has_child = true;
                submesh_child[last_submesh].child_index = submesh_index;
                last_submesh = submesh_index;

                // Unbind VAO
                glBindVertexArray(0);
            }
            // Remove child for the last submesh
            submesh_child[last_submesh].has_child = false;
        }

        // 3) Load Textures =========================================
        // 4) Load Materials ========================================
        for(size_t material_i = 0; material_i < model.materials.size(); material_i++) {
            uint16_t material_index = 0;
            for(;material_index < MAX_MATERIAL_COUNT; material_index++) {
                if (!is_material_full[material_index]) {
                    break;
                }
            }

            tinygltf::Material *tiny_material = &model.materials[material_i];
            sMaterial *material = &materials[material_index];

            //tiny_material->normalTexture;
        }
        // 5) Load Nodes ============================================
        // Cleanup
        free(total_VBOs);
        // free gltf
    }


 void sScene::render() {
        for(uint16_t node_i = 0; node_i < MAX_NODE_COUNT; node_i++) {
            if (!enabled[node_i]) {
                continue;
            }

            for(uint16_t submesh_index = mesh_of_object[node_i]; submesh_index < MAX_SUBMESH_COUNT ;) {
                sSubMeshRenderData *render_data = &submeshes_render[submesh_index];
                // Bind VAO
                glBindVertexArray(render_data->VAO);

                // Bind material
                materials[submesh_material[submesh_index]].enable();

                // TODO: uniforms

                glDrawElements(render_data->render_mode,
                               render_data->indices_size,
                               GL_UNSIGNED_SHORT,
                               0);

                // Render all the child/ associated submeshes with the current mesh
                if (submesh_child[submesh_index].has_child) {
                    submesh_index = submesh_child[submesh_index].child_index;
                } else {
                    break;
                }
            }
        }
    }
