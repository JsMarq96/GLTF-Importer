#ifndef MATRIX44_H_
#define MATRIX44_H_

#include <string.h>
#include <assert.h>
#include <cmath>

#include "math_types.h"

union sMat44 {
    float raw_values[16] = { 0.0f, 0.0f, 0.0f,0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,0.0f, 0.0f, 0.0f, 0.0f };
    float mat_values[4][4];
    struct {
        float sx1; float sy1; float sz1; float tmp1;
        float sx2; float sy2; float sz2; float tmp2;
        float sx3; float sy3; float sz3; float tmp3;
        float px; float py; float pz; float tmp4;
    };

    void
    set_identity();
    ///
    /// SIMPLE OPERATIONS
    ///

    inline void set_position(const sVector3 &vec);

    inline void add_position(const sVector3 &vec);

    inline void set_scale(const sVector3 &vec);


    ///
    /// COMPLEX OPERATIONS
    ///

    // TODO: Wrap loops
    // TODO: Optimize via SIMD
    inline void multiply(const sMat44   *B);

    inline void rotate(const sQuaternion4 &quat);

    inline void scale(const sVector3 &vect);

    inline sQuaternion4 multiply(const sQuaternion4 &quat) const;

    inline sVector4 multiply(const sVector4   &vect) const;

    inline sVector3 multiply(const sVector3   &vect) const;

    inline void transpose_to(sMat44* result) const;

    inline void transpose();

    inline void convert_quaternion_to_matrix(const sQuaternion4 &quat);

    // Yoinked from a stackoverlof that yoinked from the MESA implmentation
    // of GLU
    // https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
    // It uses Sarrus' rule
    // TODO: SIMD...?
    void invert(sMat44 *result) const;

    inline void print() const;
};

#include "matrix44.inl"

#endif // MATRIX44_H_
