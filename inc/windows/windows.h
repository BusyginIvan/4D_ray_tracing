#ifndef RAY_TRACING_WINDOWS_GROUP_H
#define RAY_TRACING_WINDOWS_GROUP_H

#include "properties.h"
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace sf::Glsl;

struct WindowParameters {
  string windowType;      // Тип окна: main, additional
  unsigned style;         // Тип окна: с шапкой или без
  string title;           // Заголовок окна
  unsigned width, height; // Ширина и высота окна в пикселях
  unsigned cellSize;      // Размер одной клетки (большого пикселя) изображения в пикселях

  WindowParameters(const string& windowType, unsigned style);
  string fullPropertyName(const string& propertyName);
  void scale(float multiplier);
};

struct CellsWindow {
  unsigned cellsWidth{}, cellsHeight{}; // Ширина и высота окна в клетках
  RenderWindow renderWindow;
  RenderTexture texture;
  Sprite sprite;

  CellsWindow(WindowParameters& params);
  void setPosition(const unsigned x, const unsigned y);
  void drawShaderImage(const Vec4 top, const Vec4 right);
  void drawFPS(float seconds);
};

struct WindowGroup {
  virtual CellsWindow& getMainWindow() = 0;
  virtual void drawShaderImage() = 0;
  virtual void display() = 0;
};

#endif
