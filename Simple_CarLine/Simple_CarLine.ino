/* ------------------------------------------------------------------------- */
/*                                  LIBRARY                                  */
/* ------------------------------------------------------------------------- */

#include "SimpleMotor.h"

/* ------------------------------------------------------------------------- */
/*                              DEFINE (DRIVER)                              */
/* ------------------------------------------------------------------------- */

/**
 * PIN_IN2 kết nối Driver L298
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
/*                              DEFINE (SENSOR)                              */
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
/*                                DEFINE (CAR)                               */
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
 * Khoảng thời gian "timeout"
 *
 * Là khoảng thời gian cho phép xe vẫn chạy tiếp dù đã ra ngoài line
 * Đây là khoảng thời gian để xe tìm lại đường line
 *
 * Hết thời gian mà xe vẫn ko thấy line, sẽ cho xe dừng hẳn
 */
#define TIME_OUT 750 //! Đơn vị (ms)

/* ------------------------------------------------------------------------- */
/*                                  VARIABLE                                 */
/* ------------------------------------------------------------------------- */

// Khởi tạo kết nối các chân với Driver
SimpleMotor car(PIN_IN1, PIN_IN2, PIN_IN3, PIN_IN4);

// Lưu giá trị đọc từ cảm biến dò line
bool lineRight;  // Cho OUT1
bool lineCenter; // Cho OUT2
bool lineLeft;   // Cho OUT3

/**
 * Cho biết hướng lệch hiện tại của xe
 * Lệch Phải mức 2 : +2
 * Lệch Phải mức 1 : +1
 * Ngay giữa line  :  0
 * Lệch Trái mức 1 : -1
 * Lệch Trái mức 2 : -2
 */
int direction;

/**
 * Cho biết xe đang trong line hay ngoài line
 * Trong line : TRUE
 * Ngoài line : FALSE
 */
bool inLine;

// Lưu thời điểm phát hiện mỗi khi xe vừa ra ngoài line
unsigned long capturePoint;

/* ------------------------------------------------------------------------- */
/*                                  FUNCTION                                 */
/* ------------------------------------------------------------------------- */

void read_sensor()
{
  lineRight = digitalRead(PIN_OUT1);
  lineCenter = digitalRead(PIN_OUT2);
  lineLeft = digitalRead(PIN_OUT3);
}

void level_line()
{
  if (!lineLeft && !lineCenter && lineRight) // 001
  {
    /**
     * Lệch trái nhiều ... rẽ phải ... bánh trái quay nhanh hơn nữa
     */
    direction = -2;
    inLine = true;
    car.go_custom(FAST, SLOW);
  }
  else if (!lineLeft && lineCenter && lineRight) // 011
  {
    /**
     * Lệch trái ít ... rẽ phải ... bánh trái quay nhanh hơn
     */
    direction = -1;
    inLine = true;
    car.go_custom(NORMAL, SLOW);
  }
  /* ----------------------------------------------------------------------- */
  else if ((!lineLeft && lineCenter && !lineRight) || (lineLeft && lineCenter && lineRight)) // 010 hoặc 111
  {
    /**
     * Giữa line ... đường line nhỏ ... chạy chậm
     */
    direction = 0;
    inLine = true;
    car.go_forward(SLOW);
  }
  /* ----------------------------------------------------------------------- */
  else if (lineLeft && lineCenter && !lineRight) // 110
  {
    /**
     * Lệch phải ít ... rẽ trái ... bánh phải quay nhanh hơn
     */
    direction = 1;
    inLine = true;
    car.go_custom(SLOW, NORMAL);
  }
  else if (lineLeft && !lineCenter && !lineRight) // 100
  {
    /**
     * Lệch phải nhiều ... rẽ trái ... bánh phải quay nhanh hơn nữa
     */
    direction = 2;
    inLine = true;
    car.go_custom(SLOW, FAST);
  }
  else if (!lineLeft && !lineCenter && !lineRight) // 000
  {
    /**
     * Hết line, hoặc ngoài line
     * Bắt đầu đếm thời gian "timeout"
     * Kể từ lúc xe vừa lệch ra ngoài line
     */
    if (inLine)
    {
      capturePoint = millis();
      inLine = false;
    }

    /**
     * Nếu quá thời gian giới hạn
     * Sẽ cho xe dừng lại
     */
    if (millis() - capturePoint >= TIME_OUT)
    {
      direction = 0;
      inLine = false;
      car.stop();
    }
    /**
     * Nếu vẫn còn trong thời gian cho phép
     * Điều khiển xe cố gắng tìm lại line
     * Dựa theo giá trị lần cuối lệch line
     */
    else
    {
      switch (direction)
      {
      case 2: // Lệch phải nhiều ... xoay trái vừa
        car.rotate_left(NORMAL);
        break;
      case 1: // Lệch phải ít ... xoay trái chậm
        car.rotate_left(SLOW);
        break;
      case 0: // Giữa ... cứ đi thẳng tiếp
        car.go_forward(SLOW);
        break;
      case -1: // Lệch trái ít ... xoay phải chậm
        car.rotate_right(SLOW);
        break;
      case -2: // Lệch trái nhiều ... xoay phải vừa
        car.rotate_right(NORMAL);
        break;
      }
    }
  }
}

/* ------------------------------------------------------------------------- */
/*                                RUN ONE TIME                               */
/* ------------------------------------------------------------------------- */

void setup()
{
  // Thiết lập các chân đọc cảm biến
  pinMode(PIN_OUT1, INPUT);
  pinMode(PIN_OUT2, INPUT);
  pinMode(PIN_OUT3, INPUT);

  // Tránh xe chạy liền ngay khi vừa khởi động
  delay(TIME_OUT);
}

/* ------------------------------------------------------------------------- */
/*                                    MAIN                                   */
/* ------------------------------------------------------------------------- */

void loop()
{
  // Đọc giá trị từ cảm biến
  read_sensor();

  // Từ giá trị cảm biến dò line
  // Tính toán mức độ lệch so với line
  // Và điều khiển xe để dò lại line
  level_line();
}