#include "util.h"
#include <SFML/Graphics.hpp>
#include <cmath>
using namespace sf;

float pi = 3.14159265f;

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