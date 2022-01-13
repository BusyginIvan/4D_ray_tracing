#include "controls.h"
#include <SFML/Graphics.hpp>
#include "main.h"
#include "util.h"
using namespace sf;

bool mouse_hidden = true;

// Всё, что касается мышки и поворота камеры.

static const int scr_size = min(w, h) / 2;
static float mouse_sensitivity = pi / 300 / 2;

struct view_drct view_drct;

struct sph_drct sph_view_drct = {
  .te = pi / 2, .fi = pi / 2
};

static void build_view_drct() {
  view_drct = {
    .forward = sph_drct_to_vec(sph_view_drct),
    .top     = sph_drct_to_vec({ sph_view_drct.te - pi/2, sph_view_drct.fi }),
    .right   = sph_drct_to_vec({ pi/2, sph_view_drct.fi - pi/2 })
  };
}

static void change_drct(int dx, int dy) {
  change_sph_drct(&sph_view_drct, -dx * mouse_sensitivity, -dy * mouse_sensitivity);
  build_view_drct();
}


// Всё, что касается клавиатуры и перемещения.

Vector3f focus = Vector3f(1, -3, 0);
static float speed = 0.06f;

static struct {
  bool forward = false;
  bool back    = false;
  bool right   = false;
  bool left    = false;
  bool top     = false;
  bool down    = false;
} move_state;

static void handle_key(Event event, bool state) {
  switch (event.key.code) {
    case Keyboard::W:      move_state.forward = state;  break;
    case Keyboard::S:      move_state.back    = state;  break;
    case Keyboard::D:      move_state.right   = state;  break;
    case Keyboard::A:      move_state.left    = state;  break;
    case Keyboard::Space:  move_state.top     = state;  break;
    case Keyboard::LShift: move_state.down    = state;  break;
    default: break;
  }
}

static void move_focus(Vector3f drct) {
  focus += drct * speed;
  frames_still = 1;
}

void move() {
  if (move_state.forward) move_focus( view_drct.forward);
  if (move_state.back)    move_focus(-view_drct.forward);
  if (move_state.top)     move_focus( view_drct.top    );
  if (move_state.down)    move_focus(-view_drct.top    );
  if (move_state.right)   move_focus( view_drct.right  );
  if (move_state.left)    move_focus(-view_drct.right  );
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
          change_drct(dx, dy);
          Mouse::setPosition(Vector2i(w / 2, h / 2), window);
          frames_still = 1;
        }
      }
      break;
    case Event::MouseButtonPressed:
      window.setMouseCursorVisible(false);
      mouse_hidden = true;
      Mouse::setPosition(Vector2i(w / 2, h / 2), window);
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