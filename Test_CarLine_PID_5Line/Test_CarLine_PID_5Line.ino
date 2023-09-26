/**
 * Phiên bản này, xe chạy liên tục ko có dừng
 * - Sử dụng 5 mắt dò line
 * - Dùng thuật toán PID
 *
 * Lưu ý, có 2 cách sử dụng giá trị PID
 * Bạn cần chọn chế độ giữa "DEFAULT" hoặc "ZERO"
 */

/* ------------------------------------------------------------------------- */
/*                                 DEFINE PIN                                */
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
 * OUT4     - A4 (Digital)
 * OUT5     - A5 (Digital)
 */
#define PIN_OUT1 A1 //! A1
#define PIN_OUT2 A2 //! A2
#define PIN_OUT3 A3 //! A3
#define PIN_OUT4 A4 //! A4
#define PIN_OUT5 A5 //! A5

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
/*                            DEFINE CONFIG (PID)                            */
/* ------------------------------------------------------------------------- */

/**
 * Hệ số của các khâu PID:
 *
 * Khâu [P] thể hiện độ lớn thay đổi tốc độ
 * |        giá trị càng lớn sẽ càng tạo nhiều dao động
 * |
 * Khâu [D] để giảm đi tốc độ thay đổi đó
 * |        giá trị càng lớn sẽ giảm bớt dao động lại
 * |        nhưng nếu lớn quá sẽ dẫn đến trạng thái lệch tĩnh
 * |        tức xe vẫn còn lệch line, nhưng motor ko nhận lệnh đủ để thoát khỏi đó
 * |
 * Khâu [I] để cộng dồn các mức lệch
 * |        giúp bù lực cho tình huống bị đứng yên ở trên
 * |        nhưng hệ số nên nhỏ thôi, vì tần suất tính khâu PID tương đối nhanh
 */
#define KP 50.0    //!
#define KI 0.00001 //!
#define KD 35.0    //!

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
 * Đặt giá trị tốc độ xe mặc định ban đầu
 * Khi xe vừa mới khởi động, khoảng [0 : 255]
 */
#define SPEED_DEFAULT PER_50 //!

/**
 * Đặt ngưỡng giới hạn trên và dưới cho tốc độ
 * Giá trị dương (+), motor quay thuận
 * Giá trị âm (-), motor quay nghịch
 */
#define UP PER_70    //!
#define DOWN -PER_70 //!

/**
 * Đặt ngưỡng thấp nhất cho phép cấp vào motor
 * Tránh tình trạng các motor ko đủ nguồn để hoạt động
 *
 * Motor sẽ hoạt động trong dãi phạm vi
 * - quay thuận ... [MIN_UP : UP]
 * - quay nghịch ... [MIN_DOWN : DOWN]
 */
#define MIN_UP PER_30    //!
#define MIN_DOWN -PER_30 //!

/* ------------------------------------------------------------------------- */
/*                            DEFINE CONFIG (MODE)                           */
/* ------------------------------------------------------------------------- */

/**
 * Chọn cách sử dụng giá trị PID
 * Mode ZERO    → Sử dụng giá trị "cũ" trước đó của mỗi bánh xe làm gốc
 * Mode DEFAULT → Sử dụng giá trị "SPEED_DEFAULT" làm gốc
 *
 * Comment để chọn chế độ DEFAULT
 * Uncomment để chọn chế độ ZERO
 */
#define ZERO //!

/* ------------------------------------------------------------------------- */
/*                                  VARIABLE                                 */
/* ------------------------------------------------------------------------- */

/**
 * Tổng kích thước dữ liệu này là 1 Byte
 *
 * Bit : [7] - [6] - [5] - [4] - [3] - [2] - [1] - [0]
 * Line:  x     x     x    OUT5  OUT4  OUT3  OUT2  OUT1
 */
struct DataLine
{
  // Mép Phải
  bool line1 : 1; // OUT1 - Bit [0]
  bool line2 : 1; // OUT2 - Bit [1]
  bool line3 : 1; // OUT3 - Bit [2]
  bool line4 : 1; // OUT4 - Bit [3]
  bool line5 : 1; // OUT5 - Bit [4]
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

struct CarLine
{
  /**
   * Cho biết hướng lệch hiện tại của xe
   * Lệch phải → (+) : TRUE
   * Lệch trái → (-) : FALSE
   */
  bool direction;

  int P = 0, I = 0, D = 0;         // Giá trị hiện tại của từng khâu PID
  float Kp = KP, Ki = KI, Kd = KD; // Hệ số của các khâu PID
  float PID_value;                 // Giá trị PID sau mỗi lần tính toán

  int errorNow;  // Lưu giá trị "error" hiện tại
  int errorPrev; // Lưu giá trị "error" trước đó

  int speedRightNow = SPEED_DEFAULT; // Tốc độ hiện tại của bánh Phải
  int speedLeftNow = SPEED_DEFAULT;  // Tốc độ hiện tại của bánh Trái
} car;

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

/* ------------------------- Điều khiển xe dừng lại ------------------------ */
void stop()
{
  // Bánh phải dừng lại
  motorRight_Stop();
  // Bánh trái dừng lại
  motorLeft_Stop();
}

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

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void motor_control()
{
  /**
   * Trái -------------------------- Phải
   * |                                  |
   * | OUT5 | OUT4 | OUT3 | OUT2 | OUT1 |
   *
   * Khoảng cách phát hiện Line ĐEN (~1cm)
   * Có Line - HIGH - Bit 1
   * Ko Line - LOW  - Bit 0
   */
  raw.dataLine.line1 = digitalRead(PIN_OUT1);
  raw.dataLine.line2 = digitalRead(PIN_OUT2);
  raw.dataLine.line3 = digitalRead(PIN_OUT3);
  raw.dataLine.line4 = digitalRead(PIN_OUT4);
  raw.dataLine.line5 = digitalRead(PIN_OUT5);

  /* ----------------------------------------------------------------------- */
  /* ----------------------------------------------------------------------- */
  /* ----------------------------------------------------------------------- */

  /**
   * Chuyển giá trị DEC từ "stateLine"
   * Sang giá trị Level cho "errorNow"
   */
  switch (raw.stateLine)
  {
    /* ------------------ [000] - Hết line hoặc ngoài line ----------------- */
  case 0:
    if (car.direction)
      car.errorNow = 10; // Đang lệch phải ngoài line
    else
      car.errorNow = -10; // Đang lệch trái ngoài line
    break;
    /* -------------------------- Góc 45º bên phải ------------------------- */
  case 5: // [00101]
    car.direction = false;
    car.errorNow = -100;
    break;
    /* -------------------------- Góc 90º bên phải ------------------------- */
  case 7: // [00111]
    car.direction = false;
    car.errorNow = -50;
    break;
    /* ----------------------------- Lệch trái ----------------------------- */
  case 1: // [00001] - Lệch trái mức 4
    car.direction = false;
    car.errorNow = -4;
    break;
  case 3: // [00011] - Lệch trái mức 3
    car.direction = false;
    car.errorNow = -3;
    break;
  case 2: // [00010] - Lệch trái mức 2
    car.direction = false;
    car.errorNow = -2;
    break;
  case 6: // [00110] - Lệch trái mức 1
    car.direction = false;
    car.errorNow = -1;
    break;
    /* ----------------------------- Giữa line ----------------------------- */
  case 4: // [00100] - Giữa line
    car.errorNow = 0;
    break;
    /* ----------------------------- Lệch phải ----------------------------- */
  case 12: // [01100] - Lệch phải mức 1
    car.direction = true;
    car.errorNow = 1;
    break;
  case 8: // [01000] - Lệch phải mức 2
    car.direction = true;
    car.errorNow = 2;
    break;
  case 24: // [11000] - Lệch phải mức 3
    car.direction = true;
    car.errorNow = 3;
    break;
  case 16: // [10000] - Lệch phải mức 4
    car.direction = true;
    car.errorNow = 4;
    break;
    /* -------------------------- Góc 90º bên trái ------------------------- */
  case 28: // [11100]
    car.direction = true;
    car.errorNow = 50;
    break;
    /* -------------------------- Góc 45º bên trái ------------------------- */
  case 20: // [10100]
    car.direction = true;
    car.errorNow = 100;
    break;
    /* ------------------------ Các trường hợp khác ------------------------ */
  default:
    break;
  }

  /* ----------------------------------------------------------------------- */
  /* ----------------------------------------------------------------------- */
  /* ----------------------------------------------------------------------- */

  // Tính toán các giá trị PID
  car.P = car.errorNow;
  car.I = car.I + car.errorNow;
  car.D = car.errorNow - car.errorPrev;

  // Cập nhập giá trị "error" hiện tại
  car.errorPrev = car.errorNow;

  // Tính toán giá trị PID
  car.PID_value = (car.Kp * car.P) + (car.Ki * car.I) + (car.Kd * car.D);

  /* ----------------------------------------------------------------------- */

// Thêm PID vào điều chỉnh tốc độ riêng cho mỗi bánh xe
#ifdef ZERO
  car.speedRightNow += car.PID_value;
  car.speedLeftNow -= car.PID_value;
#else
  car.speedRightNow = SPEED_DEFAULT + car.PID_value;
  car.speedLeftNow = SPEED_DEFAULT - car.PID_value;
#endif

  /* ----------------------------------------------------------------------- */

  // Đảm bảo tốc độ Motor bánh phải nằm trong các khoảng quy định
  if (car.speedRightNow >= 0)
    car.speedRightNow = constrain(car.speedRightNow, MIN_UP, UP);
  else
    car.speedRightNow = constrain(car.speedRightNow, DOWN, MIN_DOWN);
  // Đảm bảo tốc độ Motor bánh trái nằm trong các khoảng quy định
  if (car.speedLeftNow >= 0)
    car.speedLeftNow = constrain(car.speedLeftNow, MIN_UP, UP);
  else
    car.speedLeftNow = constrain(car.speedLeftNow, DOWN, MIN_DOWN);

  /* ----------------------------------------------------------------------- */

  // Đẩy robot về phía trước với tốc độ tùy chỉnh hai bên
  go_custom(car.speedLeftNow, car.speedRightNow);
}

/* ------------------------------------------------------------------------- */
/*                                RUN ONE TIME                               */
/* ------------------------------------------------------------------------- */

void setup()
{
  // Thiết đặt các chân điều khiển Driver
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
  /**
   * Trình tự các bước điều khiển xe:
   * |
   * Bước 1: đọc giá trị các mắt dò line (5 cảm biến)
   * Bước 2: tính ra "mức độ lệch line" hiện tại từ giá trị các cảm biến phản hồi về
   * Bước 3: tính giá trị từng khâu P, khâu I, khâu D
   * Bước 4: tính giá trị tổng của cả khâu PID
   * Bước 5: bù trừ tốc độ mỗi bánh xe dựa trên giá trị PID này
   * Bước 6: điều khiển các bánh xe với tốc độ đã điều chỉnh
   */
  motor_control();
}