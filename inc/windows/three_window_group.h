#ifndef RAY_TRACING_THREE_WINDOW_GROUP_H
#define RAY_TRACING_THREE_WINDOW_GROUP_H

#include "windows.h"
#include <SFML/Graphics.hpp>

using namespace sf::Glsl;

struct ThreeWindowGroup : WindowGroup {
  WindowParameters mainWinParams, additionalWinParams;
  CellsWindow winYXZ, winYWZ, winYXW;

  ThreeWindowGroup();
  void scaleWindows();
  void setWindowPositions();
  CellsWindow& getMainWindow() override;
  void drawShaderImage() override;
  void display() override;
};

#endif
