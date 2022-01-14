#include "util.h"
using namespace sf;


const float pi = 3.14159265f;
const float small_val = 0.0003f;


int min(int x, int y) { return x < y ? x : y; }
int max(int x, int y) { return x > y ? x : y; }


Vec4 sum(Vec4 v1, Vec4 v2) { return Vec4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w); }
Vec4 sum(Vec4 v1, Vec4 v2, Vec4 v3) { return sum(sum(v1, v2), v3); }
Vec4 mul_vn(Vec4 v, float l) { return Vec4(v.x * l, v.y * l, v.z * l, v.w * l); }
Vec4 neg(Vec4 v) { return mul_vn(v, -1); }
Vec4 dif(Vec4 v1, Vec4 v2) { return sum(v1, neg(v2)); }
Vec4 div_vn(Vec4 v, float l) { return mul_vn(v, 1 / l); }
float dot(Vec4 v1, Vec4 v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
float mod(Vec4 v) { return sqrt(dot(v, v)); }
Vec4 normalize(Vec4 v) { return div_vn(v, mod(v)); }


Vec4 sph_drct4_to_vec(struct sph_drct4 sph_drct) {
  float sin_psi = sin(sph_drct.psi);
  float sin_te = sin(sph_drct.te);
  return Vec4(
    sin_psi * sin_te * cos(sph_drct.fi),
    sin_psi * sin_te * sin(sph_drct.fi),
    sin_psi * cos(sph_drct.te),
    cos(sph_drct.psi)
  );
}

void change_sph_drct4(struct sph_drct4* const sph_drct, float d_psi, float d_te, float d_fi) {
  sph_drct->psi += d_psi;
  if (sph_drct->psi < 0) sph_drct->psi = 0;
  if (sph_drct->psi > pi) sph_drct->psi = pi;

  sph_drct->te += d_te;
  if (sph_drct->te < 0) sph_drct->te = 0;
  if (sph_drct->te > pi) sph_drct->te = pi;

  sph_drct->fi += d_fi;
  if (sph_drct->fi < -pi) sph_drct->fi += 2 * pi;
  if (sph_drct->fi > pi) sph_drct->fi -= 2 * pi;
}

Vec4 sph_drct3_to_vec(struct sph_drct3 sph_drct, struct section section) {
  float sin_te = sin(sph_drct.te);
  return sum(
    mul_vn(section.x, sin_te * cos(sph_drct.fi)),
    mul_vn(section.y, sin_te * sin(sph_drct.fi)),
    mul_vn(section.z, cos(sph_drct.te))
  );
}

void change_sph_drct3(struct sph_drct3* const sph_drct, float d_te, float d_fi) {
  sph_drct->te += d_te;
  if (sph_drct->te < -pi/2) sph_drct->te = -pi/2;
  if (sph_drct->te >  pi/2) sph_drct->te =  pi/2;

  sph_drct->fi += d_fi;
  if (sph_drct->fi < -pi) sph_drct->fi += 2 * pi;
  if (sph_drct->fi > pi) sph_drct->fi -= 2 * pi;
}


float binary_search_inverse(float (*func)(float), float y, float a, float b, float max_err) {
  int iterations_num = log2((b - a) / max_err) - 1;
  float x = (a + b) / 2;
  for (int i = 0; i < iterations_num; i++) {
    const float f = func(x);
    if (f == y) return x;
    if (f < y) a = x; else b = x;
    x = (a + b) / 2;
  }
  return x;
}

float newton_search_inverse(float (*func)(float), float y, float a, float b, float max_err) {
  float c = (a + b) / 2;
  float old_x; float new_x = c;
  do {
    old_x = new_x;
    float f = func(old_x);
    float df = old_x > c ? f - func(old_x - small_val) : func(old_x + small_val) - f;
    new_x = old_x - small_val / df * (f - y);
  } while (std::abs(new_x - old_x) >= max_err);
  return new_x;
}

float volume_by_w(float w) {
  return (w * sqrt(1 - w * w) - acos(w)) / pi + 1;
}

float w_by_volume(float v) {
  return newton_search_inverse(volume_by_w, v, 0, 1, small_val);
}