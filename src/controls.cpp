#include "controls.h"
#include <iostream>
#include <cmath>
#include "main.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

bool mouseHidden = false;
static unsigned int halfW, halfH;
RenderWindow* window;


// Всё, что касается мышки и поворота камеры.

// Это чтобы не было резкого скачка, когда мышка входит в окно. Максимальное смещение курсора от центра.
static int maxMouseDeflection;
// Чувствительность мышки. Изменение угла поворота камеры при смещении курсора на один пиксель.
static const float mouseSensitivity = PI / 2 / 300;
// Чувствительность колёсика. Изменение угла поворота камеры при повороте колёсика на единицу.
static const float wheelSensitivity = PI / 2 / 40;

// Единичные векторы, характеризующие положение наблюдателя: куда он смотрит, где у него право, верх...
struct orientation orientation = {
  .w_drct = Vec4(0, 0, 0, 1)
};

// Углы, задающие ориентацию камеры (наблюдателя) в текущем трёхмерном сечении.
static struct { float te = 0, fi = 0; } sphViewDrct;

// Оси текущего трёхмерного сечения.
static struct {
  Vec4 x = Vec4(1, 0, 0, 0);
  Vec4 y = Vec4(0, 1, 0, 0);
  Vec4 z = Vec4(0, 0, 1, 0);
} curSection;

// Поворот двух осей относительно плоскости, содержащей две другие оси.
static void rotate(const float angle, Vec4* const x, Vec4* const y) {
  const float sinA = sin(angle), cosA = cos(angle);
  const Vec4 oldX = *x, oldY = *y;
  *x = sum(mulVN(oldX, cosA), mulVN(oldY, sinA));
  *y = sum(mulVN(oldX, -sinA), mulVN(oldY, cosA));
}

// Построение направляющих по углам поворота камеры.
static void buildOrientation() {
  orientation = {
    .forward = curSection.y,
    .top     = curSection.z,
    .right   = curSection.x,
    .w_drct  = orientation.w_drct
  };
  rotate(sphViewDrct.fi, &orientation.right  , &orientation.forward);
  rotate(sphViewDrct.te, &orientation.forward, &orientation.top    );
}

// Изменение углов поворота камеры в текущем трёхмерном сечении.
static void changeSphViewDrct(const float dTe, const float dFi) {
  sphViewDrct.te += dTe;
  if (sphViewDrct.te < -PI / 2) sphViewDrct.te = -PI / 2;
  if (sphViewDrct.te > PI / 2)  sphViewDrct.te = PI / 2;

  sphViewDrct.fi += dFi;
  if (sphViewDrct.fi < -PI) sphViewDrct.fi += 2 * PI;
  if (sphViewDrct.fi > PI)  sphViewDrct.fi -= 2 * PI;
}

// Поворот сечения в четвёртое измерение.
static void changeSection(const float psi) {
  rotate(sphViewDrct.fi, &curSection.x      , &curSection.y);
  rotate(psi           , &orientation.w_drct, &curSection.y);
  rotate(sphViewDrct.fi, &curSection.y      , &curSection.x);
}


// Всё, что касается клавиатуры и перемещения.

Vec4 focus = Vec4(0, -2.5, 0, 0);
static float speed = 0.08f;

static struct {
  bool forward    = false, back       = false;
  bool right      = false, left       = false;
  bool top        = false, down       = false;
  bool wDrctPos = false, wDrctNeg = false;
} moveState;

static void handleKey(const Event event, const bool state) {
  switch (event.key.code) {
    case Keyboard::W:      moveState.forward  = state;  break;
    case Keyboard::S:      moveState.back     = state;  break;
    case Keyboard::D:      moveState.right    = state;  break;
    case Keyboard::A:      moveState.left     = state;  break;
    case Keyboard::Space:  moveState.top      = state;  break;
    case Keyboard::LShift: moveState.down     = state;  break;
    case Keyboard::E:      moveState.wDrctPos = state;  break;
    case Keyboard::Q:      moveState.wDrctNeg = state;  break;
    default: break;
  }
}

static void moveFocus(const Vec4 drct) {
  focus = sum(focus, mulVN(drct, speed));
  framesStill = 1;
}

void move() {
  if (moveState.forward)  moveFocus(     orientation.forward  );
  if (moveState.back)     moveFocus( neg(orientation.forward) );
  if (moveState.top)      moveFocus(     orientation.top      );
  if (moveState.down)     moveFocus( neg(orientation.top)     );
  if (moveState.right)    moveFocus(     orientation.right    );
  if (moveState.left)     moveFocus( neg(orientation.right)   );
  if (moveState.wDrctPos) moveFocus(     orientation.w_drct   );
  if (moveState.wDrctNeg) moveFocus( neg(orientation.w_drct)  );
}


// Инициализация.
void controlsInit(RenderWindow &mainWindow) {
  window = &mainWindow;
  halfW = window->getSize().x / 2; halfH = window->getSize().y / 2;
  maxMouseDeflection = max(min(halfW, halfH) - 10, 50u);
  //window->setMouseCursorVisible(true);
  buildOrientation();
}


// Обработка событий.

void handleEvent(Event event) {
  switch (event.type) {
    case Event::Closed:
      window->close();
      break;

    case Event::MouseMoved:
      if (mouseHidden) {
        int dx = event.mouseMove.x - halfW, dy = halfH - event.mouseMove.y;
        if (abs(dx) > maxMouseDeflection || dy > maxMouseDeflection)
          Mouse::setPosition(Vector2i(halfW, halfH), *window);
        else if (dx != 0 || dy != 0) {
          changeSphViewDrct(dy * mouseSensitivity, -dx * mouseSensitivity);
          buildOrientation(); framesStill = 1;
          Mouse::setPosition(Vector2i(halfW, halfH), *window);
        }
      }
      break;

    case Event::MouseButtonPressed:
      window->setMouseCursorVisible(false);
      mouseHidden = true;
      Mouse::setPosition(Vector2i(halfW, halfH), *window);
      break;

    case Event::MouseWheelScrolled:
      if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
        changeSection(-event.mouseWheelScroll.delta * wheelSensitivity);
        buildOrientation(); framesStill = 1;
      }
      break;

    case Event::KeyPressed:
      if (event.key.code == Keyboard::Escape) {
        window->setMouseCursorVisible(true);
        mouseHidden = false;
      } else
        handleKey(event, true);
      break;

    case Event::KeyReleased:
      handleKey(event, false);
      break;

    default: break;
  }
}