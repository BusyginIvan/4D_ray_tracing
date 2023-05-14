#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>
#include "controls.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

unsigned frameNumber = 1; // Номер кадра с тех пор, как камера неподвижна
const unsigned MAX_FPS = 60;
Shader shader;

struct windowParameters {
  string title;           // Заголовок окна
  unsigned width, height; // Ширина и высота окна в пикселях
  unsigned cellSize;      // Размер одной клетки (большого пикселя) окрана в пикселях
  unsigned style;         // Тип окна

  void scale(float multiplier) {
    width *= multiplier; height *= multiplier;
  }
};

struct window {
  unsigned cellsWidth{}, cellsHeight{}; // Ширина и высота окна в клетках
  RenderWindow renderWindow;
  RenderTexture texture;
  Sprite sprite;

  void init(struct windowParameters &params) {
    cellsWidth  = params.width  / params.cellSize;
    cellsHeight = params.height / params.cellSize;
    params.width  = cellsWidth  * params.cellSize;
    params.height = cellsHeight * params.cellSize;
    renderWindow.create(VideoMode(params.width, params.height), params.title, params.style);
    renderWindow.setFramerateLimit(MAX_FPS);
    texture.create(cellsWidth, cellsHeight);
    sprite = Sprite(texture.getTexture());
    sprite.setScale(params.cellSize, params.cellSize);
  }

  void setPosition(const unsigned x, const unsigned y) {
    renderWindow.setPosition(Ivec2(x, y));
  }

  void display() { renderWindow.display(); }

  void redraw(const Vec4 top, const Vec4 right, const bool callDisplay = true) {
    shader.setUniform("resolution", Vec2(cellsWidth, cellsHeight));
    shader.setUniform("old_frame", texture.getTexture());
    shader.setUniform("top_drct", top);
    shader.setUniform("right_drct", right);
    texture.draw(sprite, &shader);
    renderWindow.draw(sprite);
    if (callDisplay) display();
  }
};

int main() {
  // Параметры главного и двух дополнительных окон
  struct windowParameters mainWinParams = {
    .title = "Main section",
    .width = 850,
    .cellSize = 5,
    .style = Style::Close,
  };
  mainWinParams.height = mainWinParams.width / GOLDEN;

  struct windowParameters additionalWinParams  = {
    .width = 600,
    .cellSize = 8,
    .style = Style::None,
  };
  additionalWinParams.height = additionalWinParams.width / GOLDEN;

  // Корректировка размеров окон на случай, если экран слишком маленький
  const unsigned
    winTitleHeight = 37, // Высота шапки главного окна приложения в пикселях
    taskBarHeight = 60,  // Высота панели задач Windows в пикселях
    screenWidth = VideoMode::getDesktopMode().width,
    screenHeight = VideoMode::getDesktopMode().height - taskBarHeight - winTitleHeight;

  const float multiplier = min(
    1.0f,
    (float) screenHeight / (mainWinParams.height + additionalWinParams.height),
    (float) screenWidth / 2 / additionalWinParams.width,
    (float) screenWidth / mainWinParams.width
  );

  mainWinParams.scale(multiplier);
  additionalWinParams.scale(multiplier);

  // Инициализация окон
  struct window winYXZ, winYWZ, winYXW;
  winYXZ.init(mainWinParams);
  winYWZ.init(additionalWinParams);
  winYXW.init(additionalWinParams);
  initControls(winYXZ.renderWindow);

  // Положение окон на экране
  const unsigned indentY = (screenHeight - mainWinParams.height - additionalWinParams.height) / 3;
  const unsigned indentX = (screenWidth - additionalWinParams.width * 2) / 3;

  winYXZ.setPosition((screenWidth - mainWinParams.width) / 2, indentY);
  winYWZ.setPosition(indentX, mainWinParams.height + winTitleHeight + indentY * 2);
  winYXW.setPosition(additionalWinParams.width + indentX * 2, mainWinParams.height + winTitleHeight + indentY * 2);

  // Инициализация шейдера
  shader.loadFromFile("shader.frag", Shader::Fragment);
  shader.setUniform("mtr_sizes", Vec2(mtrHeight * GOLDEN, mtrHeight));

  // Настройки формата отображения текста
  Font font;
  if(!font.loadFromFile("font.ttf")) {
    cout << "Error! Cannot find file \"font.ttl\"." << endl;
    return -1;
  }

  Text text; text.setFont(font);
  text.setFillColor(Color::White); text.setOutlineColor(Color::Black);
  text.setCharacterSize(24); text.setOutlineThickness(2);
  text.setPosition(15, 10);

  // Главный цикл приложения
  Clock timer;
  while (winYXZ.renderWindow.isOpen())
  {
    Event event{};
    while (winYXZ.renderWindow.pollEvent(event))
      handleEvent(event);

    if (mouseHidden) {
      if (mouseJustHidden) {
        timer.restart();
        mouseJustHidden = false;
      }

      shader.setUniform("seed", rand());
      shader.setUniform("part", 1.0f / frameNumber);
      if (frameNumber++ == 1) {
        shader.setUniform("focus", focus);
        shader.setUniform("vec_to_mtr", mulVN(orientation.forward, focusToMtrDist));
      }

      winYWZ.redraw(orientation.top, orientation.w_drct);
      winYXW.redraw(orientation.w_drct, orientation.right);
      winYXZ.redraw(orientation.top, orientation.right, false);

      float seconds = timer.restart().asSeconds();
      move(seconds);

      stringstream ss; ss << "FPS: " << setw(5) << setprecision(1) << fixed << 1 / seconds;
      text.setString(ss.str());
      winYXZ.renderWindow.draw(text);
      winYXZ.display();
    }
  }
  return 0;
}