#include "loadobj.hpp"
#include <rapidobj/rapidobj.hpp>
#include <filesystem> // for checking file existence
#include "../support/error.hpp"

SimpleMeshData load_wavefront_obj(const char* aPath) {
    // Check if the file exists
    if (!std::filesystem::exists(aPath)) {
        throw Error("OBJ file does not exist: '%s'", aPath);
    }

    auto result = rapidobj::ParseFile(aPath);
    if (result.error) {
        throw Error("Unable to load OBJ file '%s': %s", aPath, result.error.code.message().c_str());
    }

    rapidobj::Triangulate(result);

    SimpleMeshData ret;

    for (auto const& shape : result.shapes) {
        for (std::size_t i = 0; i < shape.mesh.indices.size(); ++i) {
            auto const& idx = shape.mesh.indices[i];

            auto unsigned_pos_idx = static_cast<std::size_t>(idx.position_index);
            auto unsigned_norm_idx = static_cast<std::size_t>(idx.normal_index);
            auto unsigned_texcoord_idx = static_cast<std::size_t>(idx.texcoord_index);

            if (unsigned_pos_idx < result.attributes.positions.size() / 3 &&
                unsigned_norm_idx < result.attributes.normals.size() / 3 &&
                unsigned_texcoord_idx < result.attributes.texcoords.size() / 2) {

                // Load positions
                ret.positions.emplace_back(Vec3f{
                    result.attributes.positions[idx.position_index * 3 + 0],
                    result.attributes.positions[idx.position_index * 3 + 1],
                    result.attributes.positions[idx.position_index * 3 + 2]
                    });

                // Load normals
                ret.normals.emplace_back(Vec3f{
                    result.attributes.normals[idx.normal_index * 3 + 0],
                    result.attributes.normals[idx.normal_index * 3 + 1],
                    result.attributes.normals[idx.normal_index * 3 + 2]
                    });

                // Check for valid material indices
                if (i / 3 < shape.mesh.material_ids.size()) {
                    auto const& mat = result.materials[shape.mesh.material_ids[i / 3]];

                    // Load colors or any other attributes
                    ret.colors.emplace_back(Vec3f{
                        mat.ambient[0],
                        mat.ambient[1],
                        mat.ambient[2]
                        });
                }
                else {
                    // Default color or handling when material index is not available
                    ret.colors.emplace_back(Vec3f{ 1.0f, 1.0f, 1.0f });
                }

                // Load texture coordinates
                ret.texCoords.emplace_back(Vec2f{
                    result.attributes.texcoords[idx.texcoord_index * 2 + 0],
                    result.attributes.texcoords[idx.texcoord_index * 2 + 1]
                    });
            }
        }
    }

    return ret;
}