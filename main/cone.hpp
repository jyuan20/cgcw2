#ifndef CONE_HPP
#define CONE_HPP

#include <vector>
#include "../vmlib/vec3.hpp" // Replace this with the appropriate Vec3 header
#include "../vmlib/vec2.hpp" // Replace this with the appropriate Vec2 header
#include "../vmlib/mat44.hpp" // Replace this with the appropriate Mat44 header
#include "simple_mesh.hpp"

SimpleMeshData make_cone(
    bool aCapped = true,
    std::size_t aSubdivs = 16,
    Vec3f aColor = { 1.f, 1.f, 1.f },
    Mat44f aPreTransform = kIdentity44f
);

#endif // CONE_HPP
