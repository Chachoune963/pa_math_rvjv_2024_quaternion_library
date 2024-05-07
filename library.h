#ifndef QUATERNION_LIBRARY_H
#define QUATERNION_LIBRARY_H

struct Quaternion
{
public:
    float a, b, c, d;

    Quaternion(float a, float b, float c, float d);
    Quaternion add(const Quaternion& other);
    Quaternion multiply(float x);
    Quaternion mutliply(const Quaternion& other);
    Quaternion conjugate();
    Quaternion getUnit();
    float getNorm();
    float scalarProduct(const Quaternion& other);
    class QuaternionMatrix toMatrix();

private:
    Quaternion();
};

struct QuaternionMatrix
{
public:
    float a1, a2, a3, a4;
    float b1, b2, b3, b4;
    float c1, c2, c3, c4;
    float d1, d2, d3, d4;

    QuaternionMatrix(float a1, float a2, float a3, float a4, float b1, float b2, float b3, float b4, float c1, float c2,
                     float c3, float c4, float d1, float d2, float d3, float d4);

    QuaternionMatrix multiply(float x);
    QuaternionMatrix multiply(QuaternionMatrix other);

    Quaternion toQuaternion();

private:
    QuaternionMatrix() {};
};

#endif //QUATERNION_LIBRARY_H
