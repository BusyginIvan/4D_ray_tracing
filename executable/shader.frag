#version 330

// Значения, передаваемые в шейдер нашей программой

uniform int seed; // Рандомное число, получаемое извне для каждого кадра

uniform int reflections_amount; // Максимальное число переотражений
uniform int samples; // Число запускаемых для каждой клетки экрана лучей

uniform float light_to_color_conversion_coefficient; // Чем больше эта константа, тем будет светлее

uniform vec2 resolution;     // Ширина и высота экрана (окна) в пикселях
uniform sampler2D old_frame; // Информация о предыдущем кадре
uniform float part;          // Доля текущего кадра в результирующем изображении

// mtr (matrix) – матрица, как у фотоаппарата; виртуальное представление нашего экрана в пространстве
uniform vec2 mtr_sizes; // Ширина и высота матрицы в единицах пространства сцены
uniform vec4 focus; // Фокус – точка схождения лучей за матрицей: в фотоаппарате он перед матрицей, а у нас лучи от этой точки летят
uniform vec4 vec_to_mtr; // Вектор от фокуса до середины матрицы
uniform vec4 top_drct, right_drct; // Единичные векторы по направлению вверх и вправо для наблюдателя


// Константы
const float PI = 3.14159265f;
const float SMALL_FLOAT = 0.0003f; // Маленькая величина: примерно равна 2^(-12)


// Переменная вынесена сюда, так как используется в том числе при генерации псевдорандомных чисел
vec2 scr_coord; // Координата пикселя в окне (scr - screen)


// Геометрические объекты
struct line   { vec4 point, drct; };
struct ray    { vec4 point, drct; };
struct space  { vec4 point, norm; };
struct sphere { vec4 center; float r; };


// Вспомогательные функции

// Синус по косинусу или наоборот
float cos_to_sin(float cos) { return sqrt(1 - cos * cos); }
float sin_to_cos(float sin) { return sqrt(1 - sin * sin); }

// Косинус угла между векторами
float v_cos(vec4 v1, vec4 v2) {
  return dot(v1, v2) / length(v1) / length(v2);
}

// Угол между векторами
float angle(vec4 v1, vec4 v2) { return acos(v_cos(v1, v2)); }

// Убираем из вектора составляющую, коллинеарную направлению
vec4 vec_in_space(vec4 vec, vec4 norm) { return vec - norm * dot(vec, norm); }
vec4 vec_in_space(vec4 vec, space space) { return vec_in_space(vec, space.norm); }

// Вектор от точки к прямой
vec4 vec_to_line(vec4 point, line line) {
  vec4 v1 = line.point - point;
  vec4 v2 = -dot(v1, line.drct) * line.drct;
  return v1 + v2;
}

// Вектор от точки к пространству
vec4 vec_to_space(vec4 point, space space) {
  return space.norm * dot(space.point - point, space.norm);
}

// Проецируем точку на просранство
vec4 point_in_space(vec4 point_in_hyperspace, space space) {
  return point_in_hyperspace + vec_to_space(point_in_hyperspace, space);
}

// Проецируем луч на пространство. Но поле drct перестаёт быть единичным вектором.
ray ray_in_space(ray ray_in_hyperspace, space space) {
  return ray(
    point_in_space(ray_in_hyperspace.point, space),
    vec_in_space(ray_in_hyperspace.drct, space)
  );
}

// Отражение вектора, если он смотрит внутрь поверхности
vec4 redirect(vec4 vec, vec4 norm) {
  float dot = dot(vec, norm);
  return dot >= 0 ? vec : vec - 2 * dot * norm;
}


// Псевдорандом

uint uint_seed = uint(seed);
// Для большей хаотичности каждое вычисление рандомного числа делается уникальным
uint rand_iter_seed = uint_seed;

uint hash(uint x) {
  x += ( x << 10 );
  x ^= ( x >>  6 );
  x += ( x <<  3 );
  x ^= ( x >> 11 );
  x += ( x << 15 );
  x ^= ( x >>  9 );
  return x;
}

uint random_uint() {
  rand_iter_seed += 0x79A010A9u;
  uvec2 v2 = floatBitsToUint(scr_coord);
  return hash(v2.x ^ (v2.y << 9) ^ rand_iter_seed ^ uint_seed);
}

// Псевдорандомное число от 0 до 1
float rand() {
  uint bits = random_uint();
  const uint ieeeMantissa = 0x007FFFFFu;
  const uint ieeeOne      = 0x3F800000u;
  bits &= ieeeMantissa;
  bits |= ieeeOne;
  return uintBitsToFloat(bits) - 1.0;
}

// Случайный исход
bool rand_outcome(float probability) { return rand() > probability ? false : true; }


// Цилиндрические координаты

struct cyl_vec { float z, r, fi; };      // cyl (cylinder) – цилиндрические координаты

vec3 cyl_vec_to_vec(cyl_vec cyl_vec) {   // Преобразование к обычному вектору
  return vec3(cyl_vec.r * cos(cyl_vec.fi), cyl_vec.r * sin(cyl_vec.fi), cyl_vec.z);
}


// Случайная точка на гиперсфере

// Вычисляет долю объёма гиперсферы по одну сторону от значения координаты w.
float volume_by_w(float w) {
  return (w * sqrt(1 - w * w) - acos(w)) / PI + 1;
}

// Вычисляет четвёртую координату w по доле объёма гиперсферы по одну сторону от этой координаты.
float w_by_volume(float v) {
  float old_w; float new_w = 0;
  do {
    old_w = new_w;
    float old_v = volume_by_w(old_w);
    float df = old_w > 0 ? old_v - volume_by_w(old_w - SMALL_FLOAT) : volume_by_w(old_w + SMALL_FLOAT) - old_v;
    new_w = old_w - SMALL_FLOAT / df * (old_v - v);
  } while (abs(new_w - old_w) >= SMALL_FLOAT);
  return new_w;
}

// Случайная точка на единичной гиперсфере. drct (direct) – направление; единичный вектор.
vec4 rand_drct() {
  float w = w_by_volume(rand());
  float r = sqrt(1 - w * w);
  float z = (rand() * 2 - 1) * r;
  return vec4(cyl_vec_to_vec(cyl_vec(z, sqrt(r * r - z * z), rand() * 2 * PI)), w);
}


// Характеристики материала и результат пересечения с лучом

struct material {
  float glow;      // Сила свечения
  float refl_prob; // Вероятность зеркального отражения луча
  vec3  color;     // Цвет: доли непоглощаемого цвета, а также множители для свечения
};

const material NULL_MATERIAL = material(0, 0, vec3(0));

struct intersection {
  bool  did_intersect; // Было ли пересечение
  float dist;          // Расстояние, пройденное лучом до пересечения с объектом
  vec4  norm;          // Нормаль к поверхности в точке пересечения
  material material;   // Характеристики материала объекта
};

const intersection NOT_INTERSECT = intersection(false, 0, vec4(0), NULL_MATERIAL);

// Определение ближайшего пересечения
intersection closest(intersection inter1, intersection inter2) {
  if (!inter1.did_intersect) return inter2;
  if (!inter2.did_intersect) return inter1;
  return inter1.dist < inter2.dist ? inter1 : inter2;
}


// Гиперcфера
struct visible_sphere {
  sphere figure;
  material material;
};

// Пересечение луча с гиперсферой
// outer: если false, луч, летящий снаружи, пролетит переднюю стенку насквозь.
// Обозначения: o - центр сферы, p - ray.point, a - точка пересечения.
intersection sphere_intersection(visible_sphere sphere, ray ray, bool outer) {
  vec4 vec_po = sphere.figure.center - ray.point;
  float len_po = length(vec_po);
  float r = sphere.figure.r;
  float cos_opa;
  if (len_po < SMALL_FLOAT) {
    cos_opa = 0;
  } else {
    float dot_pord = dot(vec_po, ray.drct);
    if (len_po >= r && dot_pord < 0) return NOT_INTERSECT;
    cos_opa = dot_pord / len_po;
    if (cos_opa > 1) cos_opa = 1;
    if (cos_opa < -1) cos_opa = -1;
  }
  float angle_opa = acos(cos_opa);
  float sin_oap = len_po * sin(angle_opa) / r;
  if (sin_oap >= 1) return NOT_INTERSECT;
  float angle_oap = asin(sin_oap);
  if (outer && len_po > r) angle_oap = PI - angle_oap;
  float angle_aop = PI - angle_opa - angle_oap;
  float dist = sqrt(r * r + len_po * len_po - 2 * r * len_po * cos(angle_aop));
  vec4 norm = (sphere.figure.center - (ray.point + ray.drct * dist)) / r;
  if (outer && len_po > r) norm *= -1;
  return intersection(true, dist, norm, sphere.material);
}


// Пространство (трёхмерное)
struct visible_space {
  space figure;
  material material;
};

// Пересечение луча с пространством
intersection space_intersection(visible_space space, ray ray) {
  vec4 vec_v = space.figure.point - ray.point;
  float dot_vn = dot(vec_v, space.figure.norm);   // Расстояние до пространства (со знаком)
  vec4 drct_h = space.figure.norm * sign(dot_vn); // Единичный вектор в сторону пространства
  float cos_dh = dot(drct_h, ray.drct);           // Косинус угла между этим вектором и лучём
  if (cos_dh < SMALL_FLOAT) return NOT_INTERSECT; // Если луч летит от пространства, пересечения нет
  float dist = abs(dot_vn) / cos_dh;
  return intersection(true, dist, -drct_h, space.material);
}


// Цилиндр, бесконечный по двум направлениям
struct visible_cylinder {
  vec4 point, axis1, axis2;
  float r;
  material material;
};

// Пересечение с цилиндром
// outer: если false, луч, летящий снаружи, пролетит переднюю стенку насквозь.
intersection cylinder_intersection(visible_cylinder cylinder, ray ray_in_hyperspace, bool outer) {
  ray ray_in_space1 = ray_in_space(ray_in_hyperspace, space(cylinder.point, cylinder.axis1));
  if (length(ray_in_space1.drct) < SMALL_FLOAT) return NOT_INTERSECT;

  ray ray_in_plane12 = ray_in_space(ray_in_space1, space(cylinder.point, cylinder.axis2));
  float drct_in_plane_length = length(ray_in_plane12.drct);
  if (drct_in_plane_length < SMALL_FLOAT) return NOT_INTERSECT;
  ray_in_plane12.drct /= drct_in_plane_length;

  intersection inter = sphere_intersection(
    visible_sphere(sphere(cylinder.point, cylinder.r), cylinder.material),
    ray_in_plane12,
    outer
  );
  inter.dist /= drct_in_plane_length;
  return inter;
}

// Расстояние до плоскости осей цилиндра
float dist_to_axes_plane(float dist, ray ray, visible_cylinder cylinder) {
  vec4 point_in_hyperspace = ray.point + ray.drct * dist;
  vec4 point_in_space1 = point_in_space(point_in_hyperspace, space(cylinder.point, cylinder.axis1));
  vec4 point_in_plane12 = point_in_space(point_in_space1, space(cylinder.point, cylinder.axis2));
  return length(cylinder.point - point_in_plane12);
}


// Четырёхмерный цилиндр, полученный пересечением двух бесконечных цилиндров
struct visible_cylinders_union {
  visible_cylinder cylinder1, cylinder2;
};

// Пересечение с объединением цилиндров
intersection cylinders_union_intersection(visible_cylinders_union cylinders_union, ray ray) {
  intersection inter1 = cylinder_intersection(cylinders_union.cylinder1, ray, true);
  if (dist_to_axes_plane(inter1.dist, ray, cylinders_union.cylinder2) > cylinders_union.cylinder2.r)
    inter1 = NOT_INTERSECT;

  intersection inter2 = cylinder_intersection(cylinders_union.cylinder2, ray, true);
  if (dist_to_axes_plane(inter2.dist, ray, cylinders_union.cylinder1) > cylinders_union.cylinder2.r)
    inter2 = NOT_INTERSECT;

  return closest(inter1, inter2);
}


// Tiger. Так обычно называют фигуру в четырёхмерном пространстве, очень похожую на эту.
struct visible_tiger {
  visible_cylinder inner_cyl1, outer_cyl1, inner_cyl2, outer_cyl2;
};

// Правильная инициализация tiger'а
visible_tiger init_tiger(
  vec4 point, vec4 axis1, vec4 axis2, vec4 axis3, vec4 axis4,
  float inner_r, float outer_r,
  material material1, material material2
) {
  return visible_tiger(
    visible_cylinder(point, axis1, axis2, inner_r, material1),
    visible_cylinder(point, axis1, axis2, outer_r, material1),
    visible_cylinder(point, axis3, axis4, inner_r, material2),
    visible_cylinder(point, axis3, axis4, outer_r, material2)
  );
}

// Пересечение с гранью tiger'а
intersection tigers_face_intersection(
  visible_cylinder cyl, visible_cylinder outer_cyl, visible_cylinder inner_cyl, ray ray, bool outer
) {
  intersection inter = cylinder_intersection(cyl, ray, outer);
  if (dist_to_axes_plane(inter.dist, ray, outer_cyl) > outer_cyl.r) return NOT_INTERSECT;
  if (dist_to_axes_plane(inter.dist, ray, inner_cyl) < inner_cyl.r) return NOT_INTERSECT;
  return inter;
}

// Пересечение с tiger'ом
intersection tiger_intersection(visible_tiger tiger, ray ray) {
  intersection inter111 = tigers_face_intersection(tiger.inner_cyl1, tiger.outer_cyl2, tiger.inner_cyl2, ray, true );
  intersection inter112 = tigers_face_intersection(tiger.inner_cyl1, tiger.outer_cyl2, tiger.inner_cyl2, ray, false);
  intersection inter121 = tigers_face_intersection(tiger.outer_cyl1, tiger.outer_cyl2, tiger.inner_cyl2, ray, true );
  intersection inter122 = tigers_face_intersection(tiger.outer_cyl1, tiger.outer_cyl2, tiger.inner_cyl2, ray, false);
  intersection inter211 = tigers_face_intersection(tiger.inner_cyl2, tiger.outer_cyl1, tiger.inner_cyl1, ray, true );
  intersection inter212 = tigers_face_intersection(tiger.inner_cyl2, tiger.outer_cyl1, tiger.inner_cyl1, ray, false);
  intersection inter221 = tigers_face_intersection(tiger.outer_cyl2, tiger.outer_cyl1, tiger.inner_cyl1, ray, true );
  intersection inter222 = tigers_face_intersection(tiger.outer_cyl2, tiger.outer_cyl1, tiger.inner_cyl1, ray, false);

  return closest(
    closest(closest(inter111, inter112), closest(inter121, inter122)),
    closest(closest(inter211, inter212), closest(inter221, inter222))
  );
}


// Куб (трёхмерный)
struct visible_cube {
  space space;
  vec4 x, y, z;
  float r;
  material material;
};

intersection cube_intersection(visible_cube cube, ray ray) {
  vec4 vec_n = -cube.space.norm;
  vec4 vec_c = cube.space.point - ray.point;
  float h = dot(vec_c, vec_n);
  if (h < 0) return NOT_INTERSECT;
  float cos_dn = dot(ray.drct, vec_n);
  if (cos_dn < 0) return NOT_INTERSECT;
  float dist = h / cos_dn;
  vec4 point = ray.point + ray.drct * dist;
  vec4 vec_cp = point - cube.space.point;
  if (abs(dot(vec_cp, cube.x)) > cube.r) return NOT_INTERSECT;
  if (abs(dot(vec_cp, cube.y)) > cube.r) return NOT_INTERSECT;
  if (abs(dot(vec_cp, cube.z)) > cube.r) return NOT_INTERSECT;
  return intersection(true, dist, cube.space.norm, cube.material);
}


// Гиперкуб
struct visible_hypercube {
  visible_cube[8] cubes;
};

visible_hypercube init_hypercube(
  vec4 point, vec4 x, vec4 y, vec4 z, vec4 w,
  float r,
  material mxp, material myp, material mzp, material mwp,
  material mxn, material myn, material mzn, material mwn
) {
  visible_cube[8] cubes = visible_cube[8](
    visible_cube(space(point + x * r,  x), y, z, w, r, mxp),
    visible_cube(space(point + y * r,  y), x, z, w, r, myp),
    visible_cube(space(point + z * r,  z), x, y, w, r, mzp),
    visible_cube(space(point + w * r,  w), x, y, z, r, mwp),
    visible_cube(space(point - x * r, -x), y, z, w, r, mxn),
    visible_cube(space(point - y * r, -y), x, z, w, r, myn),
    visible_cube(space(point - z * r, -z), x, y, w, r, mzn),
    visible_cube(space(point - w * r, -w), x, y, z, r, mwn)
  );

  return visible_hypercube(cubes);
}

intersection hypercube_intersection(visible_hypercube hypercube, ray ray) {
  for (int i = 0; i < 8; i++) {
    intersection inter = cube_intersection(hypercube.cubes[i], ray);
    if (inter.did_intersect) return inter;
  }
  return NOT_INTERSECT;
}


// Солнце
struct sun_properties {
  vec4  drct;         // Направление, в котором находится солнце
  float angular_size; // Угловой размер солнца
  vec3  light;        // Испускаемый солнцем свет
  float sharpness;    // Если 1, солнце становится ровным одноцветным кругом. Чем ближе к нулю, тем сильнее оно размывается.
};


// Инициализация объектов сцены

const vec3 sky_light = vec3(0.2, 0.6, 1.2);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), PI * 0.09, vec3(500, 500, 10), 0.0);

const uint spaces_count = 1;
const visible_space[spaces_count] spaces = visible_space[spaces_count](
  visible_space(space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(0.4, 0.25, 0.07)))
);

visible_tiger tiger = init_tiger(
  vec4(0, 2, 0, 0),
  vec4(1, 0, 0, 0), vec4(0, 0, 0, 1), vec4(0, 0, 1, 0), vec4(0, 1, 0, 0),
  0.9, 1.4,
  material(0, 0, vec3(1.0, 0.0, 0.0)), material(0, 0, vec3(0.07, 0.67, 0.25))
);


// Трассировка луча

// Поиск ближайшего пересечения с объектом.
// Перебираются все объекты сцены без каких-то умных оптимизаций.
intersection find_intersection(ray ray) {
  intersection inter = NOT_INTERSECT;
  
  for (int i = 0; i < spaces.length(); i++)
    inter = closest(space_intersection(spaces[i], ray), inter);
  
  //for (int i = 0; i < spheres.length(); i++)
  //  inter = closest(sphere_intersection(spheres[i], ray, true), inter);
  
  //for (int i = 0; i < cylinders.length(); i++)
  //  inter = closest(cylinder_intersection(cylinders[i], ray, true), inter);
  
  //inter = closest(cylinders_union_intersection(cylinders_union, ray), inter);
  //inter = closest(hypercube_intersection(hypercube, ray), inter);
  inter = closest(tiger_intersection(tiger, ray), inter);
  
  return inter;
}

// Обработка луча, улетевшего в пустоту
vec3 final_light(vec4 drct) {
  // При желании можно убрать солнце.
  // return vec3(0);
  
  // Луч попадает в солнце или небо, а они испускают свет
  float deviation = angle(drct, sun.drct);
  if (deviation < sun.angular_size) {
    // Нетривиальный градиент, чтобы солнышко было красивым.
    float k = deviation / sun.angular_size, s = sun.sharpness;
    k = (s * s * k / (1 - s * k) + 1) * (1 - k);
    return sun.light * k + sky_light * (1 - k);
  } else {
    return sky_light;
  }
}

// Трассировка луча. Возвращает свет, прилетающий по лучу.
vec3 trace(ray ray) {
  vec3 result_light = vec3(0);          // Свет, дошедший в сумме от источников
  vec3 unabsorbed_light_part = vec3(1); // Доли света, не поглощённого при переотражениях луча
  for (int i = 0; i <= reflections_amount; i++) {
    intersection inter = find_intersection(ray);

    if (!inter.did_intersect) {
      return result_light + unabsorbed_light_part * final_light(ray.drct);
    }

    result_light += inter.material.color * inter.material.glow * unabsorbed_light_part; // Учёт свечения объекта
    unabsorbed_light_part *= inter.material.color; // Поглощение света

    // Новая точка начала луча: с небольшим отступом, чтобы не попадать внутрь объекта
    ray.point += ray.drct * inter.dist + inter.norm * SMALL_FLOAT;

    // Отражение или случайное направление луча
    if (rand_outcome(inter.material.refl_prob))
      ray.drct = reflect(ray.drct, inter.norm);
    else
      ray.drct = redirect(rand_drct(), inter.norm);
  }
  // При достижении максимально числа отражений нового света не добавляется, получаются тени.
  return result_light;
}


// Главный метод, основная работа c экраном и матрицей

// Изначальное направление полёта луча: от фокуса через точку на матрице
vec4 ray_drct() {
  vec2 mtr_coord = vec2((scr_coord.x - 0.5) * mtr_sizes.x, (0.5 - scr_coord.y) * mtr_sizes.y);
  vec4 ray_drct = vec_to_mtr + top_drct * mtr_coord.y + right_drct * mtr_coord.x;
  return normalize(ray_drct);
}

// Преобразование света в цвет для пикселя
// Свет может быть от нуля до бесконечности, а цвет лишь от нуля до единицы.
vec3 light_to_color(vec3 light) {
  return 1 - 1 / (light_to_color_conversion_coefficient * light + 1);
}

void main() {
  // Координата на экране
  scr_coord = gl_FragCoord.xy;
  scr_coord = vec2(scr_coord.x / resolution.x, scr_coord.y / resolution.y);

  vec3 light = vec3(0);
  vec4 ray_drct = ray_drct();
  for (int i = 0; i < samples; i++)
    light += trace(ray(focus, ray_drct));
  light /= samples;

  // Смешиваем старый цвет с новым по пропорции
  vec3 new_color = light_to_color(light);
  vec3 old_color = texture(old_frame, scr_coord).rgb;
  gl_FragColor = vec4(mix(old_color, new_color, part), 1);
}
