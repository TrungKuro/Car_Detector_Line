/*
 * Lựa chọn "Data Stream Frequency"
 * → On change/touch
 *
 *- Các tập lệnh AT cho JDY-33 ---------------------------|
 *                                                        |
 *    AT            kiểm tra kết nối                      |
 *    AT+NAME...    thay đổi tên                          |--> name: CarBLE
 *    AT+BAUD...    với 1 set to 1200 bps                 |
 *                      2 set to 2400   bps               |
 *                      3 set to 4800   bps               |
 *                      4 set to 9600   bps (mặc định)    |--> baud rate: 9600 bps
 *                      5 set to 19200  bps               |
 *                      6 set to 38400  bps               |
 *                      7 set to 57600  bps               |
 *                      8 set to 115200 bps               |
 *    AT+PIN...   cài đặt mã PIN (mặc định: 1234)         |--> pass: 1234
 *                                                        |
 *--------------------------------------------------------|
 *
 *- Các tín hiệu điều khiển xe ----------------------------------|
 *                                                               |
 *    S       Ko làm gì (stop)  | . Xe đứng yên                  |
 *    F       Tiến              | . (Chiều dọc) - Đi tới         |
 *    B       Lùi               | . (Chiều dọc) - Đi lùi         |
 *    L       Trái              | . Xoay ngược chiều kim đồng hồ |
 *    R       Phải              | . Xoay chiều kim đồng hồ       |
 *                                                               |
 *    G       Tiến + Trái       | . Rẽ hướng Tây-Bắc             |
 *    I       Tiến + Phải       | . Rẽ hướng Đông-Bắc            |
 *    H       Lùi  + Trái       | . Rẽ hướng Tây-Nam             |
 *    J       Lùi  + Phải       | . Rẽ hướng Đông-Nam            |
 *                                                               |
 * Nhấn giữa tổ hợp phím này càng lâu, xe càng rẽ hướng đó nhiều |
 *---------------------------------------------------------------|
 *
 *- Các tín hiệu điều khiển tốc độ xe |
 *------------------------------------|
 *    0     Tốc độ MIN      |
 *    1     ...             |
 *    2     ...             |
 *    3     ...             |
 *    4     ...             |
 *    5     ...             |
 *    6     ...             |
 *    7     ...             |
 *    8     ...             |
 *    9     ...             |
 *    q     Tốc độ MAX      |
 *  ------------------------|-----------------|
 *    D     Tắt hết tất cả (stop everything)  |
 *--------------------------------------------|
 */

/* ------------------------------------------------------------------------- */
/*                                   DEFINE                                  */
/* ------------------------------------------------------------------------- */

/**
 * Pin kết nối Module Bluetooth (JDY-33)
 *
 * JDY33 : Arduino
 * STATE - ... none
 * RXD   - D3 (TX Software Serial)
 * TXD   - D2 (RX Software Serial)
 * GND   - GND
 * VCC   - 5V
 * PWRC  - ... none
 */
#define PIN_TX_BLE 2 //! D2
#define PIN_RX_BLE 3 //! D3

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
 * L298 : Arduino : Chức năng
 * IN1  : D4      :
 * IN2  : D5 (~)  :
 * IN3  : D6 (~)  :
 * IN4  : D7      :
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

/**
 * Đặt giá trị tốc độ xe mặc định ban đầu
 * Khi xe vừa mới khởi động, khoảng [0 : 255]
 */
#define SPEED_DEFAULT PER_50

/**
 * Đặt ngưỡng giới hạn trên và dưới cho tốc độ
 * Khi xe thực hiện rẽ trái hoặc phải
 */
#define MIN PER_20
#define MAX PER_100

/* ------------------------------------------------------------------------- */
/*                                  LIBRARY                                  */
/* ------------------------------------------------------------------------- */

#include <SoftwareSerial.h>

/* ------------------------------------------------------------------------- */
/*                                  VARIABLE                                 */
/* ------------------------------------------------------------------------- */

byte blue = 0;                                   // Nhận dữ liệu qua Bluetooth
SoftwareSerial mySerial(PIN_TX_BLE, PIN_RX_BLE); // RX (Soft), TX (Soft)
int speed = SPEED_DEFAULT;                       // Tốc độ hiện tại của xe

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
void go_custom(int speedLeft, int speedRight)
{
  // Xử lý motor bên Phải
  if (speedRight >= MIN)
    motorRight_RotateForward(speedRight);
  else if (speedRight <= -MIN)
    motorRight_RotateReverse(speedRight);
  else
    motorRight_Stop();

  // Xử lý motor bên Trái
  if (speedLeft >= MIN)
    motorLeft_RotateForward(speedLeft);
  else if (speedLeft <= -MIN)
    motorLeft_RotateReverse(speedLeft);
  else
    motorLeft_Stop();
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

void motor_control()
{
  switch (blue)
  {
  /* ----------------------------- Xe đi thẳng ----------------------------- */
  case 'F':
    go_custom(speed, speed);
    break;
  /* ------------------------------ Xe đi lùi ------------------------------ */
  case 'B':
    go_custom(-speed, -speed);
    break;
  /* ---------------- Xe xoay trái (ngược chiều kim đồng hồ) --------------- */
  case 'L':
    go_custom(speed, -speed);
    break;
  /* ---------------- Xe xoay phải (cùng chiều kim đồng hồ) ---------------- */
  case 'R':
    go_custom(-speed, speed);
    break;
  /* ------------------------- Xe đi thẳng rẽ trái ------------------------- */
  case 'G':
    go_custom(speed / 2, speed);
    break;
  /* ------------------------- Xe đi thẳng rẽ phải ------------------------- */
  case 'I':
    go_custom(speed, speed / 2);
    break;
  /* -------------------------- Xe đi lùi rẽ trái -------------------------- */
  case 'H':
    go_custom(-speed / 2, -speed);
    break;
  /* -------------------------- Xe đi lùi rẽ phải -------------------------- */
  case 'J':
    go_custom(-speed, -speed / 2);
    break;
  /* ------------------------- Điều chỉnh tốc độ xe ------------------------ */
  case '0':
    speed = PER_0;
    break;
  case '1':
    speed = PER_10;
    break;
  case '2':
    speed = PER_20;
    break;
  case '3':
    speed = PER_30;
    break;
  case '4':
    speed = PER_40;
    break;
  case '5':
    speed = PER_50;
    break;
  case '6':
    speed = PER_60;
    break;
  case '7':
    speed = PER_70;
    break;
  case '8':
    speed = PER_80;
    break;
  case '9':
    speed = PER_90;
    break;
  case 'q':
    speed = PER_100;
    break;
  /* ------------------------ Dừng xe (ko làm gì cả) ----------------------- */
  case 'D':
  case 'S':
    stop();
    break;
  /* ------------------------- Các trường hợp khác ------------------------- */
  default:
    break;
  }
}

/* ------------------------------------------------------------------------- */
/*                                RUN ONE TIME                               */
/* ------------------------------------------------------------------------- */

void setup()
{
  mySerial.begin(9600);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
}

/* ------------------------------------------------------------------------- */
/*                                    MAIN                                   */
/* ------------------------------------------------------------------------- */

void loop()
{
  // Nếu có dữ liệu gửi về thì đọc
  if (mySerial.available() > 0)
  {
    // Đọc dữ liệu
    blue = mySerial.read();
    // Xử lý dữ liệu và điều khiển xe
    motor_control();
  }
}