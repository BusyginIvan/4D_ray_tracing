#include "windows/windows.h"
#include "main.h"
#include "util/math.h"
#include <iomanip>

WindowParameters::WindowParameters(const string& windowType, unsigned style) {
  this->windowType = windowType;
  this->style = style;
  title = props.getStringOrNull(fullPropertyName("title"));
  width = props.getUnsignedInt(fullPropertyName("width"));
  height = width / GOLDEN;
  cellSize = props.getUnsignedInt(fullPropertyName("cell_size"));
}

string WindowParameters::fullPropertyName(const string& propertyName) {
  return "window." + windowType + "." + propertyName;
}

void WindowParameters::scale(float multiplier) {
  width *= multiplier; height *= multiplier;
}


CellsWindow::CellsWindow(struct WindowParameters& params) {
  cellsWidth  = params.width  / params.cellSize;
  cellsHeight = params.height / params.cellSize;
  params.width  = cellsWidth  * params.cellSize;
  params.height = cellsHeight * params.cellSize;
  renderWindow.create(VideoMode(params.width, params.height), params.title, params.style);
  renderWindow.setFramerateLimit(maxFPS);
  texture.create(cellsWidth, cellsHeight);
  sprite = Sprite(texture.getTexture());
  sprite.setScale(params.cellSize, params.cellSize);
}

void CellsWindow::setPosition(const unsigned x, const unsigned y) {
  renderWindow.setPosition(Ivec2(x, y));
}

void CellsWindow::drawShaderImage(const Vec4 top, const Vec4 right) {
  shader.setUniform("resolution", Vec2(cellsWidth, cellsHeight));
  shader.setUniform("old_frame", texture.getTexture());
  shader.setUniform("top_drct", top);
  shader.setUniform("right_drct", right);
  texture.draw(sprite, &shader);
  renderWindow.draw(sprite);
}

void CellsWindow::drawFPS(float seconds) {
  stringstream ss; ss << "FPS: " << setw(5) << setprecision(1) << fixed << 1 / seconds;
  text.setString(ss.str());
  renderWindow.draw(text);
}