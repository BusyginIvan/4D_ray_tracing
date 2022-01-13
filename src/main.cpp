#include <SFML/Graphics.hpp>
#include "controls.h"

using namespace sf;

int w = 1132, h = 700; // Размеры окна.
//int w = 810, h = 500;

RenderWindow window(VideoMode(w, h), "Ray tracing", Style::Titlebar | Style::Close);
int frames_still = 1; // Номер кадра с тех пор, как камера неподвижна.

int main() {
  window.setPosition(Vector2i(200, 50));
  window.setFramerateLimit(60); // Максимальное FPS.
  controls_init();

  RenderTexture texture; texture.create(w, h);
  Sprite sprite = Sprite(texture.getTexture());

  Shader shader;
  shader.loadFromFile("../src/shader.frag", Shader::Fragment);

  float mtr_height = 1.0f; // Половина высоты матрицы (виртуального экрана в пространстве).
  shader.setUniform("resolution", Vector2f(w, h));
  shader.setUniform("mtr_sizes", Vector2f(mtr_height / h * w, mtr_height));

  while (window.isOpen())
  {
    Event event;
    while (window.pollEvent(event))
      handle_event(event);

    if (mouse_hidden) {
      move();

      shader.setUniform("seed", rand());
      shader.setUniform("part", 1.0f / frames_still);
      shader.setUniform("old_frame", texture.getTexture());

      if (frames_still == 1) {
        shader.setUniform("focus", focus);
        shader.setUniform("vec_to_mtr", view_drct.forward);
        shader.setUniform("top_drct", view_drct.top);
        shader.setUniform("right_drct", view_drct.right);
      }

      texture.draw(sprite, &shader);
      window.draw(sprite);
      window.display();
      frames_still++;
    }
  }
  return 0;
}