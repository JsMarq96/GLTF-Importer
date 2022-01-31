#include "gltf_scene.h"
#include "gl3w.h"
#include "glcorearb.h"
#include <cstdint>


void sScene::init() {
        memset(enabled, false, sizeof(sScene::enabled));
        memset(submesh_child, 0, sizeof(sScene::submesh_child));
        memset(is_submesh_full, false, sizeof(sScene::is_submesh_full));
        memset(is_material_full, false, sizeof(sScene::is_material_full));
    };



void sScene::render(const sCamera &camera,
                    const sMat44 &view_proj) const {
        for(uint16_t node_i = 0; node_i < MAX_NODE_COUNT; node_i++) {
            if (!enabled[node_i]) {
                continue;
            }

            for(uint16_t submesh_index = mesh_of_object[node_i]; submesh_index < MAX_SUBMESH_COUNT ;) {
                const sSubMeshRenderData *render_data = &submeshes_render[submesh_index];
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
