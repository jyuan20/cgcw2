#ifndef BOX_HPP
#define BOX_HPP

#include <vector>
#include "simple_mesh.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

SimpleMeshData make_box(
    float width = 1.0f,
    float height = 1.0f,
    float depth = 1.0f,
    Vec3f color = { 1.0f, 1.0f, 1.0f },
    Mat44f preTransform = kIdentity44f
);

#endif // BOX_HPP