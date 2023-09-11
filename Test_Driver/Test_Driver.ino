/* ------------------------------------------------------------------------- */
/*                                   DEFINE                                  */
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
  Serial.println("Chạy thẳng");
  go_straight(255);
  delay(5000);

  Serial.println("Xoay trái");
  turn_left(255);
  delay(5000);

  Serial.println("Xoay phải");
  turn_right(255);
  delay(5000);

  Serial.println("Dừng xe");
  stop();
  delay(5000);
}