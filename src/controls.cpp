#include "controls.h"
#include <iostream>
#include <cmath>
#include "main.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

bool mouse_hidden = false;
static unsigned int half_w, half_h;
RenderWindow* window;


// Всё, что касается мышки и поворота камеры.

// Это чтобы не было резкого скачка, когда мышка входит в окно. Максимальное смещение курсора от центра.
static int max_mouse_deflection;
// Чувствительность мышки. Изменение угла поворота камеры при смещении курсора на один пиксель.
static const float mouse_sensitivity = PI / 2 / 300;
// Чувствительность колёсика. Изменение угла поворота камеры при повороте колёсика на единицу.
static const float wheel_sensitivity = PI / 2 / 40;

// Единичные векторы, характеризующие положение наблюдателя: куда он смотрит, где у него право, верх...
struct orientation orientation = {
  .w_drct = Vec4(0, 0, 0, 1)
};

// Углы, задающие ориентацию камеры (наблюдателя) в текущем трёхмерном сечении.
static struct { float te = 0, fi = 0; } sph_view_drct;

// Оси текущего трёхмерного сечения.
static struct {
  Vec4 x = Vec4(1, 0, 0, 0);
  Vec4 y = Vec4(0, 1, 0, 0);
  Vec4 z = Vec4(0, 0, 1, 0);
} cur_section;

// Поворот двух осей относительно плоскости, содержащей две другие оси.
static void rotate(const float angle, Vec4* const x, Vec4* const y) {
  const float sin_a = sin(angle), cos_a = cos(angle);
  const Vec4 old_x = *x, old_y = *y;
  *x = sum(mul_vn(old_x, cos_a), mul_vn(old_y, sin_a));
  *y = sum(mul_vn(old_x, -sin_a), mul_vn(old_y, cos_a));
}

// Построение направляющих по углам поворота камеры.
static void build_view_drct() {
  orientation = {
    .forward = cur_section.y,
    .top     = cur_section.z,
    .right   = cur_section.x,
    .w_drct  = orientation.w_drct
  };
  rotate(sph_view_drct.fi, &orientation.right  , &orientation.forward);
  rotate(sph_view_drct.te, &orientation.forward, &orientation.top    );
}

// Изменение углов поворота камеры в текущем трёхмерном сечении.
static void change_sph_view_drct(const float d_te, const float d_fi) {
  sph_view_drct.te += d_te;
  if (sph_view_drct.te < -PI / 2) sph_view_drct.te = -PI / 2;
  if (sph_view_drct.te > PI / 2) sph_view_drct.te = PI / 2;

  sph_view_drct.fi += d_fi;
  if (sph_view_drct.fi < -PI) sph_view_drct.fi += 2 * PI;
  if (sph_view_drct.fi > PI) sph_view_drct.fi -= 2 * PI;
}

// Поворот сечения в четвёртое измерение.
static void change_section(const float psi) {
  rotate(sph_view_drct.fi, &cur_section.x   , &cur_section.y);
  rotate(psi             , &orientation.w_drct, &cur_section.y);
  rotate(sph_view_drct.fi, &cur_section.y   , &cur_section.x);
}


// Всё, что касается клавиатуры и перемещения.

Vec4 focus = Vec4(0, -2.5, 0, 0);
static float speed = 0.08f;

static struct {
  bool forward    = false, back       = false;
  bool right      = false, left       = false;
  bool top        = false, down       = false;
  bool w_drct_pos = false, w_drct_neg = false;
} move_state;

static void handle_key(const Event event, const bool state) {
  switch (event.key.code) {
    case Keyboard::W:      move_state.forward    = state;  break;
    case Keyboard::S:      move_state.back       = state;  break;
    case Keyboard::D:      move_state.right      = state;  break;
    case Keyboard::A:      move_state.left       = state;  break;
    case Keyboard::Space:  move_state.top        = state;  break;
    case Keyboard::LShift: move_state.down       = state;  break;
    case Keyboard::E:      move_state.w_drct_pos = state;  break;
    case Keyboard::Q:      move_state.w_drct_neg = state;  break;
    default: break;
  }
}

static void move_focus(const Vec4 drct) {
  focus = sum(focus, mul_vn(drct, speed));
  frames_still = 1;
}

void move() {
  if (move_state.forward) move_focus(orientation.forward  );
  if (move_state.back)    move_focus( neg(orientation.forward) );
  if (move_state.top)     move_focus(orientation.top      );
  if (move_state.down)    move_focus( neg(orientation.top    ) );
  if (move_state.right)   move_focus(orientation.right    );
  if (move_state.left)    move_focus( neg(orientation.right  ) );
  if (move_state.w_drct_pos)  move_focus(orientation.w_drct   );
  if (move_state.w_drct_neg)   move_focus(neg(orientation.w_drct ) );
}


// Инициализация.
void controls_init(RenderWindow &main_window) {
  window = &main_window;
  half_w = window->getSize().x / 2; half_h = window->getSize().y / 2;
  max_mouse_deflection = max(min(half_w, half_h) - 10, 50u);
  //window->setMouseCursorVisible(true);
  build_view_drct();
}


// Обработка событий.

void handle_event(const Event event) {
  switch (event.type) {
    case Event::Closed:
      window->close();
      break;

    case Event::MouseMoved:
      if (mouse_hidden) {
        int dx = event.mouseMove.x - half_w, dy = half_h - event.mouseMove.y;
        if (abs(dx) > max_mouse_deflection || dy > max_mouse_deflection)
          Mouse::setPosition(Vector2i(half_w, half_h), *window);
        else if (dx != 0 || dy != 0) {
          change_sph_view_drct(dy * mouse_sensitivity, -dx * mouse_sensitivity);
          build_view_drct(); frames_still = 1;
          Mouse::setPosition(Vector2i(half_w, half_h), *window);
        }
      }
      break;

    case Event::MouseButtonPressed:
      window->setMouseCursorVisible(false);
      mouse_hidden = true;
      Mouse::setPosition(Vector2i(half_w, half_h), *window);
      break;

    case Event::MouseWheelScrolled:
      if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
        change_section(-event.mouseWheelScroll.delta * wheel_sensitivity);
        build_view_drct(); frames_still = 1;
      }
      break;

    case Event::KeyPressed:
      if (event.key.code == Keyboard::Escape) {
        window->setMouseCursorVisible(true);
        mouse_hidden = false;
      } else
        handle_key(event, true);
      break;

    case Event::KeyReleased:
      handle_key(event, false);
      break;

    default: break;
  }
}