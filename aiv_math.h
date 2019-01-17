#include <math.h>
#include <stdio.h>

typedef struct Vector3
{
    float x;
    float y;
    float z;
} Vector3_t;

Vector3_t Vector3_new(float x, float y, float z);

Vector3_t Vector3_zero();

Vector3_t Vector3_add(Vector3_t a, Vector3_t b);
Vector3_t Vector3_mul(Vector3_t a, float b);
Vector3_t Vector3_normalized(Vector3_t a);
Vector3_t Vector3_sub(Vector3_t a, Vector3_t b);
float Vector3_dot(Vector3_t a, Vector3_t b);
Vector3_t Vector3_cross(Vector3_t a, Vector3_t b);
void Vector3_print(Vector3_t v);

float lerp(float start, float end, float gradient);
Vector3_t lerp3(Vector3_t start, Vector3_t end, float gradient);

int clamp(int value, int _min, int _max);
float clampf(float value, float _min, float _max);

float linear_convert(float value, float old_min, float old_max, float new_min, float new_max);

Vector3_t Vector3_roty(Vector3_t a, float r);

Vector3_t Vector3_reflect(Vector3_t a, Vector3_t n);