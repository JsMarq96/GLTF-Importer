#include "../gltf_parser.h"


void Parser::_load_gltf_textures(sScene *scene,
                                const tinygltf::Model &model) {
}

void Parser::_load_gltf_materials(sScene *scene,
                                 const tinygltf::Model &model) {
    for(size_t material_i = 0; material_i < model.materials.size(); material_i++) {
            uint16_t material_index = 0;
            for(;material_index < MAX_MATERIAL_COUNT; material_index++) {
                if (!scene->is_material_full[material_index]) {
                    break;
                }
            }

            const tinygltf::Material *tiny_material = &model.materials[material_i];
            sMaterial *material = &scene->materials[material_index];

            //tiny_material->normalTexture;
        }
}
