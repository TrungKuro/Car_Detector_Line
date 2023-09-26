/* ------------------------------------------------------------------------- */
/*                                 DEFINE PIN                                */
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
/*                           DEFINE CONFIG (DRIVER)                          */
/* ------------------------------------------------------------------------- */

// Tốc độ motor, đơn vị PWM (0-255)
#define PER_100 255
#define PER_95 243
#define PER_90 230
#define PER_85 218
#define PER_80 205
#define PER_75 192
#define PER_70 179
#define PER_65 166
#define PER_60 154
#define PER_55 141
#define PER_50 128
#define PER_45 115
#define PER_40 102
#define PER_35 90
#define PER_30 77
#define PER_25 64
#define PER_20 51
#define PER_15 38
#define PER_10 26
#define PER_5 13
#define PER_0 0

/**
 * Đặt ngưỡng giới hạn trên và dưới cho tốc độ
 */
#define MAX PER_50 //!
#define MIN PER_35 //!

/* ------------------------------------------------------------------------- */
/*                           DEFINE CONFIG (ULTRA)                           */
/* ------------------------------------------------------------------------- */

/**
 * Đặt khoảng vùng đo cho cảm biến Siêu âm
 * Khoảng xa nhất và khoảng ngắn nhất
 * Đơn vị (cm)
 */
#define MAX_DISTANCE 30 //!
#define MIN_DISTANCE 3  //!

/**
 * Đặt khoảng cách hoạt động cho cảm biến Siêu âm
 * Khoảng cách "nguy hiểm" để xe dừng lại
 * Và khoảng "vượt quá" để xe đi dò tìm
 * Đơn vị (cm)
 */
#define DANGER_DISTANCE 5 //!
#define OVER_DISTANCE 25  //!

/**
 * Khoảng thời gian chờ giữa mỗi lần đo cảm biến Siêu âm
 * Đơn vị (ms)
 *
 * Wait 50ms between pings (about 20 pings/sec)
 * 29ms should be the shortest delay between pings
 */
#define WAIT_ULTRA 35 //!

/* ------------------------------------------------------------------------- */
/*                           DEFINE CONFIG (SERVO)                           */
/* ------------------------------------------------------------------------- */

/**
 * Khoảng thời gian chờ giữa mỗi lần Servo di chuyển góc lớn
 * Và khoảng thời gian chờ giữa mỗi lần Servo di chuyển bước nhỏ
 * Đơn vị (ms)
 */
#define WAIT_SERVO 1500   //!
#define WAIT_SERVO_STEP 7 //!

/**
 * Độ lớn góc cho mỗi bước di chuyển của Servo
 * Đơn vị độ (º)
 */
#define ANGLE_STEP 1 //!

/**
 * Góc quay của Servo, để chỉnh hướng quét của Siêu âm
 * Đơn vị độ (º) từ 0º đến 180º
 */
#define SCAN_LEFT_END 165   //! (+70)
#define SCAN_LEFT_BEGIN 105 //! (+10)
#define SCAN_CENTER 95      //!
#define SCAN_RIGHT_BEGIN 85 //! (-10)
#define SCAN_RIGHT_END 25   //! (-70)

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
 * Lưu giá trị khoảng cách, đơn vị (cm)
 */
int distance = 0;

/**
 * Cập nhập thời điểm đọc giá trị cảm biến Siêu âm
 * Đơn vị (ms)
 */
unsigned long timeUpdate;

/**
 * Lưu góc quay hiện tại của Servo, đơn vị (º)
 */
int angle;

/**
 * Tính toán tốc độ riêng cho mỗi bánh xe
 * Bánh xe bên trái và bánh xe bên phải
 * Đơn vị (PWM)
 */
int wheelLeft;
int wheelRight;

/* ------------------------------------------------------------------------- */
/*                                  FUNCTION                                 */
/* ------------------------------------------------------------------------- */

// Điều khiển Motor bên Phải quay tới
void motorRight_RotateForward(int PWM)
{
  digitalWrite(PIN_IN4, LOW);
  analogWrite(PIN_IN3, PWM);
}

// Điều khiển Motor bên Trái quay tới
void motorLeft_RotateForward(int PWM)
{
  analogWrite(PIN_IN2, PWM);
  digitalWrite(PIN_IN1, LOW);
}

// Điều khiển Motor bên Phải quay lùi
void motorRight_RotateReverse(int PWM)
{
  digitalWrite(PIN_IN4, HIGH);
  analogWrite(PIN_IN3, 255 - PWM);
}

// Điều khiển Motor bên Trái quay lùi
void motorLeft_RotateReverse(int PWM)
{
  analogWrite(PIN_IN2, 255 - PWM);
  digitalWrite(PIN_IN1, HIGH);
}

// Điều khiển Motor bên Phải dừng lại
void motorRight_Stop()
{
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

// Điều khiển Motor bên Trái dừng lại
void motorLeft_Stop()
{
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
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
  int cm = sonar.ping_cm();
  if (cm == 0)
  {
    return MAX_DISTANCE;
  }
  else
  {
    return constrain(cm, MIN_DISTANCE, MAX_DISTANCE);
  }
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void motor_control()
{
  /* ------------------- Step 1: Đọc giá trị khoảng cách ------------------- */
  if (millis() - timeUpdate >= WAIT_ULTRA)
  {
    timeUpdate = millis();
    distance = readPing();
  }

  if (distance <= DANGER_DISTANCE)
  {
    // Dừng lại, gần chạm rồi
    stop();
  }
  else
  {
    /* -------------------- Step 2: Điều khiển góc quét -------------------- */
    if (distance >= OVER_DISTANCE)
    {
      /**
       * Ko tìm thấy đối tượng, phải đi dò tìm
       * Cho máy quét hướng về bên trái, đồng thời di chuyển xe về bên trái
       */
      angle += ANGLE_STEP;
      if (angle >= SCAN_LEFT_END)
        angle = SCAN_LEFT_END;
    }
    else
    {
      /**
       * Phát hiện thấy đối tượng, phải bám theo
       * Cho máy quét hướng về bên phải, đồng thời di chuyển xe về bên phải
       */
      angle -= ANGLE_STEP;
      if (angle <= SCAN_RIGHT_END)
        angle = SCAN_RIGHT_END;
    }
    servo_motor.write(angle);
    delay(WAIT_SERVO_STEP);

    /* ------------- Step 3: Di chuyển xe theo giá trị góc quay ------------ */
    if (angle >= SCAN_LEFT_BEGIN)
    {
      // Cho xe rẽ dần sang trái
      wheelRight = MAX;
      //
      wheelLeft = map(angle, SCAN_LEFT_BEGIN, SCAN_LEFT_END, MAX, -MAX);
      if (wheelLeft >= 0)
        wheelLeft = constrain(wheelLeft, MIN, MAX);
      else
        wheelLeft = constrain(wheelLeft, -MAX, -MIN);
    }
    else if (angle <= SCAN_RIGHT_BEGIN)
    {
      // Cho xe rẽ dần sang phải
      wheelLeft = MAX;
      //
      wheelRight = map(angle, SCAN_RIGHT_BEGIN, SCAN_RIGHT_END, MAX, -MAX);
      if (wheelRight >= 0)
        wheelRight = constrain(wheelRight, MIN, MAX);
      else
        wheelRight = constrain(wheelRight, -MAX, -MIN);
    }
    else
    {
      // Cho xe đi thẳng
      wheelLeft = MIN;
      wheelRight = MIN;
    }
    go_custom(wheelLeft, wheelRight);
  }
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
   */
  servo_motor.write(SCAN_CENTER);
  delay(WAIT_SERVO);
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
   * Bước 2: Điều khiển góc quay để quét
   * Bước 3: Điều khiển xe đi theo hướng quét
   */
  motor_control();
}