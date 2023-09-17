#ifndef RAY_TRACING_MATH_H
#define RAY_TRACING_MATH_H

#include <SFML/Graphics.hpp>

using namespace sf::Glsl;

const float PI = 3.14159265f;
const float GOLDEN = 1.61803399f;

Vec4 sum(Vec4 v1, Vec4 v2);
Vec4 sum(Vec4 v1, Vec4 v2, Vec4 v3);
Vec4 mulVN(Vec4 v, float l);
Vec4 neg(Vec4 v);
Vec4 dif(Vec4 v1, Vec4 v2);
Vec4 divVN(Vec4 v, float l);
float dot(Vec4 v1, Vec4 v2);
float mod(Vec4 v);
Vec4 normalize(Vec4 v);

float min(float a, float b, float c, float d);
float min(float a, float b, float c);
void pullIntoRange(float& f, float center, float r);

void normalizeAngle(float& angle);
float convertDegreesToRadians(const float degrees);

#endif
