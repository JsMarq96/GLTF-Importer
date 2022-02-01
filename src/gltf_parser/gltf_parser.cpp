#include "../gltf_parser.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

void Parser::load_gltf_model(      sScene  *scene,
                             const char*   gltf_root_dir) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string error, warn;

    bool parse_result = loader.LoadASCIIFromFile(&model, &error, &warn, gltf_root_dir);
    assert(parse_result && "Error parsing GLTF model");

    Parser::_load_gltf_geometry(scene, model);
    Parser::_load_gltf_materials(scene, model);

    // free gltf
}
