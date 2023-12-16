#include "cylinder.hpp"

#include "../vmlib/mat33.hpp"

SimpleMeshData make_cylinder(bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform) {
    std::vector<Vec3f> pos;
    std::vector<Vec3f> col; // Color buffer
    std::vector<Vec3f> norms; // Normals buffer

    // Compute the normal transformation matrix only once, outside the loop
    Mat44f normalMatrix = transpose(invert(aPreTransform));

    float prevY = std::cos(0.f);
    float prevZ = std::sin(0.f);
    for (std::size_t i = 0; i < aSubdivs; ++i) {
        float const angle = (i + 1) / float(aSubdivs) * 2.f * 3.1415926f;
        float y = std::cos(angle);
        float z = std::sin(angle);

        pos.emplace_back(Vec3f{ 0.f, prevY, prevZ });
        pos.emplace_back(Vec3f{ 0.f, y, z });
        pos.emplace_back(Vec3f{ 1.f, prevY, prevZ });
        pos.emplace_back(Vec3f{ 0.f, y, z });
        pos.emplace_back(Vec3f{ 1.f, y, z });
        pos.emplace_back(Vec3f{ 1.f, prevY, prevZ });

        // Cylinder side normal is constant
        Vec3f normal = Vec3f{ 1.0, 0.0, 0.0 };
        Vec4f transformedNormal4 = normalMatrix * Vec4f{ normal.x, normal.y, normal.z, 0.0f };
        Vec3f transformedNormal = normalize(Vec3f{ transformedNormal4.x, transformedNormal4.y, transformedNormal4.z });

        for (int j = 0; j < 6; ++j) { // 6 vertices (two triangles) per segment
            norms.push_back(transformedNormal);
        }

        prevY = y;
        prevZ = z;
    }

    if (aCapped) {
        // Compute and transform normals for the caps
        Vec3f topCapNormal = normalize(Vec3f{ 0.0, 0.0, 1.0 });
        Vec3f bottomCapNormal = normalize(Vec3f{ 0.0, 0.0, -1.0 });

        // Center of the base
        Vec3f center = Vec3f{ 0.f, 0.f, 0.f };
        prevY = std::cos(0.f);
        prevZ = std::sin(0.f);
        for (std::size_t i = 0; i < aSubdivs; ++i) {
            float const angle = (i + 1) / float(aSubdivs) * 2.f * 3.1415926f;
            float y = std::cos(angle);
            float z = std::sin(angle);

            pos.emplace_back(center);
            pos.emplace_back(Vec3f{ 0.f, prevY, prevZ });
            pos.emplace_back(Vec3f{ 0.f, y, z });

            for (int j = 0; j < 3; ++j) { // 3 vertices per triangle
                norms.push_back(bottomCapNormal);
            }

            pos.emplace_back(Vec3f{ 1.f, 0.f, 0.f }); // Top center point
            pos.emplace_back(Vec3f{ 1.f, prevY, prevZ });
            pos.emplace_back(Vec3f{ 1.f, y, z });

            for (int j = 0; j < 3; ++j) { // 3 vertices per triangle
                norms.push_back(topCapNormal);
            }

            prevY = y;
            prevZ = z;
        }
    }

    col.resize(pos.size(), aColor);

    for (Vec3f& position : pos) {
        Vec4f transformedPosition = aPreTransform * Vec4f{ position.x, position.y, position.z, 1.0f };
        position = Vec3f{ transformedPosition.x, transformedPosition.y, transformedPosition.z };
    }

    return SimpleMeshData{ std::move(pos), std::move(col), std::move(norms), std::vector<Vec2f>() };
}