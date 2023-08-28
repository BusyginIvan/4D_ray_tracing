// Инициализация объектов сцены

const vec3 sky_light = vec3(0.4, 0.6, 1.53);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), PI * 0.09, vec3(2100, 1000, 20), 0.0);

const uint spaces_count = 1;
const visible_space[spaces_count] spaces = visible_space[spaces_count](
  visible_space(space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(1, 1, 1)))
);

visible_hypercube hypercube = init_hypercube(
  vec4(0, 4, 0, 0),
  vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1),
  1,
  material(0, 0, vec3(0.72, 0.07, 0.20)),
  material(0, 0, vec3(0.00, 0.61, 0.28)),
  material(0, 0, vec3(1.00, 0.84, 0.00)),
  material(0, 0, vec3(0.40, 0.00, 0.80)),
  material(0, 0, vec3(1.00, 0.35, 0.00)),
  material(0, 0, vec3(0.00, 0.27, 0.68)),
  material(0, 0, vec3(1.00, 1.00, 1.00)),
  material(0, 0, vec3(0.01, 0.01, 0.01))
);


// Трассировка луча

intersection find_intersection(ray ray) {
  intersection inter = NOT_INTERSECT;
  
  for (int i = 0; i < spaces.length(); i++)
    inter = closest(inter, space_intersection(spaces[i], ray));
  
  inter = closest(inter, hypercube_intersection(hypercube, ray));
  
  return inter;
}
