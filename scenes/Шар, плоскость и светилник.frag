const vec3 sky_color = vec3(0.001, 0.001, 0.003);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), PI / 25, vec3(1, 1, 0.1));

const uint spaces_count = 1;
const space_visible[spaces_count] spaces = space_visible[spaces_count](
  space_visible(space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(0.6, 0.4, 0.2)))
);

const uint spheres_count = 2;
const sphere_visible[spheres_count] spheres = sphere_visible[spheres_count](
  sphere_visible(sphere(vec4(0, 0, 0, 0), 1.0), material(0, 0.7, vec3(0.2, 1.0, 0.2))),
  sphere_visible(sphere(vec4(2, 0, 0, 0), 0.5), material(1, 0.0, vec3(1, 1, 1)))
);