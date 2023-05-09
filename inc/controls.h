#ifndef RAY_TRACING_CONTROLS_H
#define RAY_TRACING_CONTROLS_H

#include <SFML/Graphics.hpp>
using namespace sf;
using namespace sf::Glsl;

struct orientation {
  Vec4 forward, top, right, w_drct;
};

extern struct orientation orientation;
extern Vec4 focus;
extern const float focusToMtrDist, mtrHeight;
extern bool mouseHidden, mouseJustHidden;

void initControls(RenderWindow &mainWindow);
void handleEvent(Event event);
void move(float seconds);

#endif
