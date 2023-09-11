/**
 * Kích thước tổng bộ nhớ chỉ 1 Byte
 * Kết hợp STRUCT vs BIT-FIELDS
 * Cho phép truy cập vào từng Bit riêng lẽ trong 1 biến
 */
struct Data
{
  bool line1 : 1; // Bit [0] (LSB)
  bool line2 : 1; // Bit [1]
  bool line3 : 1; // Bit [2]
  bool line4 : 1; // Bit [3]
  bool line5 : 1; // Bit [4]
                  // Bit [5]
                  // Bit [6]
                  // Bit [7] (MSB)
};

// Dữ liệu nạp vào "data"
// Dữ liệu đầu ra "state"
/**
 * STRUCT "data" sẽ nhận dữ liệu đầu vào
 * Từng giá trị cảm biến phản hồi sẽ lưu trên mỗi Bit
 *
 * BYTE "state" sẽ tổng hợp dữ liệu đầu ra
 * Từ giá trị từng Bit thành giá trị DEC
 */
union Map
{
  Data data;
  byte state;
};

/* ------------------------------------------------------------------------- */

void setup()
{
  Serial.begin(115200);

  /* ----------------------------------------------------------------------- */

  /**
   * Bit : [7] - [6] - [5] - [4] - [3] - [2] - [1] - [0]
   * Line:  x     x     x    OUT1  OUT2  OUT3  OUT4  OUT5
   *
   * 10  =  0     0     0     0     1     0     1     0
   *                         Line1 Line2 Line3 Line4 Line5
   */
  Map raw;
  raw.state = 10; // 0000.1010

  Serial.print("Line1: ");
  Serial.println(raw.data.line1);

  Serial.print("Line2: ");
  Serial.println(raw.data.line2);

  Serial.print("Line3: ");
  Serial.println(raw.data.line3);

  Serial.print("Line4: ");
  Serial.println(raw.data.line4);

  Serial.print("Line5: ");
  Serial.println(raw.data.line5);

  /* ----------------------------------------------------------------------- */

  Serial.print("Size \"Data\": ");
  Serial.println(sizeof(Data));

  Serial.print("Size \"Map\": ");
  Serial.println(sizeof(Map));

  /* ----------------------------------------------------------------------- */

  Serial.print("Size \"int8_t\": ");
  Serial.println(sizeof(int8_t));

  Serial.println("Range:");
  int8_t test = 0;
  for (int i = 0; i < 256; i++)
  {
    test++;
    Serial.print("[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(test);
  }
}

void loop()
{
}
