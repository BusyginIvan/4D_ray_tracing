#include "controls.h"
#include <cmath>
#include "main.h"
#include "math.h"

using namespace sf;
using namespace sf::Glsl;
using namespace std;

RenderWindow* window;
float focusToMtrDist, mtrHeight;
bool mouseHidden = false, mouseJustHidden = false;


// Всё, что касается мышки и поворота камеры

// Чувствительность мыши
static float mouseSensitivity, wheelSensitivity;
// Максимальное смещение курсора от центра по каждой из координат. Вычисляется по проперте mouse_border_width.
static unsigned maxMouseOffset;

// Углы, задающие ориентацию камеры (наблюдателя)
// sph (sphere) в названии символизирует сферическую систему координат.
static struct {
  float fi, te, psi, startPsi;
  void (*psiNormalization)(float&);

  void initStartPsi() { startPsi = psi; normalizeAngle(startPsi); }

  void normalizeFi() { normalizeAngle(fi); }
  void normalizeTe() { pullIntoRange(te, 0, PI / 2); }
  void normalizePsi() { psiNormalization(psi); }
  void normalize() { normalizeFi(); normalizeTe(); normalizePsi(); }

  void changeFi (const float delta) { fi  += delta; normalizeFi();  }
  void changeTe (const float delta) { te  += delta; normalizeTe();  }
  void changePsi(const float delta) { psi += delta; normalizePsi(); }
} sphOrientation;

// Единичные векторы, характеризующие ориентацию наблюдателя: куда он смотрит, где у него право, верх...
// Вычисляется по sphOrientation.
struct Orientation orientation;

// Поворот двух осей относительно плоскости, содержащей две другие оси
static void rotate(const float angle, Vec4* const x, Vec4* const y) {
  const float sinA = sin(angle), cosA = cos(angle);
  const Vec4 oldX = *x, oldY = *y;
  *x = sum(mulVN(oldX, cosA), mulVN(oldY, sinA));
  *y = sum(mulVN(oldX, -sinA), mulVN(oldY, cosA));
}

// Построение направляющих по углам поворота камеры
void Orientation::update() {
  forward = Vec4(0, 1, 0, 0),
  top     = Vec4(0, 0, 1, 0),
  right   = Vec4(1, 0, 0, 0),
  w_drct  = Vec4(0, 0, 0, 1),

  rotate(sphOrientation.psi, &top    , &w_drct );
  rotate(sphOrientation.fi , &right  , &forward);
  rotate(sphOrientation.te , &forward, &top    );
}


// Всё, что касается клавиатуры, перемещения и положения камеры в пространстве

Vec4 focus; // Координаты точки фокуса за матрицей, откуда исходят все лучи при рейтрейсинге
static float movementSpeed; // Константа, позволяющая регулировать скорость перемещения камеры

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
    focus = sum(focus, mulVN(drct, seconds * movementSpeed / drctLength));
    frameNumber = 1;
  }
}


// Инициализация
static unsigned halfW, halfH; // Половины ширины и высоты основного окна

void initControls(RenderWindow &mainWindow) {
  window = &mainWindow;
  halfW = window->getSize().x / 2; halfH = window->getSize().y / 2;

  maxMouseOffset = max(min(halfW, halfH) - properties.getUnsignedInt("mouse_border_width"), 50u);
  mouseSensitivity = properties.getFloat("mouse_sensitivity");
  wheelSensitivity = properties.getFloat("wheel_sensitivity");

  movementSpeed = properties.getFloat("movement_speed");

  focusToMtrDist = properties.getFloat("focus_to_matrix_distance");
  mtrHeight = properties.getFloat("matrix_height");

  focus = Vec4(
    properties.getFloat("initial_camera_position.x"),
    properties.getFloat("initial_camera_position.y") - focusToMtrDist,
    properties.getFloat("initial_camera_position.z"),
    properties.getFloat("initial_camera_position.w")
  );

  sphOrientation = {
    .fi = properties.getFloat("initial_camera_position.fi"),
    .te = properties.getFloat("initial_camera_position.te"),
    .psi = properties.getFloat("initial_camera_position.psi"),
  };

  if (properties.getBool("constrain_psi_range")) {
    sphOrientation.initStartPsi();
    sphOrientation.psiNormalization = [](float &psi) {
      pullIntoRange(psi, sphOrientation.startPsi, PI / 4);
    };
  } else {
    sphOrientation.psiNormalization = normalizeAngle;
  }

  sphOrientation.normalize();
  orientation.update();
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
          sphOrientation.changeFi(-dx * mouseSensitivity);
          sphOrientation.changeTe( dy * mouseSensitivity);
          orientation.update(); frameNumber = 1;
          centerMouseCursor();
        }
      }
      break;

    case Event::MouseWheelScrolled:
      if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
        sphOrientation.changePsi(event.mouseWheelScroll.delta * wheelSensitivity);
        orientation.update(); frameNumber = 1;
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