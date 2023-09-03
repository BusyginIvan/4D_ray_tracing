// Инициализация объектов сцены

const vec3 sky_light = vec3(0.02, 0.06, 0.12);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), PI * 0.09, vec3(10, 10, 0.95), 0.8);

const uint spaces_count = 1;
const visible_space[spaces_count] spaces = visible_space[spaces_count](
  visible_space(space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(0.6, 0.4, 0.2)))
);

const uint spheres_count = 2;
const visible_sphere[spheres_count] spheres = visible_sphere[spheres_count](
  visible_sphere(sphere(vec4(0, 1, 0, 0), 1.0), material( 0, 0.7, vec3(0.2, 1.0, 0.2))),
  visible_sphere(sphere(vec4(2, 1, 0, 0), 0.5), material(90, 0.0, vec3(  1,   1,   1)))
);


// Трассировка луча

intersection find_intersection(ray ray) {
  intersection inter = NOT_INTERSECT;
  
  for (int i = 0; i < spaces.length(); i++)
    inter = closest(space_intersection(spaces[i], ray), inter);
  
  for (int i = 0; i < spheres.length(); i++)
    inter = closest(sphere_intersection(spheres[i], ray, true), inter);
  
  return inter;
}
