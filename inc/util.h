#ifndef RAY_TRACING_UTIL_H
#define RAY_TRACING_UTIL_H

#include <SFML/Graphics.hpp>
using namespace sf;

extern float pi;

struct sph_drct { float te, fi; };

int min(int x, int y);
int max(int x, int y);
float dot(Vector3f v1, Vector3f v2);
Vector3f cross(Vector3f v1, Vector3f v2);
float mod(Vector3f v);
Vector3f normalize(Vector3f v);
Vector3f sph_drct_to_vec(struct sph_drct sph_drct);
void change_sph_drct(struct sph_drct* sph_drct, float d_fi, float d_te);

#endif
