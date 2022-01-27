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
    uint16_t  VBOs[MAX_VBO_COUNT] = { 0 };
    uint16_t  EBO = 0;

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

    void clean();
};

#endif // GLTF_SCENE_H_
