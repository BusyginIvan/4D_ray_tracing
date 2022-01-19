#include <windef.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "controls.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

unsigned int frames_still = 1;  // Номер кадра с тех пор, как камера неподвижна.

struct window {
  unsigned int width_in_cells, height_in_cells;
  unsigned int pixel_size;
  RenderWindow render_window;
  RenderTexture texture;
  Sprite sprite;
};

static void init_window(struct window* window, string title) {
  window->height_in_cells = window->width_in_cells / fib;
  unsigned int real_w = window->width_in_cells * window->pixel_size;
  unsigned int real_h = window->height_in_cells * window->pixel_size;
  window->render_window.create(VideoMode(real_w, real_h), title, Style::Close);
  window->render_window.setFramerateLimit(60);
  window->texture.create(window->width_in_cells, window->height_in_cells);
  window->sprite = Sprite(window->texture.getTexture());
  window->sprite.setScale(window->pixel_size, window->pixel_size);
}

int main() {
  const float dist_to_mtr = 1.2f;          // Расстояние от фокуса до матрицы. Влияет на угол обзора.
  const float mtr_height = 1.0f;           // Половина высоты матрицы (виртуального экрана в пространстве).

  const VideoMode video_mode = VideoMode::getDesktopMode();
  unsigned int screen_width = video_mode.width - 10, screen_height = video_mode.height - 130;

  struct window main_window = { .width_in_cells = 200, .pixel_size = 4 };
  init_window(&main_window, "Main section");
  main_window.render_window.setPosition(Ivec2(
    (screen_width - main_window.render_window.getSize().x) / 2, 50
  ));
  controls_init(main_window.render_window); // Управление будет привязано к этому окну.

  Shader shader; shader.loadFromFile("../src/shader.frag", Shader::Fragment);

  while (main_window.render_window.isOpen())
  {
    Event event;
    while (main_window.render_window.pollEvent(event))
      handle_event(event);

    if (mouse_hidden) {
      move();

      shader.setUniform("seed", rand());
      shader.setUniform("part", 1.0f / frames_still);
      shader.setUniform("focus", focus);

      if (frames_still == 1) {
        shader.setUniform("vec_to_mtr", mul_vn(view_drct.forward, dist_to_mtr));
        shader.setUniform("right_drct", view_drct.right);
      }

      shader.setUniform("resolution", Vec2(main_window.width_in_cells, main_window.height_in_cells));
      shader.setUniform("mtr_sizes", Vec2(
        mtr_height / main_window.height_in_cells * main_window.width_in_cells, mtr_height
      ));
      shader.setUniform("old_frame", main_window.texture.getTexture());
      shader.setUniform("top_drct", view_drct.top);
      main_window.texture.draw(main_window.sprite, &shader);
      main_window.render_window.draw(main_window.sprite);
      main_window.render_window.display();

      frames_still++;
    }
  }
  return 0;
}