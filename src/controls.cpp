#include "controls.h"
#include <iostream>
#include "main.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

bool mouse_hidden = true;

// Всё, что касается мышки и поворота камеры.

static const int scr_size = min(w, h) / 2;
static const float mouse_sensitivity = pi/2 / 300;
static const float wheel_sensitivity = pi/2 / 70;

struct view_drct view_drct = {
  .w_drct = Vec4(0, 0, 0, 1)
};

static struct sph_drct3 sph_view_drct = {
  .te = 0, .fi = 0
};

static struct section cur_section = {
  .x = Vec4(1, 0, 0, 0),
  .y = Vec4(0, 1, 0, 0),
  .z = Vec4(0, 0, 1, 0)
};

static void build_view_drct() {
  float sin_fi = sin(sph_view_drct.fi), cos_fi = cos(sph_view_drct.fi);
  Vec4 x = cur_section.x, y = cur_section.y, z = cur_section.z;

  Vec4 x2 = sum(mul_vn(x, cos_fi), mul_vn(y, sin_fi));
  Vec4 y2 = sum(mul_vn(x, -sin_fi), mul_vn(y, cos_fi));
  Vec4 z2 = z;

  float sin_te = sin(sph_view_drct.te), cos_te = cos(sph_view_drct.te);
  view_drct = {
    .forward = sum(mul_vn(y2, cos_te), mul_vn(z2, sin_te)),
    .top     = sum(mul_vn(y2, -sin_te), mul_vn(z2, cos_te)),
    .right   = x2,
    .w_drct  = view_drct.w_drct
  };
}

static void change_view_drct(float d_te, float d_fi) {
  change_sph_drct3(&sph_view_drct, d_te, d_fi);
  build_view_drct();
  frames_still = 1;
}

static void build_cur_section() {
  float sin_te = sin(sph_view_drct.te), cos_te = cos(sph_view_drct.te);
  Vec4 x = view_drct.right, y = view_drct.forward, z = view_drct.top;

  Vec4 x2 = x;
  Vec4 y2 = sum(mul_vn(y, cos_te), mul_vn(z, -sin_te));
  Vec4 z2 = sum(mul_vn(y, sin_te), mul_vn(z, cos_te));

  float sin_fi = sin(sph_view_drct.fi), cos_fi = cos(sph_view_drct.fi);
  cur_section = {
    .x = sum(mul_vn(x2, cos_fi), mul_vn(y2, -sin_fi)),
    .y = sum(mul_vn(x2, sin_fi), mul_vn(y2, cos_fi)),
    .z = z2
  };
}


// Всё, что касается клавиатуры и перемещения.

Vec4 focus = Vec4(1, -3, 0, 0);
static float speed = 0.06f;

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
        int dx = event.mouseMove.x - w / 2, dy = h / 2 - event.mouseMove.y;
        if (abs(dx) > scr_size - 10 || dy > scr_size - 10)
          Mouse::setPosition(Vector2i(w / 2, h / 2), window);
        else if (dx != 0 || dy != 0) {
          change_view_drct(dy * mouse_sensitivity, -dx * mouse_sensitivity);
          Mouse::setPosition(Vector2i(w / 2, h / 2), window);
        }
      }
      break;

    case Event::MouseButtonPressed:
      window.setMouseCursorVisible(false);
      mouse_hidden = true;
      Mouse::setPosition(Vector2i(w / 2, h / 2), window);
      break;

    case Event::MouseWheelScrolled:
      if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
        float d = event.mouseWheelScroll.delta * wheel_sensitivity;
        Vec4 forward = view_drct.forward, w_drct = view_drct.w_drct;
        view_drct.forward = normalize(sum(forward, mul_vn(w_drct, -d)));
        view_drct.w_drct  = normalize(sum(w_drct , mul_vn(forward, d)));
        build_cur_section();
        frames_still = 1;
      }
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