#include "simple_mesh.hpp"
SimpleMeshData concatenate(SimpleMeshData aM, SimpleMeshData const& aN) {
    // Concatenate positions, colors, normals, and texCoords
    aM.positions.insert(aM.positions.end(), aN.positions.begin(), aN.positions.end());
    aM.colors.insert(aM.colors.end(), aN.colors.begin(), aN.colors.end());
    aM.normals.insert(aM.normals.end(), aN.normals.begin(), aN.normals.end());
    aM.texCoords.insert(aM.texCoords.end(), aN.texCoords.begin(), aN.texCoords.end());

    return aM;
}

GLuint create_vao(SimpleMeshData const& aMeshData)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint posVbo, colorVbo, normalVbo, texCoordVbo; // Adding normal VBO

    // Create a VBO for positions
    glGenBuffers(1, &posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glBufferData(GL_ARRAY_BUFFER, aMeshData.positions.size() * sizeof(Vec3f), aMeshData.positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create a VBO for colors
    glGenBuffers(1, &colorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    glBufferData(GL_ARRAY_BUFFER, aMeshData.colors.size() * sizeof(Vec3f), aMeshData.colors.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1); // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create a VBO for normals
    glGenBuffers(1, &normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
    glBufferData(GL_ARRAY_BUFFER, aMeshData.normals.size() * sizeof(Vec3f), aMeshData.normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2); // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create a VBO for texture coordinates
    glGenBuffers(1, &texCoordVbo);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordVbo);
    glBufferData(GL_ARRAY_BUFFER, aMeshData.texCoords.size() * sizeof(Vec2f), aMeshData.texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3); // Texture attribute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Unbind the VAO to prevent accidental changes
    glBindVertexArray(0);

    // Clean up by unbinding any VBOs
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}

