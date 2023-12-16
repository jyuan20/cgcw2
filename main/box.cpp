#include "box.hpp"

SimpleMeshData make_box(float width, float height, float depth, Vec3f color, Mat44f preTransform) {
    std::vector<Vec3f> positions;
    std::vector<Vec3f> colors;
    std::vector<Vec3f> normals;

    // Define the eight vertices of the box forming a perfect cube
    std::vector<Vec3f> vertices = {
        {0.f, 0.f, 0.f},
        {width, 0.f, 0.f},
        {0.f, height, 0.f},
        {width, height, 0.f},
        {0.f, 0.f, depth},
        {width, 0.f, depth},
        {0.f, height, depth},
        {width, height, depth}
    };

    // Define the indices for the two triangles of each face of the box
    std::vector<std::vector<int>> faces = {
        {0, 1, 2, 1, 3, 2}, // Front face
        {1, 5, 3, 5, 7, 3}, // Right face
        {5, 4, 7, 4, 6, 7}, // Back face
        {4, 0, 6, 0, 2, 6}, // Left face
        {2, 3, 6, 3, 7, 6}, // Top face
        {4, 5, 0, 5, 1, 0}  // Bottom face
    };

    // Calculate normals for each face
    for (size_t i = 0; i < faces.size(); ++i) {
        Vec3f v0 = vertices[faces[i][0]];
        Vec3f v1 = vertices[faces[i][1]];
        Vec3f v2 = vertices[faces[i][2]];

        Vec3f normal = normalize(cross(v1 - v0, v2 - v0));

        for (int j = 0; j < 6; ++j) {
            normals.push_back(normal);
        }
    }

    // Create the box by iterating through the faces and assigning vertices
    for (size_t i = 0; i < faces.size(); ++i) {
        for (int vertexIdx : faces[i]) {
            positions.push_back(vertices[vertexIdx]);
            colors.push_back(color);
        }
    }

    // Apply pre-transformation to each vertex position
    for (Vec3f& position : positions) {
        Vec4f transformedPosition = preTransform * Vec4f{ position.x, position.y, position.z, 1.0f };
        position = Vec3f{ transformedPosition.x, transformedPosition.y, transformedPosition.z };
    }

    // Return the mesh data with positions, colors, and normals (without texture coordinates)
    return SimpleMeshData{ std::move(positions), std::move(colors), std::move(normals), std::vector<Vec2f>() };
}