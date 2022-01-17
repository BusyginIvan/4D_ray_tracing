#include "controls.h"
#include "main.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;

bool mouse_hidden = true;
static const int half_w = real_w / 2, half_h = real_h / 2;

// Всё, что касается мышки и поворота камеры.

// Это чтобы не было резкого скачка, когда мышка входит в окно. Максимальное смещение курсора от центра.
static const int max_mouse_deflection = max(min(half_w, half_h) - 10, 50);
// Чувствительность мышки. Изменение угла поворота камеры при смещении курсора на один пиксель.
static const float mouse_sensitivity = pi/2 / 300;
// Чувствительность колёсика. Изменение угла поворота камеры при повороте колёсика на единицу.
static const float wheel_sensitivity = pi/2 / 40 ;

// Углы, задающие ориентацию камеры (наблюдателя) в пространстве.
static struct sph_drct sph_view_drct = {
  .psi = 0, .te = 0, .fi = 0
};
// Единичные векторы, характеризующие положение наблюдателя: куда он смотрит, где у него право, верх...
struct view_drct view_drct;

// Поворот двух осей относительно плоскости, содержащей две другие оси.
static void rotate(float angle, Vec4* const x, Vec4* const y) {
  const float sin_a = sin(angle), cos_a = cos(angle);
  const Vec4 old_x = *x, old_y = *y;
  *x = sum(mul_vn(old_x, cos_a), mul_vn(old_y, sin_a));
  *y = sum(mul_vn(old_x, -sin_a), mul_vn(old_y, cos_a));
}

// Построение направляющих по углам поворота камеры.
static void build_view_drct() {
  view_drct = {
    .forward = Vec4(0, 1, 0, 0),
    .top     = Vec4(0, 0, 1, 0),
    .right   = Vec4(1, 0, 0, 0),
    .w_drct  = Vec4(0, 0, 0, 1)
  };
  rotate(sph_view_drct.psi, &view_drct.forward, &view_drct.w_drct );
  rotate(sph_view_drct.fi , &view_drct.right  , &view_drct.forward);
  rotate(sph_view_drct.te , &view_drct.forward, &view_drct.top    );
}

// Поворот камеры (взора).
static void change_view_drct(float d_psi, float d_te, float d_fi) {
  change_sph_drct(&sph_view_drct, d_psi, d_te, d_fi);
  build_view_drct();
  frames_still = 1;
}


// Всё, что касается клавиатуры и перемещения.

Vec4 focus = Vec4(0, -5, 0, 0); // Точка за матрицей (виртуальным экраном), откуда летят лучи.
static float speed = 0.12f;     // Скорость перемещения.

static struct {
  bool forward = false;  bool back  = false;
  bool right   = false;  bool left  = false;
  bool top     = false;  bool down  = false;
  bool w_drct  = false;  bool neg_w = false;
} move_state;

static void handle_key(Event event, bool state) {
  switch (event.key.code) {
    case Keyboard::W:      move_state.forward = state;  break;
    case Keyboard::S:      move_state.back    = state;  break;
    case Keyboard::D:      move_state.right   = state;  break;
    case Keyboard::A:      move_state.left    = state;  break;
    case Keyboard::Space:  move_state.top     = state;  break;
    case Keyboard::LShift: move_state.down    = state;  break;
    case Keyboard::E:      move_state.w_drct  = state;  break;
    case Keyboard::Q:      move_state.neg_w   = state;  break;
    default: break;
  }
}

static void move_focus(Vec4 drct) {
  focus = sum(focus, mul_vn(drct, speed));
  frames_still = 1;
}

void move() {
  if (move_state.forward) move_focus(     view_drct.forward  );
  if (move_state.back)    move_focus( neg(view_drct.forward) );
  if (move_state.top)     move_focus(     view_drct.top      );
  if (move_state.down)    move_focus( neg(view_drct.top    ) );
  if (move_state.right)   move_focus(     view_drct.right    );
  if (move_state.left)    move_focus( neg(view_drct.right  ) );
  if (move_state.w_drct)  move_focus(     view_drct.w_drct   );
  if (move_state.neg_w)   move_focus( neg(view_drct.w_drct ) );
}


// Инициализация.
void controls_init() {
  window.setMouseCursorVisible(false);
  build_view_drct();
}


// Обработка событий.

void handle_event(Event event) {
  switch (event.type) {
    case Event::Closed:
      window.close();
      break;

    case Event::MouseMoved:
      if (mouse_hidden) {
        int dx = event.mouseMove.x - half_w, dy = half_h - event.mouseMove.y;
        if (abs(dx) > max_mouse_deflection || dy > max_mouse_deflection)
          Mouse::setPosition(Vector2i(half_w, half_h), window);
        else if (dx != 0 || dy != 0) {
          change_view_drct(0, dy * mouse_sensitivity, -dx * mouse_sensitivity);
          Mouse::setPosition(Vector2i(half_w, half_h), window);
        }
      }
      break;

    case Event::MouseButtonPressed:
      window.setMouseCursorVisible(false);
      mouse_hidden = true;
      Mouse::setPosition(Vector2i(half_w, half_h), window);
      break;

    case Event::MouseWheelScrolled:
      if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
        change_view_drct(event.mouseWheelScroll.delta * wheel_sensitivity, 0, 0);
      break;

    case Event::KeyPressed:
      if (event.key.code == Keyboard::Escape) {
        window.setMouseCursorVisible(true);
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