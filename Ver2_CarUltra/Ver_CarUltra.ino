/* ------------------------------------------------------------------------- */
/*                                   DEFINE                                  */
/* ------------------------------------------------------------------------- */

/**
 * Pin kết nối Thanh dò line TCRT5000
 *
 * TCRT5000 : Arduino
 * GND      - GND
 * 5V       - 5V
 * OUT1     - A1 (Digital)
 * OUT2     - A2 (Digital)
 * OUT3     - A3 (Digital)
 */
#define PIN_OUT1 A1 //! A1
#define PIN_OUT2 A2 //! A2
#define PIN_OUT3 A3 //! A3

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
 * Pin kết nối cảm biến Siêu âm US-015
 *
 * US-015 : Arduino
 * VCC    - 5V
 * TRIG   - D9
 * ECHO   - D10
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
#define PIN_RX_BLE 3 //! D3
#define PIN_TX_BLE 2 //! D2

/* ------------------------------------------------------------------------- */

/**
 * Hệ số của các khâu PID
 */
#define KP 25.0    //!
#define KI 0.00001 //!
#define KD 11.0    //!

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

/**
 * Đặt giá trị tốc độ xe mặc định ban đầu
 * Khi xe vừa mới khởi động, khoảng [0 : 255]
 */
#define SPEED_DEFAULT PER_30

/**
 * Đặt ngưỡng giới hạn trên và dưới cho tốc độ
 */
#define MIN -PER_50
#define MAX PER_50

/* ------------------------------------------------------------------------- */
/*                                  VARIABLE                                 */
/* ------------------------------------------------------------------------- */

/**
 * Tổng kích thước dữ liệu này là 1 Byte
 *
 * Bit : [7] - [6] - [5] - [4] - [3] - [2] - [1] - [0]
 * Line:  x     x     x     x     x    OUT3  OUT2  OUT1
 */
struct DataLine
{
  // Mép Phải
  bool line1 : 1; // OUT1 - Bit [0]
  bool line2 : 1; // OUT2 - Bit [1]
  bool line3 : 1; // OUT3 - Bit [2]
  // Mép Trái
};

/**
 * Tổng kích thước dữ liệu này là 1 Byte
 * Biến "dataLine" và "stateLine" cùng chia sẽ vị trí bộ nhớ
 */
union MapLine
{
  DataLine dataLine;
  byte stateLine;
} raw;

/* ------------------------------------------------------------------------- */

struct CarLine
{
  /**
   * Cho biết hướng lệch hiện tại của xe
   * Lệch phải → (+) : TRUE
   * Lệch trái → (-) : FALSE
   */
  bool direction;

  int8_t P = 0, I = 0, D = 0;      // Giá trị hiện tại của từng khâu PID
  float Kp = KP, Ki = KI, Kd = KD; // Hệ số của các khâu PID
  int8_t errorPrev;                // Lưu giá trị "error" trước đó

  // Lưu tốc độ của riêng mỗi bánh xe
  byte speedRightNow;
  byte speedLeftNow;
} car;

/* ------------------------------------------------------------------------- */
/*                                  FUNCTION                                 */
/* ------------------------------------------------------------------------- */

// Đọc thanh dò line TCRT5000
int8_t read_TCRT5000()
{
  /**
   * Trái ---------- Giữa ---------- Phải
   * |                                  |
   * | OUT1 | OUT2 | OUT3 | OUT4 | OUT5 |
   *
   * Khoảng cách phát hiện Line ĐEN (~1cm)
   * Có Line - HIGH - Bit 1
   * Ko Line - LOW  - Bit 0
   */
  raw.dataLine.line5 = digitalRead(PIN_OUT5);
  raw.dataLine.line4 = digitalRead(PIN_OUT4);
  raw.dataLine.line3 = digitalRead(PIN_OUT3);
  raw.dataLine.line2 = digitalRead(PIN_OUT2);
  raw.dataLine.line1 = digitalRead(PIN_OUT1);

  /**
   * Chuyển giá trị DEC từ "stateLine"
   * Sang giá trị Level cho "levelLine"
   */
  int8_t levelLine;
  switch (raw.stateLine)
  {
  case 0:
    if (car.direction)
      levelLine = 5; // Đang lệch phải ngoài line
    else
      levelLine = -5; // Đang lệch trái ngoài line
    break;
  case 16: // Lệch phải mức 4
    levelLine = 4;
    break;
  case 24: // Lệch phải mức 3
    levelLine = 3;
    break;
  case 8: // Lệch phải mức 2
    levelLine = 2;
    break;
  case 12: // Lệch phải mức 1
    levelLine = 1;
    break;
  case 4: // Giữa line
    levelLine = 0;
    break;
  case 6: // Lệch trái mức 1
    levelLine = -1;
    break;
  case 2: // Lệch trái mức 2
    levelLine = -2;
    break;
  case 3: // Lệch trái mức 3
    levelLine = -3;
    break;
  case 1: // Lệch trái mức 4
    levelLine = -4;
    break;
  default:
    break;
  }

  // Cập nhập hướng lệch của xe
  if (levelLine >= 0)
    car.direction = true;
  else
    car.direction = false;

  // Cho biết mức độ lệch hiện tại của xe
  return levelLine;
}

/* ------------------------------------------------------------------------- */

// Tính toán PID
float calculate_pid(int8_t errorNow)
{
  // Tính toán các giá trị PID
  car.P = errorNow;
  car.I = car.I + errorNow;
  car.D = errorNow - car.errorPrev;

  // Cập nhập giá trị "error" hiện tại
  car.errorPrev = errorNow;

  // Tính toán giá trị PID
  return (car.Kp * car.P) + (car.Ki * car.I) + (car.Kd * car.D);
}

/* ------------------------------------------------------------------------- */

void motor_control(float PID_value)
{
  // Thêm PID vào điều chỉnh tốc độ riêng cho mỗi bánh xe
  byte speedMotorRight = SPEED_DEFAULT - PID_value;
  byte speedMotorLeft = SPEED_DEFAULT + PID_value;

  // Đảm bảo tốc độ Motor ko vượt quá giá trị xung PWM tối đa
  constrain(speedMotorLeft, 0, 255);
  constrain(speedMotorRight, 0, 255);

  // Đẩy robot về phía trước với tốc độ tùy chỉnh hai bên
  go_straight_custom(speedMotorLeft, speedMotorRight);
}

/* ------------------------------------------------------------------------- */
/*                                CONTROL CAR                                */
/* ------------------------------------------------------------------------- */

/**
 * Yêu cầu trong lắp đặt:
 * |
 * Cổng [OUT1:OUT2] điều khiển Motor bên Phải
 * Cổng [OUT3:OUT4] điều khiển Motor bên Trái
 * |
 * Khi OUT1(-) và OUT2(+), "Motor Phải" quay bánh đẩy xe đi tới
 * Khi OUT3(+) và OUT4(-), "Motor Trái" quay bánh đẩy xe đi tới
 */

// Điều khiển Motor bên Phải quay tới
void motorRight_RotateForward(byte PWM)
{
  digitalWrite(PIN_IN1, LOW);
  analogWrite(PIN_IN2, PWM);
}

// Điều khiển Motor bên Trái quay tới
void motorLeft_RotateForward(byte PWM)
{
  analogWrite(PIN_IN3, PWM);
  digitalWrite(PIN_IN4, LOW);
}

// Điều khiển Motor bên Phải quay lùi
void motorRight_RotateReverse(byte PWM)
{
  digitalWrite(PIN_IN1, HIGH);
  analogWrite(PIN_IN2, 255 - PWM);
}

// Điều khiển Motor bên Trái quay lùi
void motorLeft_RotateReverse(byte PWM)
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

/* ------------------------- Điều khiển xe dừng lại ------------------------ */
void stop()
{
  motorRight_Stop();
  motorLeft_Stop();
}

/* ------------------------- Điều khiển xe đi thẳng ------------------------ */
void go_straight(byte speed)
{
  motorRight_RotateForward(speed);
  motorLeft_RotateForward(speed);
}

void go_straight_custom(byte speedLeft, byte speedRight)
{
  motorRight_RotateForward(speedRight);
  motorLeft_RotateForward(speedLeft);
}

/* -------------------- Điều khiển xe đi thẳng lệch trái ------------------- */
void go_left(byte speed)
{
  // Bánh phải đi tới nhanh
  motorRight_RotateForward(speed);

  // Bánh trái đi tới chậm
  motorLeft_RotateForward(speed * SPEED);
}

/* -------------------- Điều khiển xe đi thẳng lệch phải ------------------- */
void go_right(byte speed)
{
  // Bánh phải đi tới chậm
  motorRight_RotateForward(speed * SPEED);

  // Bánh trái đi tới nhanh
  motorLeft_RotateForward(speed);
}

/* ------------------------ Điều khiển xe quay trái ------------------------ */
void turn_left(byte speed)
{
  // Bánh phải đi tới
  motorRight_RotateForward(speed);

  // Bánh trái đi lùi
  motorLeft_RotateReverse(speed);
}

/* ------------------------ Điều khiển xe quay phải ------------------------ */
void turn_right(byte speed)
{
  // Bánh phải đi lùi
  motorRight_RotateReverse(speed);

  // Bánh trái đi tới
  motorLeft_RotateForward(speed);
}

/* ------------------------------------------------------------------------- */
/*                                RUN ONE TIME                               */
/* ------------------------------------------------------------------------- */

void setup()
{
  Serial.begin(115200);

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
  motor_control(calculate_pid(read_TCRT5000()));
}