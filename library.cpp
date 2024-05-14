#include "library.h"

#include <cmath>

// ----- QUATERNIONS -----

Quaternion::Quaternion() {
    a = 0;
    b = 0;
    c = 0;
    d = 0;
}

Quaternion::Quaternion(float a, float b, float c, float d) {
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
}

Quaternion Quaternion::add(const Quaternion &other) {
    return {
            a + other.a,
            b + other.b,
            c + other.c,
            d + other.d
        };
}

Quaternion Quaternion::multiply(float x) {
    Quaternion result = Quaternion(a, b, c, d);
    result.a *= x;
    result.b *= x;
    result.c *= x;
    result.d *= x;

    return result;
}

Quaternion Quaternion::multiply(const Quaternion &other) {
    Quaternion result = Quaternion();
    result.a = (a * other.a) - (b * other.b) - (c * other.c) - (d * other.d);
    result.b = (a * other.b) + (b * other.a) + (c * other.d) - (d * other.c);
    result.c = (a * other.c) - (b * other.d) + (c * other.a) + (d * other.b);
    result.d = (a * other.d) + (b * other.c) - (c * other.b) + (d * other.a);

    return result;
}

Quaternion Quaternion::conjugate() {
    return {a, -b, -c, -d};
}

Quaternion Quaternion::getUnit() {
    return this->multiply(1 / getNorm());
}

float Quaternion::getNorm() {
    float uNorm = sqrt(b * b + c * c + d * d);
    return sqrt(a * a + uNorm * uNorm);
}

float Quaternion::scalarProduct(const Quaternion &other) {
    return a * other.a + b * other.b + c * other.c + d * other.d;
}

class QuaternionMatrix Quaternion::toMatrix() {
    return QuaternionMatrix(
            a, -b, -c, -d,
            b, a, -d, c,
            c, d, a, -b,
            d, -c, b, a
            );
}

// ----- MATRIX -----

// ugly constructor
QuaternionMatrix::QuaternionMatrix(
        float a1, float a2, float a3, float a4,
        float b1, float b2, float b3, float b4,
        float c1, float c2, float c3, float c4,
        float d1, float d2, float d3, float d4) :
        a1(a1), a2(a2), a3(a3), a4(a4),
        b1(b1), b2(b2), b3(b3), b4(b4),
        c1(c1), c2(c2), c3(c3), c4(c4),
        d1(d1), d2(d2), d3(d3), d4(d4) {}

QuaternionMatrix QuaternionMatrix::multiply(float x) {
    return {
        a1 * x, a2 * x, a3 * x, a4 * x,
        b1 * x, b2 * x, b3 * x, b4 * x,
        c1 * x, c2 * x, c3 * x, c4 * x,
        d1 * x, d2 * x, d3 * x, d4 * x,
    };
}

QuaternionMatrix QuaternionMatrix::multiply(QuaternionMatrix other) {
    return {
        a1 * other.a1 + a2 * other.b1 + a3 * other.c1 + a4 * other.d1,
        a1 * other.a2 + a2 * other.b2 + a3 * other.c2 + a4 * other.d2,
        a1 * other.a3 + a2 * other.b3 + a3 * other.c3 + a4 * other.d3,
        a1 * other.a4 + a2 * other.b4 + a3 * other.c4 + a4 * other.d4,

        b1 * other.a1 + b2 * other.b1 + b3 * other.c1 + b4 * other.d1,
        b1 * other.a2 + b2 * other.b2 + b3 * other.c2 + b4 * other.d2,
        b1 * other.a3 + b2 * other.b3 + b3 * other.c3 + b4 * other.d3,
        b1 * other.a4 + b2 * other.b4 + b3 * other.c4 + b4 * other.d4,

        c1 * other.a1 + c2 * other.b1 + c3 * other.c1 + c4 * other.d1,
        c1 * other.a2 + c2 * other.b2 + c3 * other.c2 + c4 * other.d2,
        c1 * other.a3 + c2 * other.b3 + c3 * other.c3 + c4 * other.d3,
        c1 * other.a4 + c2 * other.b4 + c3 * other.c4 + c4 * other.d4,

        d1 * other.a1 + d2 * other.b1 + d3 * other.c1 + d4 * other.d1,
        d1 * other.a2 + d2 * other.b2 + d3 * other.c2 + d4 * other.d2,
        d1 * other.a3 + d2 * other.b3 + d3 * other.c3 + d4 * other.d3,
        d1 * other.a4 + d2 * other.b4 + d3 * other.c4 + d4 * other.d4
    };
}

Quaternion QuaternionMatrix::toQuaternion() {
    return {a1, b1, c1, d1};
}