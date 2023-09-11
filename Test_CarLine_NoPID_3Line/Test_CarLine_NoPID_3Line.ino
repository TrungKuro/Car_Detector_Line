/**
 * Không có dùng PID
 *
 * Nguồn pin có ảnh hướng đến độ chính xác của xe
 */

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
 */
#define PIN_OUT1 A1 //! A1
#define PIN_OUT2 A2 //! A2
#define PIN_OUT3 A3 //! A3

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

// Các mức tốc độ
#define FAST PER_60
#define NORMAL PER_50
#define SLOW PER_40

// TimeOut, đơn vị (ms)
#define TIME_OUT 500

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

/**
 * Cho biết hướng lệch hiện tại của xe
 * Phải mức 2 : +2
 * Phải mức 1 : +1
 * Giữa       :  0
 * Trái mức 1 : -1
 * Trái mức 2 : -2
 */
int8_t direction;

/**
 * Cho biết đang trong line hay ngoài line
 * Trong line : TRUE
 * Ngoài line : FALSE
 */
bool inLine;

// Lưu thời điểm phát hiện xe ngoài line
unsigned long capPoint;

/* ------------------------------------------------------------------------- */
/*                                  FUNCTION                                 */
/* ------------------------------------------------------------------------- */

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

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

/* ------------------------- Điều khiển xe dừng lại ------------------------ */
void stop()
{
  motorRight_Stop();
  motorLeft_Stop();
}

/* ------------------------- Điều khiển xe đi thẳng ------------------------ */
void go_straight_custom(byte speedLeft, byte speedRight)
{
  motorRight_RotateForward(speedRight);
  motorLeft_RotateForward(speedLeft);
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
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void motor_control()
{
  /**
   * Trái ------------ Phải
   * |                    |
   * | OUT3 | OUT2 | OUT1 |
   *
   * Khoảng cách phát hiện Line ĐEN (~1cm)
   * Có Line - HIGH - Bit 1
   * Ko Line - LOW  - Bit 0
   */
  raw.dataLine.line1 = digitalRead(PIN_OUT1);
  raw.dataLine.line2 = digitalRead(PIN_OUT2);
  raw.dataLine.line3 = digitalRead(PIN_OUT3);

  // Điều khiển xe đi
  // Serial.println(raw.stateLine);
  switch (raw.stateLine)
  {
    /* --------------------- Hết line, hoặc ngoài line --------------------- */

  case 0: // 000
    /**
     * Bắt đầu đếm thời gian
     * Kể từ lúc xe vừa lệch ra ngoài line
     */
    if (inLine)
    {
      capPoint = millis();
      inLine = false;
    }
    /**
     * Nếu quá thời gian giới hạn
     * Sẽ cho xe dừng lại
     */
    if (millis() - capPoint >= TIME_OUT)
    {
      stop();
    }
    /**
     * Nếu vẫn còn trong thời gian cho phép
     * Điều khiển xe cố gắng tìm lại line
     */
    else
    {
      switch (direction)
      {
      case 2: // Lệch phải nhiều ... xoay trái vừa
        turn_left(NORMAL);
        break;
      case 1: // Lệch phải ít ... xoay trái chậm
        turn_left(SLOW);
        break;
      case 0: // Giữa ... cứ đi thẳng tiếp
        go_straight_custom(SLOW, SLOW);
        break;
      case -1: // Lệch trái ít ... xoay phải chậm
        turn_right(SLOW);
        break;
      case -2: // Lệch trái nhiều ... xoay phải vừa
        turn_right(NORMAL);
        break;
      }
    }
    break;

    /* ----------------------------- Lệch trái ----------------------------- */

    /* Lệch trái nhiều ... xoay phải */
  case 1: // 001
    direction = -2;
    inLine = true;
    go_straight_custom(FAST, SLOW);
    break;

    /* Lệch trái ít ... rẽ phải ... bánh trái quay nhanh hơn */
  case 3: // 011
    direction = -1;
    inLine = true;
    go_straight_custom(NORMAL, SLOW);
    break;

    /* ----------------------------- Giữa line ----------------------------- */

    /* Giữa line ... đường line nhỏ ... chạy chậm */
  case 2: // 010
    direction = 0;
    inLine = true;
    go_straight_custom(SLOW, SLOW);
    break;

    /* Giữa line ... đường line lớn ... chạy vừa */
  case 7: // 111
    direction = 0;
    inLine = true;
    go_straight_custom(NORMAL, NORMAL);
    break;

    /* ----------------------------- Lệch phải ----------------------------- */

    /* Lệch phải ít ... rẽ trái ... bánh phải quay nhanh hơn */
  case 6: // 110
    direction = 1;
    inLine = true;
    go_straight_custom(SLOW, NORMAL);
    break;

    /* Lệch phải nhiều ... xoay trái */
  case 4: // 100
    direction = 2;
    inLine = true;
    go_straight_custom(SLOW, FAST);
    break;

    /* ------------------------ Các trường hợp khác ------------------------ */

  case 5: // 101
    direction = 0;
    inLine = false;
    stop();
    break;

    /* --------------------------------------------------------------------- */
  }
}

/* ------------------------------------------------------------------------- */
/*                                RUN ONE TIME                               */
/* ------------------------------------------------------------------------- */

void setup()
{
  // Serial.begin(115200);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);

  delay(TIME_OUT);
}

/* ------------------------------------------------------------------------- */
/*                                    MAIN                                   */
/* ------------------------------------------------------------------------- */

void loop()
{
  motor_control();
}