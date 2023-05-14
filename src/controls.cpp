#include "controls.h"
#include <iostream>
#include <cmath>
#include "main.h"
#include "util.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

bool mouseHidden = false, mouseJustHidden = false;
static unsigned halfW, halfH;
RenderWindow* window;

// Расстояние от фокуса до матрицы: влияет на угол обзора.
const float focusToMtrDist = 1.5f;
// Половина высоты матрицы (виртуального экрана в пространстве): влияет на масштаб всех объектов сцены.
const float mtrHeight = 1.0f;


// Всё, что касается мышки и поворота камеры

// Толщина границы у окна, в пределах которой курсор просто телепортируется в центр.
// Это нужно, чтобы не было резкого скачка, когда мышка входит в окно.
static unsigned mouseBorderWidth = 15;
// Максимальное смещение курсора от центра по каждой из координат. Вычисляется по mouseBorderWidth.
static unsigned maxMouseOffset;

// Чувствительность мышки. Изменение угла поворота камеры при смещении курсора на один пиксель.
static const float mouseSensitivity = PI * 0.0015f;
// Чувствительность колёсика. Изменение угла поворота камеры при повороте колёсика на единицу.
static const float wheelSensitivity = PI / 4 / 7;

// Углы, задающие ориентацию камеры (наблюдателя)
// sph (sphere) в названии символизирует сферическую систему координат.
static struct { float te = 0, fi = 0, psi = 0; } sphOrientation;
// Единичные векторы, характеризующие ориентацию наблюдателя: куда он смотрит, где у него право, верх...
// Вычисляется по sphOrientation.
struct orientation orientation;

// Поворот двух осей относительно плоскости, содержащей две другие оси
static void rotate(const float angle, Vec4* const x, Vec4* const y) {
  const float sinA = sin(angle), cosA = cos(angle);
  const Vec4 oldX = *x, oldY = *y;
  *x = sum(mulVN(oldX, cosA), mulVN(oldY, sinA));
  *y = sum(mulVN(oldX, -sinA), mulVN(oldY, cosA));
}

// Построение направляющих по углам поворота камеры
static void buildOrientation() {
  orientation = {
    .forward = Vec4(0, 1, 0, 0),
    .top     = Vec4(0, 0, 1, 0),
    .right   = Vec4(1, 0, 0, 0),
    .w_drct  = Vec4(0, 0, 0, 1),
  };
  rotate(sphOrientation.psi, &orientation.top    , &orientation.w_drct );
  rotate(sphOrientation.fi , &orientation.right  , &orientation.forward);
  rotate(sphOrientation.te , &orientation.forward, &orientation.top    );
}

// Изменение углов поворота камеры
static void changeSphViewDrct(const float dTe, const float dFi) {
  sphOrientation.te += dTe;
  if (sphOrientation.te < -PI / 2) sphOrientation.te = -PI / 2;
  if (sphOrientation.te >  PI / 2) sphOrientation.te =  PI / 2;

  sphOrientation.fi += dFi;
  if (sphOrientation.fi < -PI) sphOrientation.fi += 2 * PI;
  if (sphOrientation.fi >  PI) sphOrientation.fi -= 2 * PI;
}

static void changeSphViewDrct(const float dPsi) {
  sphOrientation.psi += dPsi;
  if (sphOrientation.psi < -PI/4) sphOrientation.psi = -PI/4;
  if (sphOrientation.psi >  PI/4) sphOrientation.psi =  PI/4;
}


// Всё, что касается клавиатуры, перемещения и положения камеры в пространстве

// Координаты точки фокуса за матрицей, откуда исходят все лучи при рейтрейсинге
Vec4 focus = Vec4(0, -focusToMtrDist, 0, 0);
// Константа, позволяющая регулировать скорость перемещения камеры
static float speed = 2.0f;

// Тут хранится состояние всех клавиш, отвечающих за перемещение: пока клавиша нажата, камера двигается.
static struct {
  bool forward  = false, back     = false;
  bool right    = false, left     = false;
  bool top      = false, down     = false;
  bool wDrctPos = false, wDrctNeg = false;
} moveState;

// Вызывается при изменении состояния клавиши: нажата или отпущена.
static void handleKey(const Event event, const bool state) {
  switch (event.key.code) {
    case Keyboard::W     : moveState.forward  = state; break;
    case Keyboard::S     : moveState.back     = state; break;
    case Keyboard::D     : moveState.right    = state; break;
    case Keyboard::A     : moveState.left     = state; break;
    case Keyboard::Space : moveState.top      = state; break;
    case Keyboard::LShift: moveState.down     = state; break;
    case Keyboard::E     : moveState.wDrctPos = state; break;
    case Keyboard::Q     : moveState.wDrctNeg = state; break;
    default: break;
  }
}

// Перемещение камеры. Вызывается из главного цикла приложения.
void move(const float seconds) {
  Vec4 drct = Vec4(0, 0, 0, 0);
  if (moveState.forward ) drct = sum(drct, orientation.forward);
  if (moveState.back    ) drct = dif(drct, orientation.forward);
  if (moveState.top     ) drct = sum(drct, orientation.top    );
  if (moveState.down    ) drct = dif(drct, orientation.top    );
  if (moveState.right   ) drct = sum(drct, orientation.right  );
  if (moveState.left    ) drct = dif(drct, orientation.right  );
  if (moveState.wDrctPos) drct = sum(drct, orientation.w_drct );
  if (moveState.wDrctNeg) drct = dif(drct, orientation.w_drct );

  float drctLength = mod(drct);
  if (drctLength > 0) {
    focus = sum(focus, mulVN(drct, seconds * speed / drctLength));
    frameNumber = 1;
  }
}


// Инициализация
void initControls(RenderWindow &mainWindow) {
  window = &mainWindow;
  halfW = window->getSize().x / 2; halfH = window->getSize().y / 2;
  maxMouseOffset = max(min(halfW, halfH) - mouseBorderWidth, 50u);
  buildOrientation();
}

// Обработка событий

void centerMouseCursor() {
  Mouse::setPosition(Vector2i(halfW, halfH), *window);
}

void handleEvent(Event event) {
  switch (event.type) {
    case Event::Closed:
      window->close();
      break;

    case Event::MouseMoved:
      if (mouseHidden) {
        int dx = event.mouseMove.x - halfW, dy = halfH - event.mouseMove.y;
        if (abs(dx) > maxMouseOffset || abs(dy) > maxMouseOffset)
          centerMouseCursor();
        else if (dx != 0 || dy != 0) {
          changeSphViewDrct(dy * mouseSensitivity, -dx * mouseSensitivity);
          buildOrientation(); frameNumber = 1;
          centerMouseCursor();
        }
      }
      break;

    case Event::MouseWheelScrolled:
      if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
        changeSphViewDrct(event.mouseWheelScroll.delta * wheelSensitivity);
        buildOrientation(); frameNumber = 1;
      }
      break;

    case Event::MouseButtonPressed:
      if (!mouseHidden) mouseJustHidden = true;
      window->setMouseCursorVisible(false);
      mouseHidden = true;
      centerMouseCursor();
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