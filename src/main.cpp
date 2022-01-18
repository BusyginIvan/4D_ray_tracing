#include <SFML/Graphics.hpp>
#include <random>
#include "controls.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

static const int w = 324, h = 200;     // Размер области отображения в клеточках (крупных пикселях).
static const int pixel_size = 3;       // Размер одной клеточки в пикселях.
static const float dist_to_mtr = 1.2f; // Расстояние от фокуса до матрицы. Влияет на угол обзора.

int real_w = w * pixel_size, real_h = h * pixel_size;
RenderWindow window(VideoMode(real_w, real_h), "Ray tracing", Style::Titlebar | Style::Close);
int frames_still = 1; // Номер кадра с тех пор, как камера неподвижна.

int main() {
  window.setPosition(Ivec2(200, 50)); // Положение окна.
  window.setFramerateLimit(60);       // Максимальное FPS.
  controls_init();

  RenderTexture texture; texture.create(w, h);
  Sprite sprite = Sprite(texture.getTexture());
  sprite.setScale(pixel_size, pixel_size);

  Shader shader;
  shader.loadFromFile("../src/shader.frag", Shader::Fragment);

  float mtr_height = 1.0f; // Половина высоты матрицы (виртуального экрана в пространстве).
  shader.setUniform("resolution", Vec2(w, h));
  shader.setUniform("mtr_sizes", Vec2(mtr_height / h * w, mtr_height));

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
        shader.setUniform("vec_to_mtr", mul_vn(view_drct.forward, dist_to_mtr));
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