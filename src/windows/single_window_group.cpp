#include "windows/single_window_group.h"
#include "main.h"
#include "util/math.h"
#include "controls.h"

using namespace sf::Glsl;

SingleWindowGroup::SingleWindowGroup() :
    windowParams("main", Style::Close),
    window(windowParams) {
  scaleWindow(); // Корректировки на случай, если экран слишком маленький
  setWindowPosition();
}

void SingleWindowGroup::scaleWindow() {
  const float multiplier = min(
    1.0f,
    (float) screenHeight / windowParams.height,
    (float) screenWidth / windowParams.width
  );
  windowParams.scale(multiplier);
}

void SingleWindowGroup::setWindowPosition() {
  window.setPosition((screenWidth - windowParams.width) / 2, (screenHeight - windowParams.height) / 2);
}

CellsWindow& SingleWindowGroup::getMainWindow() {
  return window;
}

void SingleWindowGroup::drawShaderImage() {
  window.drawShaderImage(orientation.top, orientation.right);
}

void SingleWindowGroup::display() {
  window.renderWindow.display();
}