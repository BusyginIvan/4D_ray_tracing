const vec3 sky_color = vec3(0.004, 0.004, 0.004);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), PI / 20, vec3(1, 1, 1));

const uint spaces_count = 1;
const space_visible[spaces_count] spaces = space_visible[spaces_count](
  space_visible(space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(0.7, 0.7, 0.8)))
);

const cylinders_union_visible cylinders_union = cylinders_union_visible(
  cylinder_visible(
    vec4(0, 0, 0, 0),
    vec4(1, 0, 0, 0), vec4(0, 0, 0, 1),
    1.0,
    material(0, 0, vec3(0.6, 0.4, 0.2))
  ),
  cylinder_visible(
    vec4(0, 0, 0, 0),
    vec4(0, 0, 1, 0), vec4(0, 1, 0, 0),
    1.0,
    material(0, 0, vec3(0.2, 1.0, 0.2))
  )
);