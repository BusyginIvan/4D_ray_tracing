#include <windef.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "controls.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

unsigned int framesStill = 1;  // Номер кадра с тех пор, как камера неподвижна.

struct window {
  unsigned int cellsWidth, cellsHeight;
  RenderWindow renderWindow;
  RenderTexture texture;
  Sprite sprite;
};

struct windowSize {
  unsigned int width, height, cellSize;
};

static void initWindow(
  struct window* const window, struct windowSize size,
  const string title, const int style
) {
  window->cellsWidth  = size.width / size.cellSize;
  window->cellsHeight = size.height / size.cellSize;
  size.width  = window->cellsWidth * size.cellSize;
  size.height = window->cellsHeight * size.cellSize;
  window->renderWindow.create(VideoMode(size.width, size.height), title, style);
  window->renderWindow.setFramerateLimit(60);
  window->texture.create(window->cellsWidth, window->cellsHeight);
  window->sprite = Sprite(window->texture.getTexture());
  window->sprite.setScale(size.cellSize, size.cellSize);
}

static void drawWindow(struct window* const window, Shader &shader, const Vec4 top, const Vec4 right) {
  shader.setUniform("resolution", Vec2(window->cellsWidth, window->cellsHeight));
  shader.setUniform("old_frame", window->texture.getTexture());
  shader.setUniform("top_drct", top);
  shader.setUniform("right_drct", right);
  window->texture.draw(window->sprite, &shader);
  window->renderWindow.draw(window->sprite);
  window->renderWindow.display();
}

int main() {
  const float distToMtr = 1.5f; // Расстояние от фокуса до матрицы. Влияет на угол обзора.
  const float mtrHeight = 1.0f;  // Половина высоты матрицы (виртуального экрана в пространстве).
  struct windowSize mainWindow = { .width = 850, .cellSize = 4 };
  mainWindow.height = mainWindow.width / GOLDEN;
  struct windowSize additionalWindow  = { .width = 700, .cellSize = 4 };
  additionalWindow.height  = additionalWindow.width / GOLDEN;

  // Корректировка размеров окон на случай, если экран слишком маленький.
  const unsigned int windowTitleHeight = 37, taskBarHeight = 60,
    screenWidth = VideoMode::getDesktopMode().width,
    screenHeight = VideoMode::getDesktopMode().height - taskBarHeight - windowTitleHeight;

  const float decreaseFactor = min(min(
    1.0f,
    (float) screenHeight / (mainWindow.height + additionalWindow.height)
  ), min(
    (float) screenWidth / 2 / additionalWindow.width,
    (float) screenWidth / mainWindow.width
  ));

  mainWindow.width *= decreaseFactor; mainWindow.height *= decreaseFactor;
  additionalWindow.width  *= decreaseFactor; additionalWindow.height  *= decreaseFactor;

  // Инициализация окон.
  struct window windowYXZ;
  initWindow(&windowYXZ, mainWindow, "Main section", Style::Close);
  controlsInit(windowYXZ.renderWindow); // Управление будет привязано к этому окну.

  struct window windowYWZ;
  initWindow(&windowYWZ, additionalWindow, "", Style::None);

  struct window windowYXW;
  initWindow(&windowYXW, additionalWindow, "", Style::None);

  // Положение окон на экране.
  const unsigned int indentY = (screenHeight - mainWindow.height - additionalWindow.height) / 3;
  const unsigned int indentX = (screenWidth - additionalWindow.width * 2) / 3;

  windowYXZ.renderWindow.setPosition(Ivec2(
    (screenWidth - mainWindow.width) / 2, indentY
  ));
  windowYWZ.renderWindow.setPosition(Ivec2(
    indentX, mainWindow.height + windowTitleHeight + indentY * 2
  ));
  windowYXW.renderWindow.setPosition(Ivec2(
    additionalWindow.width + indentX * 2, mainWindow.height + windowTitleHeight + indentY * 2
  ));

  // Инициализация шейдера.
  Shader shader; shader.loadFromFile("shader.frag", Shader::Fragment);
  shader.setUniform("mtr_sizes", Vec2(mtrHeight * GOLDEN, mtrHeight));

  // Таймер.
  Clock clock;

  // Главный цикл приложения.
  while (windowYXZ.renderWindow.isOpen())
  {
    Event event;
    while (windowYXZ.renderWindow.pollEvent(event))
      handleEvent(event);

    if (mouseHidden) {
      move();

      shader.setUniform("seed", rand());
      shader.setUniform("part", 1.0f / framesStill);
      if (framesStill == 1) {
        shader.setUniform("focus", focus);
        shader.setUniform("vec_to_mtr", mulVN(orientation.forward, distToMtr));
      }

      drawWindow(&windowYXZ, shader, orientation.top, orientation.right);
      drawWindow(&windowYWZ, shader, orientation.top, orientation.w_drct);
      drawWindow(&windowYXW, shader, orientation.w_drct, orientation.right);

      framesStill++;
      cout << "FPS: " << 1 / clock.restart().asSeconds() << endl;
    }
  }
  return 0;
}