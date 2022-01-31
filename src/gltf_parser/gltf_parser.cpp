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

        Parser::load_gltf_geometry(scene, model);
        // 3) Load Textures =========================================
        // 4) Load Materials ========================================
        for(size_t material_i = 0; material_i < model.materials.size(); material_i++) {
            uint16_t material_index = 0;
            for(;material_index < MAX_MATERIAL_COUNT; material_index++) {
                if (!scene->is_material_full[material_index]) {
                    break;
                }
            }

            tinygltf::Material *tiny_material = &model.materials[material_i];
            sMaterial *material = &scene->materials[material_index];

            //tiny_material->normalTexture;
        }
        // 5) Load Nodes ============================================
        // free gltf
    }
