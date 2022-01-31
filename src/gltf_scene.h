#ifndef GLTF_SCENE_H_
#define GLTF_SCENE_H_

#include "gl3w.h"
#include "glcorearb.h"
#include "shader.h"
#include "raw_shaders.h"
#include "material.h"
#include "camera.h"

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


struct sSubMeshRenderData {
    uint32_t VAO = 0;
    uint32_t render_mode = 0;
    uint16_t indices_size = 0;
};

struct sSubMeshRenderBuffers {
    bool      used_VBOs[MAX_VBO_COUNT] = { false, false, false };
    uint32_t  VBOs[MAX_VBO_COUNT] = { 0 };
    uint32_t  EBO = 0;

    void clean();
};

// Tuple type, for child meshes
struct sSubMeshChild {
    bool      has_child = false;
    uint16_t  child_index = 0;
};

 /**
  * Scene's structure:
  * A scene is made out of nodes, and each node has:
  *  -A transformation
  *  -A mesh
  *
  * Each submesh is also associated with a materials, and has
  * a series of VBOs
  * */
struct sScene {
    // Scene nodes
    bool                      enabled[MAX_NODE_COUNT] = {};
    sMat44                    models[MAX_NODE_COUNT] = {};
    uint16_t                  mesh_of_object[MAX_NODE_COUNT] = {};

     // Scene composition
    // NOTE: Maybe, its better for data locality to include the is_full/used on the
    //       Submesh/material struct
    sMaterial                 materials[MAX_MATERIAL_COUNT] = {};
    bool                      is_material_full[MAX_MATERIAL_COUNT] = {};

    // SubMeshes's elements
    // TODO: double check the data locality on these
    bool                      is_submesh_full[MAX_SUBMESH_COUNT] = {};
    sSubMeshRenderData        submeshes_render[MAX_SUBMESH_COUNT] = {};
    sSubMeshRenderBuffers     submeshes_buffers[MAX_SUBMESH_COUNT] = {};
    uint16_t                  submesh_material[MAX_SUBMESH_COUNT] = {};
    sSubMeshChild             submesh_child[MAX_SUBMESH_COUNT] = {};

    void init();

    void load_gltf_model(const char* gltf_dir_file);

    void render();

    void clean();
};

#endif // GLTF_SCENE_H_
