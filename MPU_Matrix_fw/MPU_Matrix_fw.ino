/***************************************************************************
* Example sketch for the MPU6500_WE library
*
* This sketch shows how to get acceleration, gyroscocope and temperature 
* data from the MPU6500. In essence, the difference to the MPU9250 is the
* missing magnetometer. The shall only show how to "translate" all other 
* MPU9250 example sketches for use of the MPU6500
* 
* For further information visit my blog:
*
* https://wolles-elektronikkiste.de/mpu9250-9-achsen-sensormodul-teil-1  (German)
* https://wolles-elektronikkiste.de/en/mpu9250-9-axis-sensor-module-part-1  (English)
* 
***************************************************************************/
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <MPU6500_WE.h>
#include <Wire.h>
#include <cmath>
#define PIN 16
#define MPU6500_ADDR 0x68

/* There are several ways to create your MPU6500 object:
 * MPU6500_WE myMPU6500 = MPU6500_WE()              -> uses Wire / I2C Address = 0x68
 * MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR)  -> uses Wire / MPU6500_ADDR
 * MPU6500_WE myMPU6500 = MPU6500_WE(&wire2)        -> uses the TwoWire object wire2 / MPU6500_ADDR
 * MPU6500_WE myMPU6500 = MPU6500_WE(&wire2, MPU6500_ADDR) -> all together
 * Successfully tested with two I2C busses on an ESP32
 */
MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, PIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

int16_t x = matrix.width();
uint8_t color_index = 0;
char text[] = "WATER";
int scroll_limit = sizeof(text) * 6;


const int size = 16;
const int cellWidth = 32;
struct PV {
  PV() = default;
  PV(float x, float y)
    : x(x), y(y) {}
  float x;
  float y;
};

uint16_t frame[16][16] = { 0 };

struct Cell {

  int x;
  int y;
  PV center;
};

Cell sorted[256];


void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);
  delay(1000);
  if (!myMPU6500.init()) {
    Serial.println("MPU6500 does not respond");
  } else {
    Serial.println("MPU6500 is connected");
  }
  for (int i = 0; i < 128; ++i) {

    // const auto calcColor = [](int i) {
    //   if (i / 16 < 2) {
    //     return matrix.Color(i * 2, (127 - i) * 2, 0);
    //   } else if (i / 16 < 4) {
    //     return matrix.Color(0, i * 2, (127 - i) * 2);
    //   } else {
    //     return matrix.Color((127 - i) * 2, 0, i * 2);
    //   }
    // };

    frame[i % 16][i / 16] = color_wheel(i);
  }

  /* The slope of the curve of acceleration vs measured values fits quite well to the theoretical 
   * values, e.g. 16384 units/g in the +/- 2g range. But the starting point, if you position the 
   * MPU6500 flat, is not necessarily 0g/0g/1g for x/y/z. The autoOffset function measures offset 
   * values. It assumes your MPU6500 is positioned flat with its x,y-plane. The more you deviate 
   * from this, the less accurate will be your results.
   * The function also measures the offset of the gyroscope data. The gyroscope offset does not   
   * depend on the positioning.
   * This function needs to be called at the beginning since it can overwrite your settings!
   */
  Serial.println("Position you MPU6500 flat and don't move it - calibrating...");
  delay(1000);
  myMPU6500.autoOffsets();
  Serial.println("Done!");

  /*  This is a more accurate method for calibration. You have to determine the minimum and maximum 
   *  raw acceleration values of the axes determined in the range +/- 2 g. 
   *  You call the function as follows: setAccOffsets(xMin,xMax,yMin,yMax,zMin,zMax);
   *  Use either autoOffset or setAccOffsets, not both.
   */
  //myMPU6500.setAccOffsets(-14240.0, 18220.0, -17280.0, 15590.0, -20930.0, 12080.0);

  /*  The gyroscope data is not zero, even if you don't move the MPU6500. 
   *  To start at zero, you can apply offset values. These are the gyroscope raw values you obtain
   *  using the +/- 250 degrees/s range. 
   *  Use either autoOffset or setGyrOffsets, not both.
   */
  //myMPU6500.setGyrOffsets(45.0, 145.0, -105.0);

  /*  You can enable or disable the digital low pass filter (DLPF). If you disable the DLPF, you 
   *  need to select the bandwdith, which can be either 8800 or 3600 Hz. 8800 Hz has a shorter delay,
   *  but higher noise level. If DLPF is disabled, the output rate is 32 kHz.
   *  MPU6500_BW_WO_DLPF_3600 
   *  MPU6500_BW_WO_DLPF_8800
   */
  myMPU6500.enableGyrDLPF();
  //myMPU6500.disableGyrDLPF(MPU6500_BW_WO_DLPF_8800); // bandwdith without DLPF

  /*  Digital Low Pass Filter for the gyroscope must be enabled to choose the level. 
   *  MPU6500_DPLF_0, MPU6500_DPLF_2, ...... MPU6500_DPLF_7 
   *  
   *  DLPF    Bandwidth [Hz]   Delay [ms]   Output Rate [kHz]
   *    0         250            0.97             8
   *    1         184            2.9              1
   *    2          92            3.9              1
   *    3          41            5.9              1
   *    4          20            9.9              1
   *    5          10           17.85             1
   *    6           5           33.48             1
   *    7        3600            0.17             8
   *    
   *    You achieve lowest noise using level 6  
   */
  myMPU6500.setGyrDLPF(MPU6500_DLPF_6);

  /*  Sample rate divider divides the output rate of the gyroscope and accelerometer.
   *  Sample rate = Internal sample rate / (1 + divider) 
   *  It can only be applied if the corresponding DLPF is enabled and 0<DLPF<7!
   *  Divider is a number 0...255
   */
  myMPU6500.setSampleRateDivider(5);

  /*  MPU6500_GYRO_RANGE_250       250 degrees per second (default)
   *  MPU6500_GYRO_RANGE_500       500 degrees per second
   *  MPU6500_GYRO_RANGE_1000     1000 degrees per second
   *  MPU6500_GYRO_RANGE_2000     2000 degrees per second
   */
  myMPU6500.setGyrRange(MPU6500_GYRO_RANGE_250);

  /*  MPU6500_ACC_RANGE_2G      2 g   (default)
   *  MPU6500_ACC_RANGE_4G      4 g
   *  MPU6500_ACC_RANGE_8G      8 g   
   *  MPU6500_ACC_RANGE_16G    16 g
   */
  myMPU6500.setAccRange(MPU6500_ACC_RANGE_2G);

  /*  Enable/disable the digital low pass filter for the accelerometer 
   *  If disabled the bandwidth is 1.13 kHz, delay is 0.75 ms, output rate is 4 kHz
   */
  myMPU6500.enableAccDLPF(true);

  /*  Digital low pass filter (DLPF) for the accelerometer, if enabled 
   *  MPU6500_DPLF_0, MPU6500_DPLF_2, ...... MPU6500_DPLF_7 
   *   DLPF     Bandwidth [Hz]      Delay [ms]    Output rate [kHz]
   *     0           460               1.94           1
   *     1           184               5.80           1
   *     2            92               7.80           1
   *     3            41              11.80           1
   *     4            20              19.80           1
   *     5            10              35.70           1
   *     6             5              66.96           1
   *     7           460               1.94           1
   */
  myMPU6500.setAccDLPF(MPU6500_DLPF_6);

  /* You can enable or disable the axes for gyroscope and/or accelerometer measurements.
   * By default all axes are enabled. Parameters are:  
   * MPU6500_ENABLE_XYZ  //all axes are enabled (default)
   * MPU6500_ENABLE_XY0  // X, Y enabled, Z disabled
   * MPU6500_ENABLE_X0Z   
   * MPU6500_ENABLE_X00
   * MPU6500_ENABLE_0YZ
   * MPU6500_ENABLE_0Y0
   * MPU6500_ENABLE_00Z
   * MPU6500_ENABLE_000  // all axes disabled
   */
  //myMPU6500.enableAccAxes(MPU6500_ENABLE_XYZ);
  //myMPU6500.enableGyrAxes(MPU6500_ENABLE_XYZ);
  delay(200);

  matrix.begin();

  //   matrix.setTextWrap(false);
  matrix.setBrightness(20);
  //   matrix.setTextColor(matrix.Color(255, 0, 0));
  //  matrix.drawPixel(8, 8, matrix.Color(255, 0, 0));
}

void loop() {

  xyzFloat gValue = myMPU6500.getGValues();
  xyzFloat gyr = myMPU6500.getGyrValues();
  float temp = myMPU6500.getTemperature();
  float resultantG = myMPU6500.getResultantG(gValue);
  float angle = std::atan2(gValue.y, gValue.z);

  //Serial.println("Acceleration in g (x,y,z):");
  Serial.print(gValue.x);
  Serial.print("\t");
  Serial.print(gValue.y);
  Serial.print("\t");
  Serial.print(gValue.z);
  Serial.print("\t");
  Serial.println(angle * 180 / 3.14);
  // Serial.print("Resultant g: ");
  // Serial.println(resultantG);

  // Serial.println("Gyroscope data in degrees/s: ");
  // Serial.print(gyr.x);
  // Serial.print("   ");
  // Serial.print(gyr.y);
  // Serial.print("   ");
  // Serial.println(gyr.z);

  // Serial.print("Temperature in °C: ");
  // Serial.println(temp);

  //Serial.println("********************************************");
  matrix.fillScreen(0);

  makeFrame(angle);

  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      matrix.drawPixel(x, y, frame[x][y]);
    }
  }
  // if(--x < -scroll_limit) {
  //   x = matrix.width();
  //   color_index = 0; // reset the rainbow color index
  // }
  // matrix.setTextColor(color_wheel(color_index));
  // color_index += (256 / scroll_limit) > 1 ? (256 / scroll_limit) : 1;
  matrix.show();    // const auto calcColor = [](int i) {
    //   if (i / 16 < 2) {
    //     return matrix.Color(i * 2, (127 - i) * 2, 0);
    //   } else if (i / 16 < 4) {
    //     return matrix.Color(0, i * 2, (127 - i) * 2);
    //   } else {
    //     return matrix.Color((127 - i) * 2, 0, i * 2);
    //   }
    // };

  delay(80);
}

uint16_t color_wheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) {
    //  return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
    return matrix.Color((uint16_t)(255 - pos * 3), 0, (pos * 3));
  } else if (pos < 170) {
    pos -= 85;
    //  return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
    return matrix.Color(0, (uint32_t)(pos * 3), (255 - pos * 3));
  } else {
    pos -= 170;
    //  return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
    return matrix.Color((uint16_t)(pos * 3), (uint32_t)(255 - pos * 3), 0);
  }
}

PV getRotatedCellCenter(int row, int col,
                        float cellWidth, float cellHeight,
                        int totalRows, int totalCols,
                        float angle) {
  // Calculate the center of the whole matrix
  float matrixCenterX = (totalCols * cellWidth) / 2.0f;
  float matrixCenterY = (totalRows * cellHeight) / 2.0f;

  // Calculate the center of the specified cell
  float cellCenterX = col * cellWidth + cellWidth / 2.0f;
  float cellCenterY = row * cellHeight + cellHeight / 2.0f;

  // Convert to relative coordinates
  float relX = cellCenterX - matrixCenterX;
  float relY = cellCenterY - matrixCenterY;

  // Apply rotation
  float cosA = std::cos(angle);
  float sinA = std::sin(angle);
  float rotatedX = relX * cosA - relY * sinA;
  float rotatedY = relX * sinA + relY * cosA;

  // Convert back to absolute coordinates
  rotatedX += matrixCenterX;
  rotatedY += matrixCenterY;

  return PV(rotatedX, rotatedY);
}
void sortCells(float angle) {
  for (int x = 0; x < size; ++x) {
    for (int y = 0; y < size; ++y) {
      PV cellCenter = getRotatedCellCenter(y, x, cellWidth, cellWidth, size, size, angle);
      sorted[x + y * size] = Cell(x, y, cellCenter);
    }
  }

  // Sort array using std::sort on pointer range
  std::sort(sorted, sorted + size * size, [](const Cell& a, const Cell& b) {
    return (a.center.y < b.center.y);
  });
}


void makeFrame(float angle) {
  float myAngle = angle;
  sortCells(myAngle);

  for (int i = size * size - 1; i >= 0; --i) {
    Cell& c = sorted[i];
    if (frame[c.x][c.y] != 0) {
      for (int j = size * size - 1; j >= i; --j) {
        Cell& cellToCheck = sorted[j];
        if (frame[cellToCheck.x][cellToCheck.y] == 0
            && abs(cellToCheck.center.x - c.center.x) < cellWidth * 2.3) {
          frame[cellToCheck.x][cellToCheck.y] = frame[c.x][c.y];
          frame[c.x][c.y] = 0;
          //sorted[i] = null;
          break;
        }
      }
    }
  }
}