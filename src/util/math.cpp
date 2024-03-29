#include "util/math.h"
#include <cmath>

using namespace std;

Vec4 sum(const Vec4 v1, const Vec4 v2) { return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w }; }
Vec4 sum(const Vec4 v1, const Vec4 v2, const Vec4 v3) { return sum(sum(v1, v2), v3); }
Vec4 mulVN(Vec4 v, float l) { return { v.x * l, v.y * l, v.z * l, v.w * l }; }
Vec4 neg(const Vec4 v) { return mulVN(v, -1); }
Vec4 dif(const Vec4 v1, const Vec4 v2) { return sum(v1, neg(v2)); }
Vec4 divVN(const Vec4 v, const float l) { return mulVN(v, 1 / l); }
float dot(const Vec4 v1, const Vec4 v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
float mod(const Vec4 v) { return sqrt(dot(v, v)); }
Vec4 normalize(const Vec4 v) { return divVN(v, mod(v)); }

float min(const float a, const float b, const float c, const float d) { return min(min(a, b), min(c, d)); }
float min(const float a, const float b, const float c) { return min(min(a, b), c); }

void pullIntoRange(float& f, const float center, const float r) {
  if (f < center - r) f = center - r;
  if (f > center + r) f = center + r;
}

void normalizeAngle(float& angle) {
  angle = remainder(angle, 2 * PI);
  if (angle < -PI) angle += 2 * PI;
  if (angle >  PI) angle -= 2 * PI;
}
float convertDegreesToRadians(const float degrees) { return degrees / 180 * PI; }