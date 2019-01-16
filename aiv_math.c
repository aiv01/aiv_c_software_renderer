#include "aiv_math.h"

Vector3_t Vector3_new(float x, float y, float z)
{
    Vector3_t vector3 = {.x = x, .y = y, .z = z};
    return vector3;
}

Vector3_t Vector3_zero()
{
    Vector3_t vector3 = {0, 0, 0};
    return vector3;
}

float lerp(float start, float end, float gradient)
{
    return start + (end - start) * gradient;
}

float inversed_slope(float x0, float y0, float x1, float y1)
{
    return (x1 - x0) / (y1 - y0);
}

Vector3_t lerp3(Vector3_t start, Vector3_t end, float gradient)
{
    Vector3_t v;
    v.x = lerp(start.x, end.x, gradient);
    v.y = lerp(start.y, end.y, gradient);
    v.z = lerp(start.z, end.z, gradient);

    return v;
}

Vector3_t Vector3_sub(Vector3_t a, Vector3_t b)
{
    Vector3_t v;
    v.x = a.x - b.x;
    v.y = a.y - b.y;
    v.z = a.z - b.z;

    return v;
}

Vector3_t Vector3_add(Vector3_t a, Vector3_t b)
{
    Vector3_t v;
    v.x = a.x + b.x;
    v.y = a.y + b.y;
    v.z = a.z + b.z;

    return v;
}

void Vector3_print(Vector3_t v)
{
    printf("{%f, %f, %f}\n", v.x, v.y, v.z);
}

int clamp(int value, int _min, int _max)
{
    if (value < _min)
        return _min;

    if (value > _max)
        return _max;

    return value;
}

float linear_convert(float value, float old_min, float old_max, float new_min, float new_max)
{
    return (value - old_min) * (new_max - new_min) / (old_max - old_min) + new_min;
}

float Vector3_dot(Vector3_t a, Vector3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3_t Vector3_cross(Vector3_t a, Vector3_t b)
{
    return Vector3_new(a.y * b.z - a.z * b.y,
                       a.z * b.x - a.x * b.z,
                       a.x * b.y - a.y * b.x);
}