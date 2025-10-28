#include <esp_now.h>
#include <WiFi.h>
#include <esp_err.h>
#include <Bluepad32.h>

//#define RIGHT_BUTTON    1
//#define UP_BUTTON       2
#define JOYSTICK_LEFT_VRX     5
#define JOYSTICK_LEFT_VRY     4
#define JOYSTICK_RIGHT_VRX    2
#define JOYSTICK_RIGHT_VRY    1
#define BUTTON_1     18
//#define A_BUTTON        21
//#define B_BUTTON        47

// 80:b5:4e:e3:e1:2c
uint8_t receiver_address[] = {0x80, 0xb5, 0x4e, 0xe3, 0xe1, 0x2c};  // replace with your MAC address found with the other code
esp_now_peer_info_t receiver_peer;

typedef struct {
  int vrx; // 0 - 4095
  int vry; // 0 - 4095
  bool button;
} message_data;

message_data curData;
message_data prevData;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Hello World");


  delay(100);
//  pinMode(RIGHT_BUTTON, INPUT_PULLUP);    // this means the pin when
//  // pinMode(LEFT_BUTTON,  INPUT_PULLUP);    // not pressed will be
//  pinMode(UP_BUTTON,    INPUT_PULLUP);    // read as HIGH or 1
//  // pinMode(DOWN_BUTTON,  INPUT_PULLUP);
//  pinMode(A_BUTTON,     INPUT_PULLUP);    // to use properly other
//  pinMode(B_BUTTON,     INPUT_PULLUP);    // button side to gnd
//
  pinMode(JOYSTICK_LEFT_VRX, INPUT);
  pinMode(JOYSTICK_LEFT_VRY, INPUT);
  pinMode(JOYSTICK_RIGHT_VRX, INPUT);
  pinMode(JOYSTICK_RIGHT_VRY, INPUT);
  pinMode(BUTTON_1, INPUT);

  WiFi.mode(WIFI_MODE_STA);   // Arduino's wifi library, internally sets up the espressif wifi stack
  // code crashes without the WiFi.mode(WIFI_MODE_STA); line.

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Serial.println("creating esp_now peer");
  Serial.flush();
  memcpy(receiver_peer.peer_addr, receiver_address, 6);
  receiver_peer.channel = 1;
  receiver_peer.encrypt = false;
  // receiver_peer.lmk;   // used for encryption
  receiver_peer.ifidx = WIFI_IF_STA;

  Serial.println("registering esp_now peer");
  Serial.flush();
  esp_now_add_peer(&receiver_peer);

  Serial.println("Finished initialising ESP-now");
    
  Serial.flush();
}

int abs(int i) {
  return i > 0 ? i : -i;
}

void loop() {
  bool dataUpdated = BP32.update();
  if (dataUpdated && btController && btController->isConnected() && btController->hasData()) {
    curData.vrx = btController->axisRX(); // analogRead(JOYSTICK_VRX);
    curData.vry = -btController->axisY(); // y axis is flipped for left joystick
    // dumpGamepad(btController);
  }

   delay(50);
  if (
    abs(curData.vrx - prevData.vrx) > 5 ||
    abs(curData.vry - prevData.vry) > 5 ||
    curData.button != prevData.button
  ) {
    Serial.printf("x: %d, y: %d, button: %d\n",
                  curData.vrx,
                  curData.vry,
                  curData.button
                 );

    memcpy(&prevData, &curData, sizeof(curData));
    esp_err_t error = esp_now_send(receiver_address, (uint8_t*)&curData, sizeof(curData));    // works for every primitive datatype
    Serial.println(esp_err_to_name(error));
    if (error == ESP_OK) {
      Serial.printf("sent: %d bytes\n", sizeof(curData));
    }
    if (error == ESP_ERR_ESPNOW_NO_MEM) {
      Serial.printf("error occured, increasing delay time next time");
    }
  }
  delay(100);   // decrease for a higher polling rate, recommended to stay above 30ms due to ESP_NOW limitation
}
