#ifndef RAY_TRACING_UTIL_H
#define RAY_TRACING_UTIL_H

#include <SFML/Graphics.hpp>
#include <cmath>

using namespace sf::Glsl;

extern const float pi;
extern const float small_val;

struct sph_drct4 { float psi, te, fi; };
struct sph_drct3 { float te, fi; };

struct section { Vec4 x, y, z; };

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

Vec4 sph_drct3_to_vec(struct sph_drct3 sph_drct, struct section section);
void change_sph_drct3(struct sph_drct3* sph_drct, float d_te, float d_fi);
Vec4 sph_drct4_to_vec(struct sph_drct4 sph_drct);
void change_sph_drct4(struct sph_drct* sph_drct, float d_psi, float d_te, float d_fi);

float binary_search_inverse(float (*func)(float), float y, float a, float b, float max_err);
float newton_search_inverse(float (*func)(float), float y, float a, float b, float max_err);
float volume_by_w(float w);
float w_by_volume(float v);

#endif
