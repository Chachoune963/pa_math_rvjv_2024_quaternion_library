#ifndef QUATERNION_LIBRARY_H
#define QUATERNION_LIBRARY_H

struct Quaternion
{
public:
    double a, b, c, d;

    Quaternion(double a, double b, double c, double d);
    Quaternion add(const Quaternion& other);
    Quaternion multiply(double x);
    Quaternion multiply(const Quaternion& other);
    Quaternion conjugate();
    Quaternion getUnit();
    double getNorm();
    double scalarProduct(const Quaternion& other);
    class Double3 crossProduct(const Quaternion& other);

    static Quaternion eulerAngles(double rads, Double3 axis);

    class QuaternionMatrix toMatrix();
    class RotationMatrix getRotationMatrix();

private:
    Quaternion();
};

struct QuaternionMatrix
{
public:
    double a1, a2, a3, a4;
    double b1, b2, b3, b4;
    double c1, c2, c3, c4;
    double d1, d2, d3, d4;

    QuaternionMatrix(double a1, double a2, double a3, double a4, double b1, double b2, double b3, double b4, double c1, double c2,
                     double c3, double c4, double d1, double d2, double d3, double d4);

    QuaternionMatrix multiply(double x);
    QuaternionMatrix multiply(QuaternionMatrix other);

    Quaternion toQuaternion();

private:
    QuaternionMatrix() {};
};

struct RotationMatrix
{
public:
    double a1, a2, a3;
    double b1, b2, b3;
    double c1, c2, c3;

    RotationMatrix(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3);

    RotationMatrix multiply(double x);
    RotationMatrix multiply(const RotationMatrix& other);

    Quaternion toQuaternion();

private:
    RotationMatrix();
};

// Basic Vector3 structure for demonstration purposes
struct Double3
{
public:
    double x, y, z;

    Double3();
    Double3(double x, double y, double z);

    Double3 multiply(double x);

    Double3 getUnit();
    Double3 rotate(const class RotationMatrix& matrix);
    Double3 rotate(const class Quaternion& quaternion);

    Double3 crossProduct(const Double3& other);

    double getNorm();
};

#endif //QUATERNION_LIBRARY_H
