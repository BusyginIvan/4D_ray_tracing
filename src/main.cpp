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
  unsigned int cells_width, cells_height;
  RenderWindow render_window;
  RenderTexture texture;
  Sprite sprite;
};

struct window_size {
  unsigned int width, height, cell_size;
};

static void init_window(
  struct window* const window, struct window_size* const size,
  const string title, const int style
) {
  window->cells_width  = size->width  / size->cell_size;
  window->cells_height = size->height / size->cell_size;
  size->width  = window->cells_width  * size->cell_size;
  size->height = window->cells_height * size->cell_size;
  window->render_window.create(VideoMode(size->width, size->height), title, style);
  window->render_window.setFramerateLimit(60);
  window->texture.create(window->cells_width, window->cells_height);
  window->sprite = Sprite(window->texture.getTexture());
  window->sprite.setScale(size->cell_size, size->cell_size);
}

static void draw_window(struct window* const window, Shader &shader, const Vec4 top, const Vec4 right) {
  shader.setUniform("resolution", Vec2(window->cells_width, window->cells_height));
  shader.setUniform("old_frame", window->texture.getTexture());
  shader.setUniform("top_drct", top);
  shader.setUniform("right_drct", right);
  window->texture.draw(window->sprite, &shader);
  window->render_window.draw(window->sprite);
  window->render_window.display();
}

int main() {
  const float dist_to_mtr = 1.5f; // Расстояние от фокуса до матрицы. Влияет на угол обзора.
  const float mtr_height = 1.0f;  // Половина высоты матрицы (виртуального экрана в пространстве).
  struct window_size main_window = { .width = 850, .cell_size = 4 };
  main_window.height = main_window.width / GOLDEN;
  struct window_size add_window  = { .width = 700, .cell_size = 4 };
  add_window.height  = add_window.width / GOLDEN;

  // Корректировка размеров окон на случай, если экран слишком маленький.
  const unsigned int window_title_height = 37, task_bar_height = 60,
    screen_width = VideoMode::getDesktopMode().width,
    screen_height = VideoMode::getDesktopMode().height - task_bar_height - window_title_height;

  const float decrease_factor = min(min(
    1.0f,
    (float) screen_height / (main_window.height + add_window.height)
  ), min(
    (float) screen_width / 2 / add_window.width,
    (float) screen_width / main_window.width
  ));

  main_window.width *= decrease_factor; main_window.height *= decrease_factor;
  add_window.width  *= decrease_factor; add_window.height  *= decrease_factor;

  // Инициализация окон.
  struct window yxz_window;
  init_window(&yxz_window, &main_window, "Main section", Style::Close);
  controls_init(yxz_window.render_window); // Управление будет привязано к этому окну.

  struct window ywz_window; init_window(&ywz_window, &add_window, "", Style::None);

  struct window yxw_window; init_window(&yxw_window, &add_window, "", Style::None);

  // Положение окон на экране.
  const unsigned int y_indent = (screen_height - main_window.height - add_window.height) / 3;
  const unsigned int x_indent = (screen_width - add_window.width * 2) / 3;

  yxz_window.render_window.setPosition(Ivec2(
    (screen_width - main_window.width) / 2, y_indent
  ));
  ywz_window.render_window.setPosition(Ivec2(
    x_indent, main_window.height + window_title_height + y_indent * 2
  ));
  yxw_window.render_window.setPosition(Ivec2(
    add_window.width + x_indent * 2, main_window.height + window_title_height + y_indent * 2
  ));

  // Инициализация шейдера.
  Shader shader; shader.loadFromFile("shader.frag", Shader::Fragment);
  shader.setUniform("mtr_sizes", Vec2(mtr_height * GOLDEN, mtr_height));

  // Таймер.
  Clock clock;

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
      if (frames_still == 1) {
        shader.setUniform("focus", focus);
        shader.setUniform("vec_to_mtr", mul_vn(orientation.forward, dist_to_mtr));
      }

      draw_window(&yxz_window, shader, orientation.top, orientation.right);
      draw_window(&ywz_window, shader, orientation.top, orientation.w_drct);
      draw_window(&yxw_window, shader, orientation.w_drct, orientation.right);

      frames_still++;
      cout << "FPS: " << 1 / clock.restart().asSeconds() << endl;
    }
  }
  return 0;
}