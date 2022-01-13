#ifndef RAY_TRACING_UTIL_H
#define RAY_TRACING_UTIL_H

#include <SFML/Graphics.hpp>
#include <cmath>
using namespace sf;

extern const float pi;
extern const float small_val;

struct sph_drct { float te, fi; };

int min(int x, int y);
int max(int x, int y);
float dot(Vector3f v1, Vector3f v2);
Vector3f cross(Vector3f v1, Vector3f v2);
float mod(Vector3f v);
Vector3f normalize(Vector3f v);
Vector3f sph_drct_to_vec(struct sph_drct sph_drct);
void change_sph_drct(struct sph_drct* sph_drct, float d_fi, float d_te);
float float_rand();
float binary_search_inverse(float (*func)(float), float y, float a, float b, float max_err);
float newton_search_inverse(float (*func)(float), float y, float a, float b, float max_err);
float volume_by_w(float w);

#endif
