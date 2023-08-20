#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>
#include "controls.h"
#include "math.h"
#include "util.h"

using namespace sf::Glsl;
using namespace std;

unsigned frameNumber = 1; // Номер кадра с тех пор, как камера неподвижна

static const unsigned
  MAX_FPS = 60,
  winTitleHeight = 37, // Высота шапки главного окна приложения в пикселях
  taskBarHeight = 60,  // Высота панели задач Windows в пикселях
  screenWidth = VideoMode::getDesktopMode().width,
  screenHeight = VideoMode::getDesktopMode().height - taskBarHeight - winTitleHeight;

static Shader shader;
static Font font;
static Text text;

struct WindowParameters {
  string title;           // Заголовок окна
  unsigned width, height; // Ширина и высота окна в пикселях
  unsigned cellSize;      // Размер одной клетки (большого пикселя) окрана в пикселях
  unsigned style;         // Тип окна

  void setGoldenHeight() {
    height = width / GOLDEN;
  }

  void scale(float multiplier) {
    width *= multiplier; height *= multiplier;
  }
};

struct CellsWindow {
  unsigned cellsWidth{}, cellsHeight{}; // Ширина и высота окна в клетках
  RenderWindow renderWindow;
  RenderTexture texture;
  Sprite sprite;

  void init(struct WindowParameters &params) {
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

  void drawShaderImage(const Vec4 top, const Vec4 right) {
    shader.setUniform("resolution", Vec2(cellsWidth, cellsHeight));
    shader.setUniform("old_frame", texture.getTexture());
    shader.setUniform("top_drct", top);
    shader.setUniform("right_drct", right);
    texture.draw(sprite, &shader);
    renderWindow.draw(sprite);
  }

  void drawFPS(float seconds) {
    stringstream ss; ss << "FPS: " << setw(5) << setprecision(1) << fixed << 1 / seconds;
    text.setString(ss.str());
    renderWindow.draw(text);
  }
};

static void scaleWindows(WindowParameters& mainWinParams, WindowParameters& additionalWinParams) {
  const float multiplier = min(
    1.0f,
    (float) screenHeight / (mainWinParams.height + additionalWinParams.height),
    (float) screenWidth / 2 / additionalWinParams.width,
    (float) screenWidth / mainWinParams.width
  );

  mainWinParams.scale(multiplier);
  additionalWinParams.scale(multiplier);
}

static void initWindows(
  CellsWindow& winYXZ,
  CellsWindow& winYWZ,
  CellsWindow& winYXW,
  WindowParameters& mainWinParams,
  WindowParameters& additionalWinParams
) {
  winYXZ.init(mainWinParams);
  winYWZ.init(additionalWinParams);
  winYXW.init(additionalWinParams);

  const unsigned indentX = (screenWidth - additionalWinParams.width * 2) / 3;
  const unsigned indentY = (screenHeight - mainWinParams.height - additionalWinParams.height) / 3;
  const unsigned additionalWinY = mainWinParams.height + winTitleHeight + indentY * 2;
  winYXZ.setPosition((screenWidth - mainWinParams.width) / 2, indentY       );
  winYWZ.setPosition(indentX                                , additionalWinY);
  winYXW.setPosition(additionalWinParams.width + indentX * 2, additionalWinY);
}

static void initShader(String shaderFileName) {
  shader.loadFromFile(shaderFileName, Shader::Fragment);
  shader.setUniform("mtr_sizes", Vec2(mtrHeight * GOLDEN, mtrHeight));
}

static void initText(String fontFileName) {
  if(!font.loadFromFile(fontFileName)) {
    error("Error! Cannot open file \"font.ttl\".\n");
  }
  text.setFont(font);
  text.setFillColor(Color::White); text.setOutlineColor(Color::Black);
  text.setCharacterSize(24); text.setOutlineThickness(2);
  text.setPosition(15, 10);
}

int main() {
  WindowParameters mainWinParams = {
    .title = "Main section",
    .width = 850,
    .cellSize = 5,
    .style = Style::Close,
  };
  mainWinParams.setGoldenHeight();

  WindowParameters additionalWinParams  = {
    .width = 600,
    .cellSize = 8,
    .style = Style::None,
  };
  additionalWinParams.setGoldenHeight();

  scaleWindows(mainWinParams, additionalWinParams); // Корректировки на случай, если экран слишком маленький

  CellsWindow winYXZ, winYWZ, winYXW;
  initWindows(winYXZ, winYWZ, winYXW, mainWinParams, additionalWinParams);
  initControls(winYXZ.renderWindow);

  initShader("shader.frag");
  initText("font.ttf");

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

      winYWZ.drawShaderImage(orientation.top, orientation.w_drct);
      winYXW.drawShaderImage(orientation.w_drct, orientation.right);
      winYXZ.drawShaderImage(orientation.top, orientation.right);

      float seconds = timer.restart().asSeconds();
      move(seconds);
      winYXZ.drawFPS(seconds);

      winYWZ.display(); winYXW.display(); winYXZ.display();
    }
  }
  return 0;
}