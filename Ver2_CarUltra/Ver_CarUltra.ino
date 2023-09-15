/**
 * ??????????
 */

/* ------------------------------------------------------------------------- */
/*                                   DEFINE                                  */
/* ------------------------------------------------------------------------- */

/**
 * Pin kết nối động cơ RC Servo
 *
 * Servo : Arduino
 * SIG   - D8
 * VCC   - 5V
 * GND   - GND
 */
#define PIN_SERVO 8 //! D8

/* ------------------------------------------------------------------------- */

/**
 * Pin kết nối cảm biến Siêu âm HY-SRF05
 *
 * HY-SRF05 : Arduino
 * VCC    - 5V
 * TRIG   - D9
 * ECHO   - D10
 * OUT    - none
 * GND    - GND
 */
#define PIN_TRIG 9  //! D9
#define PIN_ECHO 10 //! D10

/* ------------------------------------------------------------------------- */

/**
 * Pin kết nối Driver L298
 *
 * Phần cấp nguồn cho Driver
 * +12V - ... có thể cấp nguồn trong khoảng 9V~6V
 * GND  - GND
 *
 * Dùng Jumper kết nối (mặc định)
 * ENA  - 5V
 * ENB  - 5V
 *
 * L298 : Arduino
 * IN1  : D4
 * IN2  : D5 (~)
 * IN3  : D6 (~)
 * IN4  : D7
 */
#define PIN_IN1 4 //! D4
#define PIN_IN2 5 //! D5 (~)
#define PIN_IN3 6 //! D6 (~)
#define PIN_IN4 7 //! D7

/* ------------------------------------------------------------------------- */

// Tốc độ motor, đơn vị PWM (0-255)
#define PER_100 255
#define PER_90 230
#define PER_80 205
#define PER_70 179
#define PER_60 154
#define PER_50 128
#define PER_40 102
#define PER_30 77
#define PER_20 51
#define PER_10 26
#define PER_0 0

// Các mức tốc độ xe sẽ sử dụng
#define FAST PER_60   //!
#define NORMAL PER_50 //!
#define SLOW PER_40   //!

/**
 * Đặt khoảng vùng đo cho cảm biến Siêu âm
 * Khoảng xa nhất và khoảng ngắn nhất
 * Đơn vị (cm)
 */
#define MAX_DISTANCE 100 //!
#define MIN_DISTANCE 10  //!

/**
 * Đặt khoảng cách hoạt động cho cảm biến Siêu âm
 * Khoảng cách "nguy hiểm" để xe dừng lại và "an toàn" để xe đi tiếp
 * Đơn vị (cm)
 */
#define DANGER_DISTANCE 20 //!
#define SAFE_DISTANCE 50   //!

/**
 * Khoảng dừng giữa mỗi lần xe chuyển trạng thái
 * Từ di chuyển sang dừng, và ngược lại
 * Đơn vị (ms)
 */
#define WAIT_DRIVER 500 //!

/**
 * Khoảng thời gian chờ giữa mỗi lần Servo di chuyển
 * Đơn vị (ms)
 */
#define WAIT_SERVO 700 //!

/**
 * Khoảng thời gian chờ giữa mỗi lần đo cảm biến Siêu âm
 * Đơn vị (ms)
 */
#define WAIT_ULTRA 100 //!

/**
 * Góc quay của Servo, để chỉnh hướng quét của Siêu âm
 * Đơn vị độ (º)
 */
#define SCAN_LEFT 170  //!
#define SCAN_CENTER 90 //!
#define SCAN_RIGHT 10  //!

/* ------------------------------------------------------------------------- */
/*                                  LIBRARY                                  */
/* ------------------------------------------------------------------------- */

#include <Servo.h>
#include <NewPing.h>

/* ------------------------------------------------------------------------- */
/*                                   OBJECT                                  */
/* ------------------------------------------------------------------------- */

// Khởi tạo cảm biến Siêu âm
NewPing sonar(PIN_TRIG, PIN_ECHO, MAX_DISTANCE);

// Khởi tạo động cơ Servo
Servo servo_motor;

/* ------------------------------------------------------------------------- */
/*                                  VARIABLE                                 */
/* ------------------------------------------------------------------------- */

/**
 * Quyết định đi thẳng hoặc hướng khác
 * TRUE  : được đi thẳng
 * FALSE : ko được đi thẳng
 */
bool goesForward = false;

/**
 * Lưu giá trị khoảng cách, đơn vị (cm)
 * Gồm hướng trái, giữa, phải
 */
int distanceLeft = 0;
int distanceCenter = 0;
int distanceRight = 0;

/* ------------------------------------------------------------------------- */
/*                                  FUNCTION                                 */
/* ------------------------------------------------------------------------- */

// Điều khiển Motor bên Phải quay tới
void motorRight_RotateForward(int PWM)
{
  digitalWrite(PIN_IN1, LOW);
  analogWrite(PIN_IN2, PWM);
}

// Điều khiển Motor bên Trái quay tới
void motorLeft_RotateForward(int PWM)
{
  analogWrite(PIN_IN3, PWM);
  digitalWrite(PIN_IN4, LOW);
}

// Điều khiển Motor bên Phải quay lùi
void motorRight_RotateReverse(int PWM)
{
  digitalWrite(PIN_IN1, HIGH);
  analogWrite(PIN_IN2, 255 - PWM);
}

// Điều khiển Motor bên Trái quay lùi
void motorLeft_RotateReverse(int PWM)
{
  analogWrite(PIN_IN3, 255 - PWM);
  digitalWrite(PIN_IN4, HIGH);
}

// Điều khiển Motor bên Phải dừng lại
void motorRight_Stop()
{
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
}

// Điều khiển Motor bên Trái dừng lại
void motorLeft_Stop()
{
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

/* ------------------- Điều khiển xe di chuyển tùy chỉnh ------------------- */

/**
 * Giá trị tốc độ dương (+), bánh xe quay hướng đi tới
 * Giá trị tốc độ âm (-), bánh xe quay hướng đi lùi
 */
void go_custom(int speedLeft, int speedRight)
{
  // Xử lý motor bên Phải
  if (speedRight >= 0)
    motorRight_RotateForward(speedRight);
  else
    motorRight_RotateReverse(-speedRight);

  // Xử lý motor bên Trái
  if (speedLeft >= 0)
    motorLeft_RotateForward(speedLeft);
  else
    motorLeft_RotateReverse(-speedLeft);
}

/* ------------------------- Điều khiển xe dừng lại ------------------------ */
void stop()
{
  motorRight_Stop();
  motorLeft_Stop();
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

int readPing()
{
  return constrain(sonar.ping_cm(), MIN_DISTANCE, MAX_DISTANCE);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void motor_control()
{
  // Nếu phát hiện có vật cản phía trước
  if (distanceCenter <= DANGER_DISTANCE)
  {
    // Lùi gấp liền và dừng lại
    go_custom(-FAST, -FAST);
    delay(WAIT_DRIVER);
    stop();
    delay(WAIT_DRIVER);

    // Kiểm tra khoảng cách bên phải
    servo_motor.write(SCAN_RIGHT);
    delay(WAIT_SERVO);
    distanceRight = readPing();
    delay(WAIT_ULTRA);
    servo_motor.write(SCAN_CENTER);
    delay(WAIT_SERVO);

    // Kiểm tra khoảng cách bên trái
    servo_motor.write(SCAN_LEFT);
    delay(WAIT_SERVO);
    distanceLeft = readPing();
    delay(WAIT_ULTRA);
    servo_motor.write(SCAN_CENTER);
    delay(WAIT_SERVO);

    do
    {
      /* -------------- Bên phải có khoảng trống để di chuyển -------------- */
      if (distanceRight >= distanceLeft)
      {
        // Xoay phải
        go_custom(SLOW, -SLOW);
        delay(WAIT_DRIVER);
        stop();
        delay(WAIT_DRIVER);
      }
      /* -------------- Bên trái có khoảng trống để di chuyển -------------- */
      else
      {
        // Xoay trái
        go_custom(-SLOW, SLOW);
        delay(WAIT_DRIVER);
        stop();
        delay(WAIT_DRIVER);
      }
      /* -------- Kiểm tra khoảng cách xem có phải xoay tiếp hay ko? ------- */
      distanceCenter = readPing();
      delay(WAIT_ULTRA);
    } while (distanceCenter < SAFE_DISTANCE);
  }
  else
  {
    // Không có vật cản, xe được phép chạy thẳng tới
    go_custom(NORMAL, NORMAL);
  }

  // Đọc khoảng cách hiện tại của xe
  distanceCenter = readPing();
  delay(WAIT_ULTRA);
}

/* ------------------------------------------------------------------------- */
/*                                RUN ONE TIME                               */
/* ------------------------------------------------------------------------- */

void setup()
{
  // Thiết đặt chân cho Servo
  servo_motor.attach(PIN_SERVO);

  // Thiết đặt các chân điều khiển Driver
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);

  /**
   * Điều khiển Servo quay Siêu âm ngay hướng giữa
   * Và đo khoảng cách phía trước xe
   */
  servo_motor.write(SCAN_CENTER);
  delay(WAIT_SERVO);
  distanceCenter = readPing();
  delay(WAIT_ULTRA);
}

/* ------------------------------------------------------------------------- */
/*                                    MAIN                                   */
/* ------------------------------------------------------------------------- */

void loop()
{
  /**
   * Trình tự các bước điều khiển xe:
   * |
   * Bước 1: Kiểm tra khoảng cách hiện tại
   * Bước 2: Điều khiển xe di chuyển phù hợp theo giá trị nhận được
   */
  motor_control();
}