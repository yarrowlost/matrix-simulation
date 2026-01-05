

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <MPU6500_WE.h>
#include <Wire.h>
#include <cmath>
#include <AceRoutine.h>
#include <gameEngine.h>

#define PIN 16
#define MPU6500_ADDR 0x68



using namespace ace_routine;
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



class MainCoroutine : public Coroutine {

  int runCoroutine() override {
    COROUTINE_LOOP() {
      xyzFloat gValue = myMPU6500.getGValues();

      float angle = std::atan2(gValue.y, gValue.z) * 180 / 3.14;

      matrix.fillScreen(0);

      if (makeFrame(angle, matrix)) {
        matrix.show();
      } else {
        static int i;
        static int j;
        for (i = 0; i < 16; ++i) {
          for (j = 0; j < 16; ++j) {
            matrix.drawPixel(j, i, matrix.Color(150, 0, 0));
          }
          COROUTINE_DELAY(180);
          matrix.show();
        }
        matrix.setTextColor(matrix.Color(200, 200, 200));
        matrix.setCursor(0, 0);
        matrix.print(score);
        matrix.show();

        while (digitalRead(18)) {
          COROUTINE_DELAY(180);
        }

        init();
      }


      COROUTINE_DELAY(30);
    }
  }
};

MainCoroutine mc;


void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);
  delay(1000);
  if (!myMPU6500.init()) {
    Serial.println("MPU6500 does not respond");
  } else {
    Serial.println("MPU6500 is connected");
  }

  Serial.println("Position you MPU6500 flat and don't move it - calibrating...");
  delay(1000);
  myMPU6500.autoOffsets();
  Serial.println("Done!");

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

  matrix.setBrightness(20);

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
