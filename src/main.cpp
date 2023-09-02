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
  unsigned frameNumber = 1; // Номер кадра с тех пор, как камера неподвижна

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

      windowGroup->drawShaderImage();

      float seconds = timer.restart().asSeconds();
      move(seconds);
      windowGroup->getMainWindow().drawFPS(seconds);

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