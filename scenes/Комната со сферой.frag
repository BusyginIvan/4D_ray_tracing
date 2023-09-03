// Инициализация объектов сцены

const uint spaces_count = 8;
const visible_space[spaces_count] spaces = visible_space[spaces_count](
  visible_space(space(vec4( 3, 0, 0, 0), vec4(1, 0, 0, 0)), material(0, 0, vec3(0.44, 0.04, 0.67))),
  visible_space(space(vec4(-3, 0, 0, 0), vec4(1, 0, 0, 0)), material(0, 0, vec3(1.0 , 1.0 , 0.0 ))),
  visible_space(space(vec4( 0, 3, 0, 0), vec4(0, 1, 0, 0)), material(0, 0, vec3(1.0 , 0.0 , 0.0 ))),
  visible_space(space(vec4( 0,-3, 0, 0), vec4(0, 1, 0, 0)), material(0, 0, vec3(0.0 , 0.8 , 0.0 ))),
  visible_space(space(vec4( 0, 0, 3, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(1.0 , 1.0 , 1.0 ))),
  visible_space(space(vec4( 0, 0,-3, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(1.0 , 1.0 , 1.0 ))),
  visible_space(space(vec4( 0, 0, 0, 3), vec4(0, 0, 0, 1)), material(0, 0, vec3(1.0 , 0.67, 0.0 ))),
  visible_space(space(vec4( 0, 0, 0,-3), vec4(0, 0, 0, 1)), material(0, 0, vec3(0.07, 0.25, 0.67)))
);

const uint spheres_count = 2;
const visible_sphere[spheres_count] spheres = visible_sphere[spheres_count](
  visible_sphere(sphere(vec4(0, 0, -1, 0), 1.0), material( 0, 0, vec3(1, 1, 1))),
  visible_sphere(sphere(vec4(0, 0,  3, 0), 0.8), material(90, 0, vec3(1, 1, 1)))
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

vec3 final_light(vec4 drct) {
  return vec3(0);
}