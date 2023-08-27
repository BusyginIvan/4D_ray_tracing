#ifndef RAY_TRACING_CONTROLS_H
#define RAY_TRACING_CONTROLS_H

#include <SFML/Graphics.hpp>
using namespace sf;
using namespace sf::Glsl;

struct Orientation {
  Vec4 forward, top, right, w_drct;
};

extern Orientation orientation;
extern Vec4 focus;
extern float focusToMtrDist, mtrHeight;
extern bool mouseHidden, mouseJustHidden;

void initControls(RenderWindow &mainWindow);
void handleEvent(Event event);
void move(float seconds);

#endif
