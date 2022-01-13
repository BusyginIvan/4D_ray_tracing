#ifndef RAY_TRACING_CONTROLS_H
#define RAY_TRACING_CONTROLS_H

#include <SFML/Graphics.hpp>
using namespace sf;

struct view_drct {
  Vector3f forward;
  Vector3f top;
  Vector3f right;
};

extern bool mouseHidden;
extern Vector3f focus;
extern struct view_drct view_drct;

void controls_init();
void handle_event(sf::Event event);
void move();

#endif
