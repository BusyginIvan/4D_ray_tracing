// Сцена тяжёлая. Рекомендуется увеличить размер пикселей (клеточек) и уменьшить тем самым разрешение.

const vec3 sky_color = vec3(0.001, 0.001, 0.002);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), pi / 25, vec3(1, 1, 0.1));

const space[8] spaces = space[8](
  space(vec4( 3, 0, 0, 0), vec4(1, 0, 0, 0), properties(0, 0, vec3(0.443, 0.035, 0.667))),
  space(vec4(-3, 0, 0, 0), vec4(1, 0, 0, 0), properties(0, 0, vec3(1.0  , 1.0  , 0.0  ))),
  space(vec4(0,  3, 0, 0), vec4(0, 1, 0, 0), properties(0, 0, vec3(1.0  , 0.0  , 0.0  ))),
  space(vec4(0, -3, 0, 0), vec4(0, 1, 0, 0), properties(0, 0, vec3(0.0  , 0.8  , 0.0  ))),
  space(vec4(0, 0,  3, 0), vec4(0, 0, 1, 0), properties(0, 0, vec3(1.0  , 1.0  , 1.0  ))),
  space(vec4(0, 0, -3, 0), vec4(0, 0, 1, 0), properties(0, 0, vec3(1.0  , 1.0  , 1.0  ))),
  space(vec4(0, 0, 0,  3), vec4(0, 0, 0, 1), properties(0, 0, vec3(1.0  , 0.667, 0.0  ))),
  space(vec4(0, 0, 0, -3), vec4(0, 0, 0, 1), properties(0, 0, vec3(0.071, 0.251, 0.671)))
);

const sphere[2] spheres = sphere[2](
  sphere(vec4(0, 0, -1, 0), 1.0, properties(0, 0, vec3(1, 1, 1))),
  sphere(vec4(0, 0,  3, 0), 0.8, properties(1, 0, vec3(1, 1, 1)))
);