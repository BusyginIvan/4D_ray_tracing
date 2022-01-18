#include <windef.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "controls.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

unsigned int real_w, real_h;    // Размеры области отображения в пикселях.
unsigned int frames_still = 1;  // Номер кадра с тех пор, как камера неподвижна.

int main() {
  unsigned int width_in_cells = 220,       // Ширина области отображения в клеточках (крупных пикселях).
   height_in_cells = width_in_cells / fib; // Высота области отображения в клеточках (крупных пикселях).
  unsigned int pixel_size = 4;             // Размер одной клеточки в пикселях.
  const float dist_to_mtr = 1.2f;          // Расстояние от фокуса до матрицы. Влияет на угол обзора.
  const float mtr_height = 1.0f;           // Половина высоты матрицы (виртуального экрана в пространстве).


  VideoMode video_mode = VideoMode::getDesktopMode();
  unsigned int screen_width = video_mode.width - 10, screen_height = video_mode.height - 130;
  real_w = width_in_cells * pixel_size; real_h = height_in_cells * pixel_size;

  // На случай, если заданы слишком большие размеры окна.
  if (real_w > screen_width / 2){
    width_in_cells = screen_width / 2 / pixel_size;
    real_w = width_in_cells * pixel_size;
  }
  if (real_h > screen_height) {
    height_in_cells = screen_height / pixel_size;
    real_h = height_in_cells * pixel_size;
  }

  // Отступы между окнами и краями экрана.
  unsigned int x_indent = (video_mode.width - 2 * real_w) / 3;
  unsigned int y_indent = min(50u, (screen_height - real_h) / 2);


  RenderWindow window1(VideoMode(real_w, real_h), "Main section", Style::Titlebar | Style::Close);
  window1.setPosition(Ivec2(x_indent, y_indent)); // Положение окна.
  window1.setFramerateLimit(60);                  // Максимальное FPS.
  controls_init(window1);                         // Управление будет привязано к этому окну.

  RenderTexture texture1; texture1.create(width_in_cells, height_in_cells);
  Sprite sprite1 = Sprite(texture1.getTexture());
  sprite1.setScale(pixel_size, pixel_size);


  RenderWindow window2(VideoMode(real_w, real_h), "Additional section", Style::Titlebar | Style::Close);
  window2.setPosition(Ivec2(x_indent * 2 + real_w, y_indent));
  window2.setFramerateLimit(60);

  RenderTexture texture2; texture2.create(width_in_cells, height_in_cells);
  Sprite sprite2 = Sprite(texture2.getTexture());
  sprite2.setScale(pixel_size, pixel_size);


  Shader shader; shader.loadFromFile("../src/shader.frag", Shader::Fragment);
  shader.setUniform("resolution", Vec2(width_in_cells, height_in_cells));
  shader.setUniform("mtr_sizes", Vec2(mtr_height / height_in_cells * width_in_cells, mtr_height));


  while (window1.isOpen())
  {
    Event event;
    while (window1.pollEvent(event))
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

      shader.setUniform("old_frame", texture1.getTexture());
      shader.setUniform("top_drct", view_drct.top);
      texture1.draw(sprite1, &shader); window1.draw(sprite1);
      window1.display();

      shader.setUniform("old_frame", texture2.getTexture());
      shader.setUniform("top_drct", view_drct.w_drct);
      texture2.draw(sprite2, &shader); window2.draw(sprite2);
      window2.display();

      frames_still++;
    }
  }
  return 0;
}