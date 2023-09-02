#ifndef RAY_TRACING_SINGLE_WINDOW_GROUP_H
#define RAY_TRACING_SINGLE_WINDOW_GROUP_H

#include "windows.h"
#include <SFML/Graphics.hpp>

using namespace sf::Glsl;

struct SingleWindowGroup : WindowGroup {
  WindowParameters windowParams;
  CellsWindow window;

  SingleWindowGroup();
  void scaleWindow();
  void setWindowPosition();
  CellsWindow& getMainWindow() override;
  void drawShaderImage() override;
  void display() override;
};

#endif
