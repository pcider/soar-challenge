#include <esp_now.h>
#include <WiFi.h>

#define RIGHT_ENABLE 9
#define LEFT_ENABLE 4
#define RIGHT_FORWARD 8 // right wheel forward
#define RIGHT_BACKWARD 7 // right wheel reverse
#define LEFT_FORWARD 6 // left wheel forward
#define LEFT_BACKWARD 5 // left wheel reverse

typedef struct {
  int vrx; // 0 - 4095
  int vry; // 0 - 4095
  bool button;
} message_data;

#define STEADY_TOL 50
#define STEADY_X 1940
#define STEADY_Y 1940
#define MAX_X (4096 - STEADY_X)
#define MAX_Y (4096 - STEADY_Y)
#define MIN_X (- STEADY_X)
#define MIN_Y (-STEADY_Y)

message_data curData;

void OnDataRecv(uint8_t * mac, message_data *data, uint8_t len) {
  memcpy(&curData, data, len);
//  Serial.print("Length received:");
//  Serial.println(len);
//  Serial.printf("x: %d, y: %d, button: %d\n",
//                curData.vrx,
//                curData.vry,
//                curData.button
//               );
//  Serial.println();
}

void disable_motors() {
  digitalWrite(RIGHT_ENABLE, LOW);
  digitalWrite(LEFT_ENABLE, LOW);
}

void enable_motors() {
  digitalWrite(RIGHT_ENABLE, HIGH);
  digitalWrite(LEFT_ENABLE, HIGH);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Hello World");

  // setup WiFi
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // register receiving callback
  esp_now_register_recv_cb(
    esp_now_recv_cb_t(OnDataRecv)
  );

  pinMode(RIGHT_ENABLE, OUTPUT);
  pinMode(LEFT_ENABLE, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  disable_motors();
  enable_motors();
  curData.vrx = STEADY_X;
  curData.vry = STEADY_Y;
}

void loop() {
  int x = curData.vrx - STEADY_X;
  int y = curData.vry - STEADY_Y;
  int left_spd = 0;
  int right_spd = 0;
  if (abs(y) > STEADY_TOL) {
    // move only if joystick deviates 50 units
    if (y > 0) {
      int pwm_y = y * 256 / MAX_Y;
      left_spd = right_spd = pwm_y;
    } else {
      int pwm_y = y * 256 / MIN_Y;
      left_spd = right_spd = -pwm_y;
    }
  }

  if (abs(x) > STEADY_TOL) {
    if (x > 0) {
      int pwm_x = x * 256 / MAX_X;
      right_spd -= pwm_x * 2; // TODO: scale this properly
    } else {
      int pwm_x = y * 256 / MIN_X;
      left_spd += pwm_x * 2; // TODO: scale this properly
    }
  }
  
  Serial.printf("left: %d, right: %d, \n", left_spd, right_spd);
  if (left_spd == 0) {
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(LEFT_BACKWARD, 0);
  } else if (left_spd > 0) {
    analogWrite(LEFT_FORWARD, left_spd);
    analogWrite(LEFT_BACKWARD, 0);
  } else {
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(LEFT_BACKWARD, -left_spd);
  }

  if (right_spd == 0) {
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, 0);
  } else if (right_spd > 0) {
    analogWrite(RIGHT_FORWARD, right_spd);
    analogWrite(RIGHT_BACKWARD, 0);
  } else {
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, -right_spd);
  }

  // put your main code here, to run repeatedly:
  delay(100);
}
