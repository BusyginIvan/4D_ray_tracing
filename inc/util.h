#ifndef RAY_TRACING_UTIL_H
#define RAY_TRACING_UTIL_H

#include <SFML/Graphics.hpp>
#include <cmath>
using namespace sf::Glsl;

extern const float pi;

int min(int x, int y);
int max(int x, int y);

Vec4 sum(Vec4 v1, Vec4 v2);
Vec4 sum(Vec4 v1, Vec4 v2, Vec4 v3);
Vec4 mul_vn(Vec4 v, float l);
Vec4 neg(Vec4 v);
Vec4 dif(Vec4 v1, Vec4 v2);
Vec4 div_vn(Vec4 v, float l);
float dot(Vec4 v1, Vec4 v2);
float mod(Vec4 v);
Vec4 normalize(Vec4 v);

#endif
