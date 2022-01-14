#version 450

// Разное.

const float pi = 3.14159265f;
const float small_val = 0.0003;   // Маленькая величина. Примерно равна 2^(-12).

vec2 scr_coord;                   // scr (screen) – экран. Координата пикселя в окне.

float v_cos(vec4 v1, vec4 v2) {   // Косинус угла между векторами.
  return dot(v1, v2) / length(v1) / length(v2);
}

float angle(vec4 v1, vec4 v2) {       // Угол между векторами.
  return acos(v_cos(v1, v2));
}

vec4 redirect(vec4 vec, vec4 norm) {  // Отражение вектора, если он смотрит внутрь поверхности.
  float dot = dot(vec, norm);
  return dot >= 0 ? vec : vec - 2 * dot * norm;
}


// Преобразования координат.

// drct (direct) – направление; единичный вектор.
// sph (sphere) – сфера, сферические координаты.
struct sph_drct { float te, fi; };

vec3 sph_drct_to_vec(sph_drct sph_drct) {  // Преобразование к обычному вектору.
  float sin_te = sin(sph_drct.te);
  return vec3(sin_te * cos(sph_drct.fi), sin_te * sin(sph_drct.fi), cos(sph_drct.te));
}

struct cyl_vec { float z, r, fi; };       // cyl (cylinder) – цилиндрические координаты.

vec3 cyl_vec_to_vec(cyl_vec cyl_vec) {    // Преобразование к обычному вектору.
  return vec3(cyl_vec.r * cos(cyl_vec.fi), cyl_vec.r * sin(cyl_vec.fi), cyl_vec.z);
}


// Псевдорандом.

uniform int seed;   // Рандомное число передаётся извне для каждого кадра.
uint rand_iter = 0; // Для большей хаотичности каждое вычисление рандомного числа делается уникальным.

uint hash(uint x) {
  x += rand_iter++;
  x += ( x << 10u );
  x ^= ( x >>  6u );
  x += ( x <<  3u );
  x ^= ( x >> 11u );
  x += ( x << 15u );
  return x;
}
uint hash(uvec2 v) { return hash( v.x ^ hash(v.y) ); }

float floatConstruct(uint m) {
  const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
  const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32
  m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
  m |= ieeeOne;                          // Add fractional part to 1.0
  float  f = uintBitsToFloat(m);         // Range [1:2]
  return f - 1.0;                        // Range [0:1]
}

// Рандомное число по вектору. По умолчанию используются координаты пикселя.
float rand(vec2 v)  { return floatConstruct(hash(floatBitsToUint(v))); }
float rand() { return rand(scr_coord + seed); }

// Случайный исход.
bool rand_outcome(float probability) { return rand() > probability ? false : true; }

// Случайная точка на сфере. Используется развёртка сферы в цилиндр.
vec3 rand_drct() {
  float z = rand() * 2 - 1;
  return cyl_vec_to_vec(cyl_vec(z, sqrt(1 - z * z), rand() * 2 * pi));
}


// Случайная точка на гиперсфере.

// Вычисляет долю объёма гиперсферы по одно сторону от значения координаты w.
float volume_by_w(float w) {
  return (w * sqrt(1 - w * w) - acos(w)) / pi + 1;
}

// Вычисляет четвёртую координату w по доле объёма гиперсферы по одну сторону от этой координаты.
float w_by_volume(float v) {
  float old_w; float new_w = 0;
  do {
    old_w = new_w;
    float old_v = volume_by_w(old_w);
    float df = old_w > 0 ? old_v - volume_by_w(old_w - small_val) : volume_by_w(old_w + small_val) - old_v;
    new_w = old_w - small_val / df * (old_v - v);
  } while (abs(new_w - old_w) >= small_val);
  return new_w;
}

// Случайная точка на единичной гиперсфере.
vec4 rand_drct4() {
  float w = w_by_volume(rand());
  float r = sqrt(1 - w * w);
  float z = (rand() * 2 - 1) * r;
  return vec4(cyl_vec_to_vec(cyl_vec(z, sqrt(r * r - z * z), rand() * 2 * pi)), w);
}


// Характеристики материала и результат пересечения с лучом.

struct properties {
  float glow;         // Доля испускаемого света в противовес отражаемому.
  float refl_prob;    // Вероятность зеркального отражения луча.
  vec3 color;         // Цвет. Остальной свет поглащается.
};

const properties null_properties = properties(0, 0, vec3(0));

struct intersection {
  bool valid;        // Было ли пересечение.
  float dist;        // Расстояние, пройденное лучом до пересечения с объектом.
  vec4 norm;         // Нормаль к поверхности в точке пересечения.
  properties prop;   // Характеристики материала объекта.
};

const intersection no_intersection = intersection(false, 0, vec4(0), null_properties);

// Определение ближайшего пересечения.
intersection near(intersection int1, intersection int2) {
  if (int1.valid) {
    if (int2.valid) return int1.dist < int2.dist ? int1 : int2;
    else return int1;
  } else return int2.valid ? int2 : no_intersection;
}


// Сфера.

struct sphere {
  vec4 coord;       // Координаты сферы.
  float r;          // Радиус сферы.
  properties prop;  // Характеристики материала.
};

// Пересечение луча со сферой. Принимает координату начала полёта луча и его направление.
intersection sphere_intersection(sphere sphere, vec4 coord, vec4 drct) {
  vec4 vec = sphere.coord - coord;                   // Вектор к центру сферы.
  float dot = dot(vec, drct);
  if (dot <= 0) return no_intersection;
  float length_v = length(vec);                      // Расстояние до центра сферы.
  if (length_v <= sphere.r) return no_intersection;
  float cos_vd = dot / length_v;                     // Косинус угла между vec и drct (лучом).
  float sin_vd = sqrt(1 - cos_vd * cos_vd);          // Синус угла между vec и drct.
  float sin_rd = sin_vd / sphere.r * length_v;       // Синус угла между радиусом и лучом.
  if (sin_rd >= 1) return no_intersection;
  float angle_rv = asin(sin_rd) - acos(cos_vd);      // Угол между радиусом и vec.
  float dist = sqrt(sphere.r * sphere.r + length_v * length_v - 2 * sphere.r * length_v * cos(angle_rv));
  //if (dist <= 0) return no_intersection;
  vec4 norm = normalize(coord + drct * dist - sphere.coord);
  return intersection(true, dist, norm, sphere.prop);
}


// Плоскость.

struct space {
  vec4 point;       // Точка на плоскости.
  vec4 norm;        // Нормаль к плоскости.
  properties prop;  // Характеристики материала.
};

// Пересечение луча с плоскостью. Принимает координату начала полёта луча и его направление.
intersection plane_intersection(space space, vec4 coord, vec4 drct) {
  vec4 vec = space.point - coord;
  float dot_vn = dot(vec, space.norm);      // Расстояние до плоскости (со знаком).
  vec4 drct_h = space.norm * sign(dot_vn);  // Единичный вектор в сторону плоскости.
  float cos_dh = dot(drct_h, drct);         // Косинус угла между этим вектором и лучём.
  if (cos_dh <= 0) return no_intersection;  // Если луч летит от плоскости, пересечения нет.
  float dist = abs(dot_vn) / cos_dh;
  return intersection(true, dist, -drct_h, space.prop);
}


// Инициализация объектов сцены.

// В этот цвет в итоге окрашиваются все лучи, улетевшие в пустоту.
//const vec3 sky_color = vec3(0.01, 0.01, 0.03);
//const vec3 sky_color = vec3(0, 0, 0);
const vec3 sky_color = vec3(0.001, 0.001, 0.002);

const space[1] spaces = space[1](
  //plane(vec3(0, 0, -1.5), vec3(0, 0, 1), properties(0, 0.5, vec3(100.0f/255, 55.0f/255, 36.0f/255)))
  space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0), properties(0, 0.9, vec3(1.0, 0.2, 0.2)))
);

const sphere[2] spheres = sphere[2](
  //sphere(vec3(0, 0, 0), 1, properties(0, 0.5, vec3(1.0, 0.5, 0.2))),
  sphere(vec4(0, 0, 0, 0), 1, properties(0, 0.8, vec3(0.2, 1.0, 0.2))),
  sphere(vec4(2, 0, 0, 0), 0.5, properties(1, 0, vec3(1, 1, 1)))
);


// Трассировка луча.

const uint reflections_number = 6;   // Максимальное количество переотражений.

// Трассировка луча. Возвращает цвет.
vec3 trace(vec4 coord, vec4 drct) {
  vec3 res_color = vec3(0);  // res (result) – свет, полученный в сумме от источников или неба.
  vec3 rem_color = vec3(1);  // rem (remaining) – ещё не поглощённый свет, оставшийся в луче.
  for (int i = 0; i < reflections_number; i++) {
    // Поиск ближайшего пересечения с объектом.
    intersection inter = no_intersection;
    for (int i = 0; i < spheres.length(); i++)
      inter = near(inter, sphere_intersection(spheres[i], coord, drct));
    for (int i = 0; i < spaces.length(); i++)
      inter = near(inter, plane_intersection(spaces[i], coord, drct));

    if (!inter.valid) {  // Если нет пересечения, добавляем цвет неба.
      res_color += rem_color * sky_color;
      return res_color;
    }

    rem_color = rem_color * inter.prop.color;    // Поглощение света.
    res_color += rem_color * inter.prop.glow;    // Учёт свечения объекта.
    if (inter.prop.glow == 1) return res_color;
    rem_color *= 1 - inter.prop.glow;            // Вычисляем долю отражённого света.

    // Новая точка начала луча. С небольшим отступом, чтобы не попадать внутрь объекта.
    coord += drct * inter.dist + inter.norm * small_val;

    // Отражение или случайное направление луча.
    if (rand() > inter.prop.refl_prob)
      drct = redirect(rand_drct4(), inter.norm);
    else
      drct = reflect(drct, inter.norm);
    //drct = normalize(drct);
  }
  return res_color; // При достижении максимально числа отражений нового света не добавляется. Тень.
}


// Главный метод. Основная работа c экраном и матрицей.

uniform vec2 resolution; // Ширина и высота экрана (окна) в пикселях.
// mtr (matrix) – матрица, как у фотоаппарата; виртуальное представление нашего экрана в пространстве.
uniform vec2 mtr_sizes; // Ширина и высота матрицы в единицах пространства сцены.

uniform vec4 focus; // Фокус; точка схождения лучей за матрицей. В фотоаппарате он перед матрицей,
// а у нас лучи как бы из этой точки летят.
uniform vec4 vec_to_mtr;            // Вектор от фокуса до середины матрицы.
uniform vec4 top_drct, right_drct;  // Единичные векторы по направлению вверх и вправо для наблюдателя.

// Изначальное направление полёта луча: от фокуса через точку на матрице.
vec4 ray_drct() {
  vec2 mtr_coord = vec2((scr_coord.x * 2 - 1) * mtr_sizes.x, (1 - scr_coord.y * 2) * mtr_sizes.y);
  vec4 ray_drct = vec_to_mtr + top_drct * mtr_coord.y + right_drct * mtr_coord.x;
  return normalize(ray_drct);
}

// Преобразование цвета. Фильтр, чтобы не было темно.
const float c = 120; // С увеличением этой константы усиливается эффект.
vec3 tone_mapping(vec3 color) {
  return (color * c * (1 + color / c)) / (1 + color * c);
}

uniform sampler2D old_frame;  // Информация о предыдущем кадре.
uniform float part;           // Доля текущего кадра в результирующем изображении.

void main() {
  // Координата на экране.
  scr_coord = gl_FragCoord.xy;
  scr_coord = vec2(scr_coord.x / resolution.x, scr_coord.y / resolution.y);

  vec3 new_color = vec3(0);
  int samples = 60;           // Число запускаемых лучей. С одним лучом фильтр цвета не имел бы смысла.
  vec4 ray_drct = ray_drct();
  for(int i = 0; i < samples; i++)
    new_color += trace(focus, ray_drct);
  new_color /= samples;

  new_color = tone_mapping(new_color); // Применение фильтра.

  vec3 old_color = texture(old_frame, scr_coord).rgb;       // Старый цвет пикселя.
  gl_FragColor = vec4(mix(old_color, new_color, part), 1);  // Смешиваем старый цвет с новым по пропорции.
}