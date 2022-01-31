#ifndef GLTF_PARSER_H_
#define GLTF_PARSER_H_

#include "material.h"
#include "gltf_scene.h"

namespace Parser {
    void load_gltf_model(      sScene *scene,
                         const char* gltf_root_dir);
};

#endif // GLTF_PARSER_H_
