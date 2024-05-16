#include "library.h"

#include <cmath>

// ----- QUATERNIONS -----

Quaternion::Quaternion() {
    a = 0;
    b = 0;
    c = 0;
    d = 0;
}

Quaternion::Quaternion(double a, double b, double c, double d) {
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

Quaternion Quaternion::multiply(double x) {
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

double Quaternion::getNorm() {
    double uNorm = sqrt(b * b + c * c + d * d);
    return sqrt(a * a + uNorm * uNorm);
}

double Quaternion::scalarProduct(const Quaternion &other) {
    return a * other.a + b * other.b + c * other.c + d * other.d;
}

Quaternion Quaternion::eulerAngles(double rads, double x, double y, double z) {
    double angleSin = sin(rads / 2);

    return Quaternion(cos(rads / 2), x * angleSin, y * angleSin, z * angleSin);
}

class QuaternionMatrix Quaternion::toMatrix() {
    return QuaternionMatrix(
            a, -b, -c, -d,
            b, a, -d, c,
            c, d, a, -b,
            d, -c, b, a
    );
}

class RotationMatrix Quaternion::getRotationMatrix() {
    Quaternion unit = getUnit();
    return RotationMatrix(
            1 - 2*unit.c*unit.c - 2*unit.d*unit.d, 2*unit.b*unit.c - 2*unit.d*unit.a, 2*unit.b*unit.d + 2*unit.c*unit.a,
            2*unit.b*unit.c + 2*unit.d*unit.a, 1 - 2*unit.b*unit.b - 2*unit.d*unit.d, 2*unit.c*unit.d - 2*unit.b*unit.a,
            2*unit.b*unit.d - 2*unit.c*unit.a, 2*unit.c*unit.d + 2*unit.b*unit.a, 1 - 2*unit.b*unit.b - 2*unit.c*unit.c
    );
}

double Quaternion::crossProduct(const Quaternion &other) {
    return Quaternion(
            b * other.c - c * other.b,
            c * other.d - d * other.c,
            d * other.a - a * other.d,
            a * other.b - b * other.a
    );
}

// ----- MATRIX -----

// ugly constructor
QuaternionMatrix::QuaternionMatrix(
        double a1, double a2, double a3, double a4,
        double b1, double b2, double b3, double b4,
        double c1, double c2, double c3, double c4,
        double d1, double d2, double d3, double d4) :
        a1(a1), a2(a2), a3(a3), a4(a4),
        b1(b1), b2(b2), b3(b3), b4(b4),
        c1(c1), c2(c2), c3(c3), c4(c4),
        d1(d1), d2(d2), d3(d3), d4(d4) {}

QuaternionMatrix QuaternionMatrix::multiply(double x) {
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

// ----- ROTATION MATRIX -----

// OTHER ugly constructor
RotationMatrix::RotationMatrix(
        double a1, double a2, double a3,
        double b1, double b2, double b3,
        double c1, double c2, double c3) :
        a1(a1), a2(a2), a3(a3),
        b1(b1), b2(b2), b3(b3),
        c1(c1), c2(c2), c3(c3) {}

RotationMatrix RotationMatrix::multiply(double x) {
    return {
            a1 * x, a2 * x, a3 * x,
            a1 * x, a2 * x, a3 * x,
            a1 * x, a2 * x, a3 * x
    };
}

RotationMatrix RotationMatrix::multiply(const RotationMatrix &other) {
    return {
            a1 * other.a1 + a2 * other.b1 + a3 * other.c1,
            a1 * other.a2 + a2 * other.b2 + a3 * other.c2,
            a1 * other.a3 + a2 * other.b3 + a3 * other.c3,

            b1 * other.a1 + b2 * other.b1 + b3 * other.c1,
            b1 * other.a2 + b2 * other.b2 + b3 * other.c2,
            b1 * other.a3 + b2 * other.b3 + b3 * other.c3,

            c1 * other.a1 + c2 * other.b1 + c3 * other.c1,
            c1 * other.a2 + c2 * other.b2 + c3 * other.c2,
            c1 * other.a3 + c2 * other.b3 + c3 * other.c3
    };
}

Quaternion RotationMatrix::toQuaternion() {
    Quaternion q = Quaternion(0, 0, 0, 0);
    double t;
    if (c3 < 0)
    {
        if (a1 > b2)
        {
            t = 1 + a1 - b2 - c3;
            q = Quaternion(t, a2+b1, c1+a3, b3-c2);
        }
        else
        {
            t = 1 - a1 + b2 - c3;
            q = Quaternion(a2+b1, t, b3+c2, c1-a3);
        }
    }
    else
    {
        if (a1 < - b2)
        {
            t = 1 - a1 - b2 + c3;
            q = Quaternion(c1+a3, b3+c2, t, a2-b1);
        }
        else
        {
            t = 1 + a1 + b2 + c3;
            q = Quaternion(b3-c2, c1-a3, a2-b1, t);
        }
    }
    return q.multiply(0.5 / sqrt(t));
}

// ----- DOUBLE 3 -----

Double3::Double3() {}

Double3::Double3(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Double3 Double3::rotate(const RotationMatrix &matrix) {
    return Double3(
            matrix.a1 * x + matrix.a2 * y + matrix.a3 * z,
            matrix.b1 * x + matrix.b2 * y + matrix.b3 * z,
            matrix.c1 * x + matrix.c2 * y + matrix.c3 * z
    );
}

Double3 Double3::rotate(const Quaternion &quaternion) {
    Quaternion temp = quaternion;
    Quaternion result = temp.multiply(Quaternion(0, x, y, z)).multiply(temp.conjugate());

    return Double3(result.b, result.c, result.d);
}

Double3 Double3::crossProduct(const Double3 &other) {
    return Double3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.y
    );
}