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

static unsigned int  get_width(const struct window* const window) { return window->render_window.getSize().x; }
static unsigned int get_height(const struct window* const window) { return window->render_window.getSize().y; }

static void init_window(struct window* const window, unsigned int cell_size) {
  window->cells_width  = get_width(window)  / cell_size;
  window->cells_height = get_height(window) / cell_size;
  window->render_window.setSize(Vector2u(
    window->cells_width  * cell_size,
    window->cells_height * cell_size
  ));
  window->render_window.setFramerateLimit(60);
  window->texture.create(window->cells_width, window->cells_height);
  window->sprite = Sprite(window->texture.getTexture());
  window->sprite.setScale(cell_size, cell_size);
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
  const float dist_to_mtr = 1.3f; // Расстояние от фокуса до матрицы. Влияет на угол обзора.
  const float mtr_height = 1.0f;  // Половина высоты матрицы (виртуального экрана в пространстве).

  // Инициализация окон.
  unsigned int width = 850;

  struct window yxz_window;
  yxz_window.render_window.create(VideoMode(width, width / golden), "Main section", Style::Close);
  init_window(&yxz_window, 5);
  controls_init(yxz_window.render_window); // Управление будет привязано к этому окну.

  const unsigned int cell_size = 6; width = 600;

  struct window ywz_window;
  ywz_window.render_window.create(VideoMode(width, width / golden), "", Style::None);
  init_window(&ywz_window, cell_size);

  struct window yxw_window;
  yxw_window.render_window.create(VideoMode(width, width / golden), "", Style::None);
  init_window(&yxw_window, cell_size);

  // Положение окон на экране.
  const unsigned int main_window_height = get_height(&yxz_window) + 37;
  const VideoMode video_mode = VideoMode::getDesktopMode();
  const unsigned int screen_width = video_mode.width, screen_height = video_mode.height - 60;

  const unsigned int y_indent =
    (screen_height - main_window_height - get_height(&yxw_window)) / 3;
  const unsigned int x_indent =
    (screen_width - get_width(&ywz_window) * 2) / 3;

  yxz_window.render_window.setPosition(Ivec2(
    (screen_width - get_width(&yxz_window)) / 2, y_indent
  ));
  ywz_window.render_window.setPosition(Ivec2(
    x_indent, main_window_height + y_indent * 2
  ));
  yxw_window.render_window.setPosition(Ivec2(
    get_width(&ywz_window) + x_indent * 2, main_window_height + y_indent * 2
  ));

  // Инициализация шейдера.
  Shader shader; shader.loadFromFile("../src/shader.frag", Shader::Fragment);
  shader.setUniform("mtr_sizes", Vec2(mtr_height * golden, mtr_height));

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
        shader.setUniform("vec_to_mtr", mul_vn(view_drct.forward, dist_to_mtr));
      }

      draw_window(&yxz_window, shader, view_drct.top, view_drct.right);
      draw_window(&ywz_window, shader, view_drct.top, view_drct.w_drct);
      draw_window(&yxw_window, shader, view_drct.w_drct, view_drct.right);

      frames_still++;
      cout << "FPS: " << 1 / clock.restart().asSeconds() << endl;
    }
  }
  return 0;
}