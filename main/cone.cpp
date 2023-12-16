#include "cone.hpp" // Include cone header file if it exists

SimpleMeshData make_cone(bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform) {
    std::vector<Vec3f> pos;
    std::vector<Vec3f> col; // Color buffer
    std::vector<Vec3f> norms; // Normals buffer
    std::vector<Vec2f> texCoords;

    // Compute the normal transformation matrix only once, outside the loop
    Mat44f normalMatrix = transpose(invert(aPreTransform));
    // Cone generation logic
    float topY = 0.2f; // Top point of the cone
    float bottomY = -0.2f; // Bottom point of the cone

    for (std::size_t i = 0; i < aSubdivs; ++i) {
        float const angle1 = i / float(aSubdivs) * 2.f * 3.1415926f;
        float const angle2 = (i + 1) / float(aSubdivs) * 2.f * 3.1415926f;

        float x1 = std::cos(angle1);
        float z1 = std::sin(angle1);
        float x2 = std::cos(angle2);
        float z2 = std::sin(angle2);

        // Triangle for the side of the cone
        pos.emplace_back(Vec3f{ 0.f, topY, 0.f });
        pos.emplace_back(Vec3f{ x1, bottomY, z1 });
        pos.emplace_back(Vec3f{ x2, bottomY, z2 });

        // Calculate normals for the cone side
        Vec3f sideNormal = cross(Vec3f{ x2 - x1, bottomY - topY, z2 - z1 }, Vec3f{ x1, bottomY, z1 });
        Vec4f transformedNormal4 = normalMatrix * Vec4f{ sideNormal.x, sideNormal.y, sideNormal.z, 0.0f };
        Vec3f transformedNormal = normalize(Vec3f{ transformedNormal4.x, transformedNormal4.y, transformedNormal4.z });

        // Add the transformed and normalized normal for each vertex of the triangle
        for (int j = 0; j < 3; ++j) { // 3 vertices per triangle
            norms.push_back(transformedNormal);
        }

        // Texture coordinates (you may need to adjust this based on your texture mapping)
        texCoords.emplace_back(Vec2f{ 0.5f, 0.5f }); // Center point
        texCoords.emplace_back(Vec2f{ x1 * 0.5f + 0.5f, z1 * 0.5f + 0.5f }); // Bottom point 1
        texCoords.emplace_back(Vec2f{ x2 * 0.5f + 0.5f, z2 * 0.5f + 0.5f }); // Bottom point 2
    }

    // Now that we have all the positions, we can assign the color to each vertex
    col.resize(pos.size(), aColor);

    // Apply the pre-transformation to each vertex position
    for (Vec3f& position : pos) {
        Vec4f transformedPosition = aPreTransform * Vec4f{ position.x, position.y, position.z, 1.0f };
        position = Vec3f{ transformedPosition.x, transformedPosition.y, transformedPosition.z };
    }

    // Return the mesh data with positions, colors, and normals for the cone
    return SimpleMeshData{ std::move(pos), std::move(col), std::move(norms), std::move(texCoords) };
}

