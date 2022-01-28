#include <SFML/Graphics.hpp>
#include <cmath>
using namespace sf::Glsl;

Vec4 sum(const Vec4 v1, const Vec4 v2) { return Vec4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w); }
Vec4 sum(const Vec4 v1, const Vec4 v2, Vec4 v3) { return sum(sum(v1, v2), v3); }
Vec4 mulVN(Vec4 v, float l) { return Vec4(v.x * l, v.y * l, v.z * l, v.w * l); }
Vec4 neg(const Vec4 v) { return mulVN(v, -1); }
Vec4 dif(const Vec4 v1, const Vec4 v2) { return sum(v1, neg(v2)); }
Vec4 divVN(const Vec4 v, const float l) { return mulVN(v, 1 / l); }
float dot(const Vec4 v1, const Vec4 v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
float mod(const Vec4 v) { return sqrt(dot(v, v)); }
Vec4 normalize(const Vec4 v) { return divVN(v, mod(v)); }