const vec3 sky_color = vec3(0.0035, 0.0035, 0.0035);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), PI / 20, vec3(1, 1, 1));

const tiger_visible tiger = init_tiger(
  vec4(0, 0, 0, 0),
  vec4(1, 0, 0, 0), vec4(0, 0, 0, 1), vec4(0, 0, 1, 0), vec4(0, 1, 0, 0),
  0.9, 1.4,
  material(0, 0, vec3(1.0  , 0.667, 0.0)), material(0, 0, vec3(0.071, 0.251, 0.671))
);