// Четырёхмерный цилиндр, полученный пересечением двух бесконечных цилиндров.

const vec3 sky_color = vec3(0.004, 0.004, 0.004);
const sun_properties sun = sun_properties(vec4(0, 1, 1, 0), pi / 20, vec3(1, 1, 1));

const uint spaces_count = 1;
const space_obj[spaces_count] spaces = space_obj[spaces_count](
  space_obj(space(vec4(0, 0, -1.5, 0), vec4(0, 0, 1, 0)), material(0, 0, vec3(0.7, 0.7, 0.8)))
);

const cylinders_union my_union = cylinders_union(
  cylinder_obj(
    vec4(0, 0, 0, 0),
    vec4(1, 0, 0, 0), vec4(0, 0, 0, 1),
    1.0,
    material(0, 0, vec3(0.6, 0.4, 0.2))
  ),
  cylinder_obj(
    vec4(0, 0, 0, 0),
    vec4(0, 0, 1, 0), vec4(0, 1, 0, 0),
    1.0,
    material(0, 0, vec3(0.2, 1.0, 0.2))
  )
);