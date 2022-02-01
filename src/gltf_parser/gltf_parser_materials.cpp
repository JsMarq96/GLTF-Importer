#include "../gltf_parser.h"
#include "glcorearb.h"

void _upload_texture(const eTextureType text_type,
                     const tinygltf::Image *tiny_img,
                           sMaterial *material) {
        GLenum img_format = 0;
        switch (tiny_img->component) {
            case 1:
                img_format = GL_RED;
                break;
            case 2:
                img_format = GL_RG;
                break;
            case 3:
                img_format = GL_RGB;
                break;
            case 4:
                img_format = GL_RGBA;
                break;
            default:
                assert("Unsorported texture format AKA more than 4 dims (RGBA)");
        }

        GLenum data_size = 0;
        switch(tiny_img->bits) {
            case 8:
                data_size = GL_UNSIGNED_BYTE;
                break;
            case 16:
                data_size = GL_UNSIGNED_SHORT;
                break;
            default:
                assert("Unsupported texture datatype size (not an Unsigned byte and short)");
        }

        material->add_raw_texture((const char*) &tiny_img->image.at(0),
                                    tiny_img->width,
                                    tiny_img->height,
                                    img_format,
                                    data_size,
                                    text_type);
}

void Parser::_load_gltf_materials(sScene *scene,
                                  const tinygltf::Model &model) {
    // Load Materials
    for(size_t material_i = 0; material_i < model.materials.size(); material_i++) {
        uint16_t material_index = 0;
        for(;material_index < MAX_MATERIAL_COUNT; material_index++) {
            if (!scene->is_material_full[material_index]) {
                break;
            }
        }

        const tinygltf::Material *tiny_material = &model.materials[material_i];
        sMaterial *material = &scene->materials[material_index];

        // TODO: AGUEIFNEI auto... sorry
        auto end_values = tiny_material->values.end();
        auto end_additional_values = tiny_material->additionalValues.end();

        auto it = tiny_material->values.find("baseColorTexture");
        if (it != end_values) {
            const tinygltf::Image *tiny_img = &model.images[model.textures[it->second.TextureIndex()].source];

            _upload_texture(COLOR_MAP,
                            tiny_img,
                            material);
        }

        it = tiny_material->additionalValues.find("normalTexture");
        if (it != end_additional_values) {
            const tinygltf::Image *tiny_img = &model.images[model.textures[it->second.TextureIndex()].source];

            _upload_texture(NORMAL_MAP,
                            tiny_img,
                            material);
        }

        //tiny_material->normalTexture;
    }
}
