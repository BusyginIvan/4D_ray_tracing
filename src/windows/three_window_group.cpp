#include "windows/three_window_group.h"
#include "main.h"
#include "util/math.h"
#include "controls.h"

using namespace sf::Glsl;

ThreeWindowGroup::ThreeWindowGroup() :
    mainWinParams("main", Style::Close),
    additionalWinParams("additional", Style::None),
    winYXZ(mainWinParams),
    winYWZ(additionalWinParams),
    winYXW(additionalWinParams) {
  scaleWindows(); // Корректировки на случай, если экран слишком маленький
  setWindowPositions();
}

void ThreeWindowGroup::scaleWindows() {
  const float multiplier = min(
    1.0f,
    (float) screenHeight / (mainWinParams.height + additionalWinParams.height),
    (float) screenWidth / 2 / additionalWinParams.width,
    (float) screenWidth / mainWinParams.width
  );
  mainWinParams.scale(multiplier);
  additionalWinParams.scale(multiplier);
}

void ThreeWindowGroup::setWindowPositions() {
  const unsigned indentX = (screenWidth - additionalWinParams.width * 2) / 3;
  const unsigned indentY = (screenHeight - mainWinParams.height - additionalWinParams.height) / 3;
  const unsigned additionalWinY = mainWinParams.height + winTitleHeight + indentY * 2;
  winYXZ.setPosition((screenWidth - mainWinParams.width) / 2, indentY       );
  winYWZ.setPosition(indentX                                , additionalWinY);
  winYXW.setPosition(additionalWinParams.width + indentX * 2, additionalWinY);
}

CellsWindow& ThreeWindowGroup::getMainWindow() {
  return winYXZ;
}

void ThreeWindowGroup::drawShaderImage() {
  winYXZ.drawShaderImage(orientation.top, orientation.right);
  winYWZ.drawShaderImage(orientation.top, orientation.w_drct);
  winYXW.drawShaderImage(orientation.w_drct, orientation.right);
}

void ThreeWindowGroup::display() {
  winYXZ.renderWindow.display();
  winYWZ.renderWindow.display();
  winYXW.renderWindow.display();
}