#ifndef RAY_TRACING_CONTROLS_H
#define RAY_TRACING_CONTROLS_H

#include <SFML/Graphics.hpp>
using namespace sf;
using namespace sf::Glsl;

struct orientation {
  Vec4 forward, top, right, w_drct;
};

extern bool mouseHidden;
extern Vec4 focus;
extern struct orientation orientation;

void controlsInit(RenderWindow &mainWindow);
void handleEvent(Event event);
void move();

#endif
