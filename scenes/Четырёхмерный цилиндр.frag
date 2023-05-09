// Инициализация объектов сцены

const vec3 sky_light = vec3(0.2, 0.6, 1.2);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), PI * 0.09, vec3(500, 500, 10));

const uint spaces_count = 1;
const visible_space[spaces_count] spaces = visible_space[spaces_count](
  visible_space(space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(0.4, 0.25, 0.07)))
);

const visible_cylinders_union cylinders_union = visible_cylinders_union(
  visible_cylinder(
    vec4(-1, 2, 0, 0),
    vec4(1, 0, 0, 0), vec4(0, 0, 0, 1),
    1.0,
    material(0, 0, vec3(1.0, 0.0, 0.0))
  ),
  visible_cylinder(
    vec4(-1, 2, 0, 0),
    vec4(0, 0, 1, 0), vec4(0, 1, 0, 0),
    1.0,
    material(0, 0, vec3(0.07, 0.67, 0.25))
  )
);


// Трассировка луча

intersection find_intersection(ray ray) {
  intersection inter = NOT_INTERSECT;
  
  for (int i = 0; i < spaces.length(); i++)
    inter = closest(inter, space_intersection(spaces[i], ray));
  
  inter = closest(inter, cylinders_union_intersection(cylinders_union, ray));
  
  return inter;
}

vec3 final_light(vec4 drct) {
  float deviation = angle(drct, sun.drct);
  if (deviation < sun.angular_size) {
    float k = deviation / sun.angular_size, m = 0;
    k = (m * m * k / (1 - m * k) + 1) * (1 - k);
    return sun.light * k + sky_light * (1 - k);
  } else {
    return sky_light;
  }
}