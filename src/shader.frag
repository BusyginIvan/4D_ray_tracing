#version 450

const float pi = 3.14159265f;
const float small_val = 0.0003;   // Маленькая величина. Примерно равна 2^(-12).

vec2 scr_coord;                   // scr (screen) – экран. Координата пикселя в окне.


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


// Цилиндрические координаты.

struct cyl_vec { float z, r, fi; };       // cyl (cylinder) – цилиндрические координаты.

vec3 cyl_vec_to_vec(cyl_vec cyl_vec) {    // Преобразование к обычному вектору.
  return vec3(cyl_vec.r * cos(cyl_vec.fi), cyl_vec.r * sin(cyl_vec.fi), cyl_vec.z);
}


// Случайная точка на гиперсфере.

// Вычисляет долю объёма гиперсферы по одну сторону от значения координаты w.
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

// Случайная точка на единичной гиперсфере. drct (direct) – направление; единичный вектор.
vec4 rand_drct() {
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


// Гиперcфера.

struct sphere {
  vec4 coord;       // Координаты гиперсферы.
  float r;          // Радиус гиперсферы.
  properties prop;  // Характеристики материала.
};

// Пересечение луча с гиперсферой. Принимает координату начала полёта луча и его направление.
intersection sphere_intersection(sphere sphere, vec4 coord, vec4 drct) {
  vec4 vec = sphere.coord - coord;                   // Вектор к центру гиперсферы.
  float dot = dot(vec, drct);
  if (dot <= 0) return no_intersection;
  float length_v = length(vec);                      // Расстояние до центра гиперсферы.
  if (length_v <= sphere.r) return no_intersection;
  float cos_vd = dot / length_v;                     // Косинус угла между vec и drct (лучом).
  float sin_vd = sqrt(1 - cos_vd * cos_vd);          // Синус угла между vec и drct.
  float sin_rd = sin_vd / sphere.r * length_v;       // Синус угла между радиусом и лучом.
  if (sin_rd >= 1) return no_intersection;
  float angle_rv = asin(sin_rd) - acos(cos_vd);      // Угол между радиусом и vec.
  float dist = sqrt(sphere.r * sphere.r + length_v * length_v - 2 * sphere.r * length_v * cos(angle_rv));
  vec4 norm = normalize(coord + drct * dist - sphere.coord);
  return intersection(true, dist, norm, sphere.prop);
}


// Пространство (трёхмерное).

struct space {
  vec4 point;       // Точка на плоскости.
  vec4 norm;        // Нормаль к плоскости.
  properties prop;  // Характеристики материала.
};

// Пересечение луча с пространством. Принимает координату начала полёта луча и его направление.
intersection space_intersection(space space, vec4 coord, vec4 drct) {
  vec4 vec = space.point - coord;
  float dot_vn = dot(vec, space.norm);      // Расстояние до пространства (со знаком).
  vec4 drct_h = space.norm * sign(dot_vn);  // Единичный вектор в сторону пространства.
  float cos_dh = dot(drct_h, drct);         // Косинус угла между этим вектором и лучём.
  if (cos_dh <= 0) return no_intersection;  // Если луч летит от пространства, пересечения нет.
  float dist = abs(dot_vn) / cos_dh;
  return intersection(true, dist, -drct_h, space.prop);
}


// Солнце.
struct sun_properties {
  vec4 drct;
  float angular_size;
  vec3 color;
};


// Инициализация объектов сцены.

const vec3 sky_color = vec3(0.001, 0.001, 0.003);

const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), pi / 25, vec3(1, 1, 0.1));

const space[1] spaces = space[1](
  space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0), properties(0, 0, vec3(0.6, 0.4, 0.2)))
);

const sphere[2] spheres = sphere[2](
  sphere(vec4(0, 0, 0, 0), 1.0 , properties(0, 0.7, vec3(0.2, 1.0, 0.2))),
  sphere(vec4(2, 0, 0, 0), 0.5 , properties(1, 0.0, vec3(1, 1, 1)))
);


// Трассировка луча.

const uint reflections_number = 6;   // Максимальное количество переотражений.

vec4 redirect(vec4 vec, vec4 norm) {  // Отражение вектора, если он смотрит внутрь поверхности.
  float dot = dot(vec, norm);
  return dot >= 0 ? vec : vec - 2 * dot * norm;
}

float v_cos(vec4 v1, vec4 v2) {       // Косинус угла между векторами.
  return dot(v1, v2) / length(v1) / length(v2);
}

float angle(vec4 v1, vec4 v2) {       // Угол между векторами.
  return acos(v_cos(v1, v2));
}

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
      inter = near(inter, space_intersection(spaces[i], coord, drct));

    if (!inter.valid) {  // Если нет пересечения, добавляем цвет неба.
      if (angle(drct, sun.drct) < sun.angular_size)
        res_color += rem_color * sun.color;
      else
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
      drct = redirect(rand_drct(), inter.norm);
    else
      drct = reflect(drct, inter.norm);
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
const float c = 200; // С увеличением этой константы усиливается эффект.
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
  int samples = 70;           // Число запускаемых лучей. С одним лучом фильтр цвета не имел бы смысла.
  vec4 ray_drct = ray_drct();
  for(int i = 0; i < samples; i++)
    new_color += trace(focus, ray_drct);
  new_color /= samples;

  new_color = tone_mapping(new_color); // Применение фильтра.

  vec3 old_color = texture(old_frame, scr_coord).rgb;       // Старый цвет пикселя.
  gl_FragColor = vec4(mix(old_color, new_color, part), 1);  // Смешиваем старый цвет с новым по пропорции.
}