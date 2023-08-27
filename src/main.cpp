#include "main.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>
#include "controls.h"
#include "math.h"
#include "util.h"

using namespace sf::Glsl;
using namespace std;

Properties properties("properties.txt");
unsigned int frameNumber = 1;

static const unsigned
  MAX_FPS = properties.getUnsignedInt("max_fps"),
  WIN_TITLE_HEIGHT = properties.getUnsignedInt("window_title_height"),
  TASK_BAR_HEIGHT = properties.getUnsignedInt("task_bar_height"),
  SCREEN_WIDTH = VideoMode::getDesktopMode().width,
  SCREEN_HEIGHT = VideoMode::getDesktopMode().height - TASK_BAR_HEIGHT - WIN_TITLE_HEIGHT;

static Shader shader;
static Font font;
static Text text;

struct WindowParameters {
  string windowType;      // Тип окна: main, additional
  unsigned style;         // Тип окна: с шапкой или без
  string title;           // Заголовок окна
  unsigned width, height; // Ширина и высота окна в пикселях
  unsigned cellSize;      // Размер одной клетки (большого пикселя) изображения в пикселях

  WindowParameters(const string& _windowType, unsigned _style) {
    windowType = _windowType;
    style = _style;
    title = properties.getStringOrNull(fullPropertyName("title"));
    width = properties.getUnsignedInt(fullPropertyName("width"));
    height = width / GOLDEN;
    cellSize = properties.getUnsignedInt(fullPropertyName("cell_size"));
  }

  string fullPropertyName(const string& propertyName) {
    return "window." + windowType + "." + propertyName;
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

  CellsWindow(struct WindowParameters &params) {
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
    (float) SCREEN_HEIGHT / (mainWinParams.height + additionalWinParams.height),
    (float) SCREEN_WIDTH / 2 / additionalWinParams.width,
    (float) SCREEN_WIDTH / mainWinParams.width
  );
  mainWinParams.scale(multiplier);
  additionalWinParams.scale(multiplier);
}

static void setWindowPositions(
  CellsWindow& winYXZ,
  CellsWindow& winYWZ,
  CellsWindow& winYXW,
  WindowParameters& mainWinParams,
  WindowParameters& additionalWinParams
) {
  const unsigned indentX = (SCREEN_WIDTH - additionalWinParams.width * 2) / 3;
  const unsigned indentY = (SCREEN_HEIGHT - mainWinParams.height - additionalWinParams.height) / 3;
  const unsigned additionalWinY = mainWinParams.height + WIN_TITLE_HEIGHT + indentY * 2;
  winYXZ.setPosition((SCREEN_WIDTH - mainWinParams.width) / 2, indentY       );
  winYWZ.setPosition(indentX                                , additionalWinY);
  winYXW.setPosition(additionalWinParams.width + indentX * 2, additionalWinY);
}

static void initShader() {
  shader.loadFromFile(properties.getString("shader.filename"), Shader::Fragment);
  shader.setUniform(
    "light_to_color_conversion_coefficient",
    properties.getFloat("shader.light_to_color_conversion_coefficient")
  );
  shader.setUniform("samples", (int) properties.getUnsignedInt("shader.samples"));
  shader.setUniform("reflections_amount", (int) properties.getUnsignedInt("shader.reflections_amount"));
  shader.setUniform("mtr_sizes", Vec2(mtrHeight * GOLDEN, mtrHeight));
}

static void initText() {
  if(!font.loadFromFile(properties.getString("text.font.filename"))) {
    error("Error! Cannot open file \"font.ttl\".\n");
  }
  text.setFont(font);
  text.setFillColor(Color::White); text.setOutlineColor(Color::Black);
  text.setCharacterSize(properties.getUnsignedInt("text.size"));
  text.setOutlineThickness(properties.getFloat("text.outline_thickness"));
  text.setPosition(15, 10);
}

int main() {
  WindowParameters mainWinParams("main", Style::Close);
  WindowParameters additionalWinParams("additional", Style::None);
  scaleWindows(mainWinParams, additionalWinParams); // Корректировки на случай, если экран слишком маленький

  CellsWindow winYXZ(mainWinParams);
  CellsWindow winYWZ(additionalWinParams);
  CellsWindow winYXW(additionalWinParams);
  setWindowPositions(winYXZ, winYWZ, winYXW, mainWinParams, additionalWinParams);

  initControls(winYXZ.renderWindow);
  initShader();
  initText();

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