#include <windef.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "controls.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;

unsigned int frames_still = 1;  // Номер кадра с тех пор, как камера неподвижна.

struct window {
  unsigned int width_in_cells, height_in_cells;
  unsigned int pixel_size;
  RenderWindow render_window;
  RenderTexture texture;
  Sprite sprite;
};

static void init_window(struct window* window, std::string title, int style) {
  window->height_in_cells = window->width_in_cells / fib;
  VideoMode video_mode = VideoMode(
    window->width_in_cells * window->pixel_size,
    window->height_in_cells * window->pixel_size
  );
  window->render_window.create(video_mode, title, style);
  window->render_window.setFramerateLimit(60);
  window->texture.create(window->width_in_cells, window->height_in_cells);
  window->sprite = Sprite(window->texture.getTexture());
  window->sprite.setScale(window->pixel_size, window->pixel_size);
}

static void draw_window(struct window* window, Shader &shader, Vec4 top, Vec4 right) {
  shader.setUniform("resolution", Vec2(window->width_in_cells, window->height_in_cells));
  shader.setUniform("old_frame", window->texture.getTexture());
  shader.setUniform("top_drct", top);
  shader.setUniform("right_drct", right);
  window->texture.draw(window->sprite, &shader);
  window->render_window.draw(window->sprite);
  window->render_window.display();
}

int main() {
  const float dist_to_mtr = 1.3f; // Расстояние от фокуса до матрицы. Влияет на угол обзора.
  const float mtr_height = 1.0f;  // Половина высоты матрицы (виртуального экрана в пространстве).

  // Инициализация окон.
  struct window yxz_window = { .width_in_cells = 200, .pixel_size = 4 };
  init_window(&yxz_window, "Main section", Style::Close);
  controls_init(yxz_window.render_window); // Управление будет привязано к этому окну.

  const unsigned int add_windows_width = 120, add_windows_pixel_size = 5;

  struct window ywz_window = { .width_in_cells = add_windows_width, .pixel_size = add_windows_pixel_size };
  init_window(&ywz_window, "Additional section", Style::None);

  struct window yxw_window = { .width_in_cells = add_windows_width, .pixel_size = add_windows_pixel_size };
  init_window(&yxw_window, "Additional section", Style::None);

  // Положение окон на экране.
  const unsigned int main_window_height = yxz_window.render_window.getSize().y + 37;
  const VideoMode video_mode = VideoMode::getDesktopMode();
  unsigned int screen_width = video_mode.width, screen_height = video_mode.height - 60;

  unsigned int y_indent =
    (screen_height - main_window_height - yxw_window.render_window.getSize().y) / 3;
  unsigned int x_indent =
    (screen_width - ywz_window.render_window.getSize().x * 2) / 3;

  yxz_window.render_window.setPosition(Ivec2(
    (screen_width - yxz_window.render_window.getSize().x) / 2, y_indent
  ));
  ywz_window.render_window.setPosition(Ivec2(
    x_indent, main_window_height + y_indent * 2
  ));
  yxw_window.render_window.setPosition(Ivec2(
    ywz_window.render_window.getSize().x + x_indent * 2, main_window_height + y_indent * 2
  ));

  // Инициализация шейдера.
  Shader shader; shader.loadFromFile("../src/shader.frag", Shader::Fragment);
  shader.setUniform("mtr_sizes", Vec2(mtr_height * fib, mtr_height));

  // Главный цикл приложения.
  while (yxz_window.render_window.isOpen())
  {
    Event event;
    while (yxz_window.render_window.pollEvent(event))
      handle_event(event);

    if (mouse_hidden) {
      move();

      shader.setUniform("seed", rand());
      shader.setUniform("part", 1.0f / frames_still);
      shader.setUniform("focus", focus);
      if (frames_still == 1)
        shader.setUniform("vec_to_mtr", mul_vn(view_drct.forward, dist_to_mtr));

      draw_window(&yxz_window, shader, view_drct.top, view_drct.right);
      draw_window(&ywz_window, shader, view_drct.top, view_drct.w_drct);
      draw_window(&yxw_window, shader, view_drct.w_drct, view_drct.right);

      frames_still++;
    }
  }
  return 0;
}