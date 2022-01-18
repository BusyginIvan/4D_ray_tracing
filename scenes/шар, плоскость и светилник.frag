const vec3 sky_color = vec3(0.001, 0.001, 0.003);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), pi / 25, vec3(1, 1, 0.1));

const space[1] spaces = space[1](
  space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0), properties(0, 0, vec3(0.6, 0.4, 0.2)))
);

const sphere[2] spheres = sphere[2](
  sphere(vec4(0, 0, 0, 0), 1.0 , properties(0, 0.7, vec3(0.2, 1.0, 0.2))),
  sphere(vec4(2, 0, 0, 0), 0.5 , properties(1, 0.0, vec3(1, 1, 1)))
);