// Сцена тяжёлая. Рекомендуется увеличить размер пикселей (клеточек) и уменьшить тем самым разрешение.

const vec3 sky_color = vec3(0.001, 0.001, 0.002);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), pi / 25, vec3(1, 1, 0.1));

const uint spaces_count = 8;
const space_obj[spaces_count] spaces = space_obj[spaces_count](
  space_obj(space(vec4( 3, 0, 0, 0), vec4(1, 0, 0, 0)), material(0, 0, vec3(0.443, 0.035, 0.667))),
  space_obj(space(vec4(-3, 0, 0, 0), vec4(1, 0, 0, 0)), material(0, 0, vec3(1.0  , 1.0  , 0.0  ))),
  space_obj(space(vec4(0,  3, 0, 0), vec4(0, 1, 0, 0)), material(0, 0, vec3(1.0  , 0.0  , 0.0  ))),
  space_obj(space(vec4(0, -3, 0, 0), vec4(0, 1, 0, 0)), material(0, 0, vec3(0.0  , 0.8  , 0.0  ))),
  space_obj(space(vec4(0, 0,  3, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(1.0  , 1.0  , 1.0  ))),
  space_obj(space(vec4(0, 0, -3, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(1.0  , 1.0  , 1.0  ))),
  space_obj(space(vec4(0, 0, 0,  3), vec4(0, 0, 0, 1)), material(0, 0, vec3(1.0  , 0.667, 0.0  ))),
  space_obj(space(vec4(0, 0, 0, -3), vec4(0, 0, 0, 1)), material(0, 0, vec3(0.071, 0.251, 0.671)))
);

const uint spheres_count = 2;
const sphere_obj[spheres_count] spheres = sphere_obj[spheres_count](
  sphere_obj(sphere(vec4(0, 0, -1, 0), 1.0), material(0, 0, vec3(1, 1, 1))),
  sphere_obj(sphere(vec4(0, 0,  3, 0), 0.8), material(1, 0, vec3(1, 1, 1)))
);