#include "util.h"
#include <SFML/Graphics.hpp>
using namespace sf;

const float pi = 3.14159265f;
const float small_val = 0.0003f;

int min(int x, int y) { return x < y ? x : y; }
int max(int x, int y) { return x > y ? x : y; }

float dot(Vector3f v1, Vector3f v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3f cross(Vector3f v1, Vector3f v2) {
  return Vector3f(
    v1.y * v2.z - v1.z * v2.y,
    v1.x * v2.z - v1.z * v2.x,
    v1.x * v2.y - v1.y * v2.x
  );
}

float mod(Vector3f v) {
  return sqrt(dot(v, v));
}

Vector3f normalize(Vector3f v) {
  return v / mod(v);
}

Vector3f sph_drct_to_vec(struct sph_drct sph_drct) {
  float sin_te = sin(sph_drct.te);
  return Vector3f(sin_te * cos(sph_drct.fi), sin_te * sin(sph_drct.fi), cos(sph_drct.te));
}

void change_sph_drct(struct sph_drct* const sph_drct, float d_fi, float d_te) {
  sph_drct->te += d_te; sph_drct->fi += d_fi;
  if (sph_drct->te < 0) sph_drct->te = 0;
  if (sph_drct->te > pi) sph_drct->te = pi;
  if (sph_drct->fi < -pi) sph_drct->fi += 2 * pi;
  if (sph_drct->fi > pi) sph_drct->fi -= 2 * pi;
}

float float_rand() { return float(rand()) / RAND_MAX; }

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