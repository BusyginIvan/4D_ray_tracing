#ifndef RAY_TRACING_MAIN_H
#define RAY_TRACING_MAIN_H

#include "properties.h"
#include <SFML/Graphics.hpp>

using namespace sf;

extern Properties props;
extern Shader shader;
extern Font font;
extern Text text;

extern unsigned
  maxFPS,
  winTitleHeight,
  taskBarHeight,
  screenWidth,
  screenHeight;

#endif
