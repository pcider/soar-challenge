#include <esp_now.h>
#include <WiFi.h>
#include <esp_err.h>
//#include <Bluepad32.h>x  aaaa

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
  int vel_x; // -1023 - 1023
  int vel_y; // -1023 - 1023
  bool button;
} message_data;

message_data curData;
message_data prevData;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Hello World");
//  pinMode(RIGHT_BUTTON, INPUT_PULLUP);    // this means the pin when
//  // pinMode(LEFT_BUTTON,  INPUT_PULLUP);    // not pressed will be
//  pinMode(UP_BUTTON,    INPUT_PULLUP);    // read as HIGH or 1
//  // pinMode(DOWN_BUTTON,  INPUT_PULLUP);
//  pinMode(A_BUTTON,     INPUT_PULLUP);    // to use properly other
//  pinMode(B_BUTTON,     INPUT_PULLUP);    // button side to gnd

  pinMode(JOYSTICK_LEFT_VRX, INPUT);
  pinMode(JOYSTICK_LEFT_VRY, INPUT);
  pinMode(JOYSTICK_RIGHT_VRX,  INPUT);
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
  Serial.printf("l: (%d %d), r: (%d %d)\n",
    analogRead(JOYSTICK_LEFT_VRX),
    analogRead(JOYSTICK_LEFT_VRY),
    analogRead(JOYSTICK_RIGHT_VRX),
    analogRead(JOYSTICK_RIGHT_VRY)
  );
  curData.vel_y = analogRead(JOYSTICK_LEFT_VRY); // 0 - 4096
  curData.vel_x = analogRead(JOYSTICK_RIGHT_VRX); // 0 - 4096
  curData.vel_y -= 2048;
  curData.vel_x -= 2048;
  curData.vel_y /= 2;
  curData.vel_x /= 2;

  if (
    abs(curData.vel_x - prevData.vel_x) > 5 ||
    abs(curData.vel_y - prevData.vel_y) > 5 ||
    curData.button != prevData.button
  ) {
    Serial.printf("x: %d, y: %d, button: %d\n",  
                  curData.vel_x,
                  curData.vel_y,
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
