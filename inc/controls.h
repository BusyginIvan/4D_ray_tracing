#ifndef RAY_TRACING_CONTROLS_H
#define RAY_TRACING_CONTROLS_H

#include <SFML/Graphics.hpp>
using namespace sf;
using namespace sf::Glsl;

struct orientation {
  Vec4 forward, top, right, w_drct;
};

extern bool mouse_hidden;
extern Vec4 focus;
extern struct orientation orientation;

void controls_init(RenderWindow &main_window);
void handle_event(Event event);
void move();

#endif
