#include <catch2/catch_amalgamated.hpp>

#include "../vmlib/mat44.hpp"

// See mat44-rotation.cpp first!

constexpr bool approxEqual(Mat44f const& left, Mat44f const& right, float epsilon = 1e-6f) noexcept
{
    for (std::size_t i = 0; i < 16; ++i) {
        if (std::fabs(left.v[i] - right.v[i]) > epsilon) {
            return false;
        }
    }
    return true;
}

TEST_CASE("4x4 matrix by matrix multiplication", "[mat44]")
{
    // Define two matrices to multiply
    Mat44f matrix1 = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    };

    Mat44f matrix2 = {
        2.0f, 0.0f, 1.0f, 3.0f,
        1.0f, 2.0f, 4.0f, 2.0f,
        3.0f, 1.0f, 0.0f, 2.0f,
        0.0f, 3.0f, 2.0f, 1.0f
    };

    // Define the expected result of the matrix multiplication (manually calculated)
    Mat44f expectedResult = {
        13.0f, 19.0f, 17.0f, 17.0f,
        37.0f, 43.0f, 45.0f, 49.0f,
        61.0f, 67.0f, 73.0f, 81.0f,
        85.0f, 91.0f, 101.0f, 113.0f
    };

    // Perform matrix multiplication using the custom operator*
    Mat44f result = matrix1 * matrix2;
    float epsilon = 1e-6f;
    bool check = true;

    // Validate that the result matches the expected output
    float maxDiff = 0.0f;
    for (std::size_t i = 0; i < 4; ++i) {
        for (std::size_t j = 0; j < 4; ++j) {
            float diff = std::abs(result(i, j) - expectedResult(i, j));
            maxDiff = std::max(maxDiff, diff);
        }
    }
    // Validate that the maximum absolute difference is within the tolerance
    REQUIRE(maxDiff <= epsilon);
}

TEST_CASE("4x4 matrix by vector multiplication", "[mat44][vec4]")
{
    // Define a matrix and a vector for multiplication
    Mat44f matrix = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    };

    Vec4f vector = { 1.0f, 2.0f, 3.0f, 4.0f };

    // Define the expected result of the matrix-vector multiplication (manually calculated)
    Vec4f expectedResult = {
        30.0f,
        70.0f,
        110.0f,
        150.0f
    };

    // Perform matrix-vector multiplication using the custom operator*
    Vec4f result = matrix * vector;

    float epsilon = 1e-6f;

    // Validate that each element of the result matches the corresponding expected output
    REQUIRE(std::abs(result.x - expectedResult.x) < epsilon);
    REQUIRE(std::abs(result.y - expectedResult.y) < epsilon);
    REQUIRE(std::abs(result.z - expectedResult.z) < epsilon);
    REQUIRE(std::abs(result.w - expectedResult.w) < epsilon);
}
