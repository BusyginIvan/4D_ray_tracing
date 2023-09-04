#include "controls.h"
#include "util/math.h"
#include "util/util.h"
#include "windows/windows.h"
#include "windows/three_window_group.h"
#include "windows/single_window_group.h"
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace sf::Glsl;
using namespace std;

Properties properties("properties.txt");
Shader shader;
Font font;
Text text;

unsigned
  maxFPS = properties.getUnsignedInt("max_fps"),
  winTitleHeight = properties.getUnsignedInt("window_title_height"),
  taskBarHeight = properties.getUnsignedInt("task_bar_height"),
  screenWidth = VideoMode::getDesktopMode().width,
  screenHeight = VideoMode::getDesktopMode().height - taskBarHeight - winTitleHeight;

static void initShader() {
  shader.loadFromFile(properties.getString("shader_filename"), Shader::Fragment);

  shader.setUniform("samples", (int) properties.getUnsignedInt("ray_tracing.samples"));
  shader.setUniform("reflections_amount", (int) properties.getUnsignedInt("ray_tracing.reflections_amount"));
  shader.setUniform("small_indent", properties.getFloat("ray_tracing.small_indent"));

  shader.setUniform(
    "light_to_color_conversion_coefficient",
    properties.getFloat("light_to_color_conversion_coefficient")
  );

  float mtrHeight = properties.getFloat("camera.matrix_height");
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

static int generateSeed(Clock& timer) {
  return rand() ^ (rand() << 14) ^ (rand() << 18) ^ timer.getElapsedTime().asMicroseconds();
}

int main() {
  bool threeWindows = properties.getBool("show_additional_windows");
  ThreeWindowGroup* threeWindowGroup;
  SingleWindowGroup* singleWindowGroup;

  WindowGroup* windowGroup;
  if (threeWindows) {
    threeWindowGroup = new ThreeWindowGroup;
    windowGroup = threeWindowGroup;
  } else {
    singleWindowGroup = new SingleWindowGroup;
    windowGroup = singleWindowGroup;
  }
  RenderWindow& mainRenderWindow = windowGroup->getMainWindow().renderWindow;

  int seed = 0;
  unsigned frameNumber = 1; // Номер кадра с тех пор, как камера неподвижна
  const float focusToMtrDist = properties.getFloat("camera.focus_to_matrix_distance");
  initControls(mainRenderWindow, frameNumber);
  initShader();
  initText();
  Clock timer;

  while (mainRenderWindow.isOpen())
  {
    Event event{};
    while (mainRenderWindow.pollEvent(event))
      handleEvent(event);

    if (mouseHidden) {
      seed ^= generateSeed(timer); shader.setUniform("seed", seed);
      shader.setUniform("part", 1.0f / frameNumber);
      if (frameNumber++ == 1) {
        shader.setUniform("focus", focus);
        shader.setUniform("vec_to_mtr", mulVN(orientation.forward, focusToMtrDist));
      }

      windowGroup->drawShaderImage();

      float seconds = timer.restart().asSeconds();
      move(seconds);

      if (mouseJustHidden) {
        mouseJustHidden = false;
      } else {
        windowGroup->getMainWindow().drawFPS(seconds);
      }

      windowGroup->display();
    }
  }

  if (threeWindows) {
    delete(threeWindowGroup);
  } else {
    delete(singleWindowGroup);
  }

  return 0;
}