#include "gltf_scene.h"

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
            sMesh2 *mesh = &meshes[mesh_index];

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

                // TODO: a LUT for this??
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
                curr_submesh->rendering_primitive = mode;

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
        for(size_t material_i = 0; material_i < model.materials.size(); material_i++) {
            uint16_t material_index = 0;
            for(;material_index < MAX_MATERIAL_COUNT; material_index++) {
                if (!is_material_full[material_index]) {
                    break;
                }
            }

            tinygltf::Material *tiny_material = &model.materials[material_i];
            sMaterial *material = &materials[material_index];

            tiny_material->normalTexture;
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

            sMesh2 *curr_mesh = &meshes[mesh_of_object[node_i]];

            glBindVertexArray(curr_mesh->VAO);

            for(uint16_t submesh_id = curr_mesh->first_submesh; submesh_id < MAX_SUBMESH_COUNT ;) {
                // Bind material
                materials[submesh_material[submesh_id]].enable();
                // Bind & render submesh
                //submeshes[submesh_id].rendering_bind();
                // TODO: add render uniforms
                //submeshes[submesh_id].render();
                //submeshes[submesh_id].rendering_unbind();

                // Render all the child/ associated submeshes with the current mesh
                if (submesh_child[submesh_id].has_child) {
                    submesh_id = submesh_child[submesh_id].child_index;
                } else {
                    break;
                }
            }
        }
    }
