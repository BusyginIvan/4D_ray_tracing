#version 450

const float pi = 3.14159265f;
const float small_val = 0.0003;   // Маленькая величина. Примерно равна 2^(-12).

vec2 scr_coord;                   // scr (screen) – экран. Координата пикселя в окне.

// Геометрические объекты.
struct line { vec4 point, drct; };
struct ray { vec4 point, drct; };
struct space { vec4 point, norm; };
struct sphere { vec4 center; float r; };
struct maybe_point { bool valid; vec4 point; };


// Вспомогательные функции.

// Синус по косинусу или наоборот.
float cos_to_sin(float cos) { return sqrt(1 - cos * cos); }

// Косинус угла между векторами.
float v_cos(vec4 v1, vec4 v2) {
  return dot(v1, v2) / length(v1) / length(v2);
}

// Угол между векторами.
float angle(vec4 v1, vec4 v2) { return acos(v_cos(v1, v2)); }

// Убираем из вектора составляющую, коллинеарную направлению.
vec4 vec_in_space(vec4 vec, vec4 drct) { return vec - drct * dot(vec, drct); }

// Вектор от точки к прямой.
vec4 vec_to_line(vec4 point, line line) {
  vec4 v1 = line.point - point;
  float dot_v1ld = dot(v1, line.drct);
  vec4 v2 = -dot_v1ld * line.drct;
  return v1 + v2;
}

// Вектор от точки к пространству.
vec4 vec_to_space(vec4 point, space space) {
  return space.norm * dot(space.point - point, space.norm);
}

// Отражение вектора, если он смотрит внутрь поверхности.
vec4 redirect(vec4 vec, vec4 norm) {
  float dot = dot(vec, norm);
  return dot >= 0 ? vec : vec - 2 * dot * norm;
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

struct material {
  float glow;         // Доля испускаемого света в противовес отражаемому.
  float refl_prob;    // Вероятность зеркального отражения луча.
  vec3 color;         // Цвет. Остальной свет поглащается.
};

const material null_material = material(0, 0, vec3(0));

struct intersection {
  bool valid;        // Было ли пересечение.
  float dist;        // Расстояние, пройденное лучом до пересечения с объектом.
  vec4 norm;         // Нормаль к поверхности в точке пересечения.
  material material; // Характеристики материала объекта.
};

const intersection no_intersection = intersection(false, 0, vec4(0), null_material);

// Определение ближайшего пересечения.
intersection nearest(intersection int1, intersection int2) {
  if (int1.valid) {
    if (int2.valid) return int1.dist < int2.dist ? int1 : int2;
    else return int1;
  } else return int2.valid ? int2 : no_intersection;
}


// Гиперcфера.
struct sphere_obj {
  sphere figure;
  material material;
};

// Пересечение луча с гиперсферой.
// outer: если false, луч, летящий снаружи, пролетит переднюю стенку насквозь.
// Обозначения: o - центр сферы, p - ray.point, a - точка пересечения.
intersection sphere_intersection(sphere_obj sphere, ray ray, bool outer) {
  vec4 vec_po = sphere.figure.center - ray.point;
  float dot_pord = dot(vec_po, ray.drct);
  float len_po = length(vec_po);
  float r = sphere.figure.r;
  if (len_po >= r && dot_pord < 0) return no_intersection;
  float cos_opa = dot_pord / len_po;
  if (cos_opa > 1) cos_opa = 1; // Бывает, что из-за неточности вычислений получается чуть больше одного.
  if (cos_opa < -1) cos_opa = -1;
  float angle_opa = acos(cos_opa);
  float sin_oap = len_po * sin(angle_opa) / r;
  if (sin_oap >= 1) return no_intersection;
  float angle_oap = asin(sin_oap);
  if (outer && len_po > r) angle_oap = pi - angle_oap;
  float angle_aop = pi - angle_opa - angle_oap;
  float dist = sqrt(r * r + len_po * len_po - 2 * r * len_po * cos(angle_aop));
  vec4 norm = (sphere.figure.center - (ray.point + ray.drct * dist)) / r;
  if (outer && len_po > r) norm *= -1;
  return intersection(true, dist, norm, sphere.material);
}


// Пространство (трёхмерное).
struct space_obj {
  space figure;
  material material;
};

// Пересечение луча с пространством.
intersection space_intersection(space_obj space, ray ray) {
  vec4 vec_cp = space.figure.point - ray.point;
  float dot_vn = dot(vec_cp, space.figure.norm);   // Расстояние до пространства (со знаком).
  vec4 drct_h = space.figure.norm * sign(dot_vn);  // Единичный вектор в сторону пространства.
  float cos_dh = dot(drct_h, ray.drct);            // Косинус угла между этим вектором и лучём.
  if (cos_dh <= 0) return no_intersection;         // Если луч летит от пространства, пересечения нет.
  float dist = abs(dot_vn) / cos_dh;
  return intersection(true, dist, -drct_h, space.material);
}


// Цилиндр, бесконечный по двум направлениям.
struct cylinder_obj {
  vec4 point, axis1, axis2;
  float r;
  material material;
};

// Пересечение с цилиндром.
// outer: если false, луч, летящий снаружи, пролетит переднюю стенку насквозь.
intersection cylinder_intersection(cylinder_obj cylinder, ray ray_in_hyperspace, bool outer) {
  vec4 vec_to_space = vec_to_space(ray_in_hyperspace.point, space(cylinder.point, cylinder.axis1));
  ray ray_in_space = ray(
    ray_in_hyperspace.point + vec_to_space,
    vec_in_space(ray_in_hyperspace.drct, cylinder.axis1)
  );
  if (length(ray_in_space.drct) == 0) return no_intersection;
  
  vec4 vec_to_plane = vec_to_space(ray_in_space.point, space(cylinder.point, cylinder.axis2));
  ray ray_in_plane = ray(
    ray_in_space.point + vec_to_plane,
    vec_in_space(ray_in_space.drct, cylinder.axis2)
  );
  float length_drct_in_plane = length(ray_in_plane.drct);
  if (length_drct_in_plane == 0) return no_intersection;
  ray_in_plane.drct = ray_in_plane.drct / length_drct_in_plane;
  
  intersection inter = sphere_intersection(
    sphere_obj(sphere(cylinder.point, cylinder.r), cylinder.material),
    ray_in_plane, outer
  );
  inter.dist /= length_drct_in_plane;
  return inter;
}

// Расстояние до плоскости осей цилиндра.
float dist_from_axis(float dist, ray ray, cylinder_obj cylinder) {
  vec4 point_in_hyperspace = ray.point + ray.drct * dist;
  vec4 point_in_space = point_in_hyperspace + vec_to_space(point_in_hyperspace, space(cylinder.point, cylinder.axis1));
  vec4 point_in_plane = point_in_space + vec_to_space(point_in_space, space(cylinder.point, cylinder.axis2));
  return length(cylinder.point - point_in_plane);
}


// Объединение двух цилиндров.
struct cylinders_union {
  cylinder_obj cylinder1, cylinder2;
};

// Пересечение с объединением.
intersection cylinders_union_intersection(cylinders_union cylinders_union, ray ray) {
  intersection inter1 = cylinder_intersection(cylinders_union.cylinder1, ray, true);
  if (dist_from_axis(inter1.dist, ray, cylinders_union.cylinder2) > cylinders_union.cylinder2.r)
    inter1 = no_intersection;
  
  intersection inter2 = cylinder_intersection(cylinders_union.cylinder2, ray, true);
  if (dist_from_axis(inter2.dist, ray, cylinders_union.cylinder1) > cylinders_union.cylinder2.r)
    inter2 = no_intersection;
  
  return nearest(inter1, inter2);
}


// Tiger. Так обычно называют фигуру в четырёхмерном пространстве, очень похожую на эту.
struct tiger {
  cylinder_obj inner_cyl1, outer_cyl1, inner_cyl2, outer_cyl2;
};

// Правильная инициализация tiger.
tiger init_tiger(
  vec4 point, vec4 axis1, vec4 axis2, vec4 axis3, vec4 axis4,
  float inner_r, float outer_r,
  material material1, material material2
) {
  return tiger(
    cylinder_obj(point, axis1, axis2, inner_r, material1),
    cylinder_obj(point, axis1, axis2, outer_r, material1),
    cylinder_obj(point, axis3, axis4, inner_r, material2),
    cylinder_obj(point, axis3, axis4, outer_r, material2)
  );
}

// Пересечение с гранью tiger'а.
intersection tigers_face_inter(
  cylinder_obj cyl, cylinder_obj outer_cyl, cylinder_obj inner_cyl, ray ray, bool outer
) {
  intersection inter = cylinder_intersection(cyl, ray, outer);
  if (dist_from_axis(inter.dist, ray, outer_cyl) > outer_cyl.r) inter = no_intersection;
  if (dist_from_axis(inter.dist, ray, inner_cyl) < inner_cyl.r) inter = no_intersection;
  return inter;
}

// Пересечение с tiger.
intersection tiger_intersection(tiger tiger, ray ray) {
  intersection inter111 = tigers_face_inter(tiger.inner_cyl1, tiger.outer_cyl2, tiger.inner_cyl2, ray, true );
  intersection inter112 = tigers_face_inter(tiger.inner_cyl1, tiger.outer_cyl2, tiger.inner_cyl2, ray, false);
  intersection inter121 = tigers_face_inter(tiger.outer_cyl1, tiger.outer_cyl2, tiger.inner_cyl2, ray, true );
  intersection inter122 = tigers_face_inter(tiger.outer_cyl1, tiger.outer_cyl2, tiger.inner_cyl2, ray, false);
  intersection inter211 = tigers_face_inter(tiger.inner_cyl2, tiger.outer_cyl1, tiger.inner_cyl1, ray, true );
  intersection inter212 = tigers_face_inter(tiger.inner_cyl2, tiger.outer_cyl1, tiger.inner_cyl1, ray, false);
  intersection inter221 = tigers_face_inter(tiger.outer_cyl2, tiger.outer_cyl1, tiger.inner_cyl1, ray, true );
  intersection inter222 = tigers_face_inter(tiger.outer_cyl2, tiger.outer_cyl1, tiger.inner_cyl1, ray, false);
  
  return nearest(
    nearest(nearest(inter111, inter112), nearest(inter121, inter122)),
    nearest(nearest(inter211, inter212), nearest(inter221, inter222))
  );
}


// Солнце.
struct sun_properties {
  vec4 drct;              // Направление, в котором находится солнце.
  float angular_size;     // Угловой размер солнца (максимальный угол отклонения полёта луча от положения солнца при попадании).
  vec3 color;             // Цвет солнца; свет, испускаемый им.
};


// Инициализация объектов сцены.

const vec3 sky_color = vec3(0.0035, 0.0035, 0.0035);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), pi / 20, vec3(1, 1, 1));

const tiger my_tiger = init_tiger(
  vec4(0, 0, 0, 0),
  vec4(1, 0, 0, 0), vec4(0, 0, 0, 1), vec4(0, 0, 1, 0), vec4(0, 1, 0, 0),
  0.9, 1.4,
  material(0, 0, vec3(1.0  , 0.667, 0.0)), material(0, 0, vec3(0.071, 0.251, 0.671))
);


// Трассировка луча.

const uint reflections_number = 3; // Максимальное количество переотражений (или точнее число перелётов).

// Трассировка луча. Возвращает цвет.
vec3 trace(ray ray) {
  vec3 res_color = vec3(0);  // res (result) – свет, полученный в сумме от источников или неба.
  vec3 rem_color = vec3(1);  // rem (remaining) – ещё не поглощённый свет, оставшийся в луче.
  for (int i = 0; i < reflections_number; i++) {
    // Поиск ближайшего пересечения с объектом.
    intersection inter = no_intersection;
    /*for (int i = 0; i < spheres.length(); i++)
      inter = nearest(inter, sphere_intersection(spheres[i], ray, true));*/
    /*for (int i = 0; i < spaces.length(); i++)
      inter = nearest(inter, space_intersection(spaces[i], ray));*/
    /*for (int i = 0; i < cylinders.length(); i++)
      inter = nearest(inter, cylinder_intersection(cylinders[i], ray));*/
    //inter = nearest(inter, cylinders_union_intersection(my_union, ray));
    inter = nearest(inter, tiger_intersection(my_tiger, ray));
    
    // Если нет пересечения, попадаем в солнце или небо.
    if (!inter.valid) {
      if (angle(ray.drct, sun.drct) < sun.angular_size)
        res_color += rem_color * sun.color;
      else
        res_color += rem_color * sky_color;
      return res_color;
    }
    
    rem_color = rem_color * inter.material.color;    // Поглощение света.
    res_color += rem_color * inter.material.glow;    // Учёт свечения объекта.
    if (inter.material.glow == 1) return res_color;
    rem_color *= 1 - inter.material.glow;            // Вычисляем долю отражённого света.

    // Новая точка начала луча. С небольшим отступом, чтобы не попадать внутрь объекта.
    ray.point += ray.drct * inter.dist + inter.norm * small_val;

    // Отражение или случайное направление луча.
    if (rand() > inter.material.refl_prob)
      ray.drct = redirect(rand_drct(), inter.norm);
    else
      ray.drct = reflect(ray.drct, inter.norm);
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
const float c = 80; // С увеличением этой константы усиливается эффект.
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
  int samples = 100;           // Число запускаемых лучей. С одним лучом фильтр цвета не имел бы смысла.
  vec4 ray_drct = ray_drct();
  for(int i = 0; i < samples; i++)
    new_color += trace(ray(focus, ray_drct));
  new_color /= samples;

  new_color = tone_mapping(new_color); // Применение фильтра.

  vec3 old_color = texture(old_frame, scr_coord).rgb;       // Старый цвет пикселя.
  gl_FragColor = vec4(mix(old_color, new_color, part), 1);  // Смешиваем старый цвет с новым по пропорции.
}