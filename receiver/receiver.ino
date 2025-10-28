#include <esp_now.h>
//#include <esp_mac.h>
#include <WiFi.h>
#include <Bluepad32.h>

// FRONT / 
#define RIGHT_FORWARD 9 // right wheel forward (IN_1)
#define RIGHT_BACKWARD 10 // right wheel reverse (IN_2)
#define LEFT_FORWARD 11 // left wheel forward (IN_3)
#define LEFT_BACKWARD 12 // left wheel reverse (IN_4)

//#define RIGHT_FORWARD 46 // right wheel forward (IN_1)
//#define RIGHT_BACKWARD 8 // right wheel reverse (IN_2)
//#define LEFT_FORWARD 6 // left wheel forward (IN_3)
//#define LEFT_BACKWARD 5 // left wheel reverse (IN_4)

typedef struct {
  int vel_x; // 0 - 4095
  int vel_y; // 0 - 4095
  int buttons; // button bitmap
} message_data;

#define STEADY_TOL 50
#define STEADY_X 0
#define STEADY_Y 0
#define MAX_X (511)
#define MAX_Y (511)
#define MIN_X (-512)
#define MIN_Y (-512)

message_data curData;

ControllerPtr btController;

void onConnectedController(ControllerPtr ctl) {
  Serial.printf("CALLBACK: Controller is connected idx: %d\n");
  ControllerProperties properties = ctl->getProperties();
  Serial.printf("Found controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                 properties.product_id);
  if (!ctl->isGamepad()) {
    return;
  }
  Serial.printf("Using controller idx: %d\n", ctl->index());
  btController = ctl;
}

void onDisconnectedController(ControllerPtr ctl) {
  if (!btController) return;
  if (ctl->index() == btController->index()) btController = NULL;
}

void OnDataRecv(uint8_t * mac, message_data *data, uint8_t len) {
  memcpy(&curData, data, len);
  Serial.printf("x: %d, y: %d, buttons: 0x%x\n",
                curData.vel_x,
                curData.vel_y,
                curData.buttons
               );
} 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Hello World");

  uint8_t mac_addr[6];
  esp_err_t ret = esp_base_mac_addr_get(mac_addr);

  if(ret == ESP_OK){
    Serial.println("Mac address: ");
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  }else{
    Serial.println("Failed to read Mac address");
  }

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);
  
  // setup WiFi
//  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
//  if (esp_now_init() != ESP_OK) {
//    Serial.println("Error initializing ESP-NOW");
//    return;
//  }

  // register receiving callback
//  esp_now_register_recv_cb(
//    esp_now_recv_cb_t(OnDataRecv)
//  );

  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);

  analogWriteResolution(10);
//  analogWriteFrequency(100);
  
//  curData.vrx = STEADY_X;
//  curData.vry = STEADY_Y;
}

void loop() {
  static int vel_x = 0;
  static int vel_y = 0;
  static double scale = 1;
  static bool prevPressed = false;
  
  bool dataUpdated = BP32.update();
  if (dataUpdated && btController && btController->isConnected() && btController->hasData()) {
    vel_x = btController->axisRX();
    vel_y = -btController->axisY(); // y axis is flipped for left joystick
    
    if (!prevPressed && btController->y()) {
      prevPressed = true;
      if (scale == 1.0) {
        scale = 0.5;
      } else if (scale == 0.5) {
        scale = 0.25;
      } else if (scale == 0.25) {
        scale = 0.125;
      } else if (scale == 0.125) {
        scale = 1.0;
      }
    } else if (!curData.buttons) {
      prevPressed = false;
    }
  }

  int x = vel_x * 2;
  int y = vel_y * 2;
  int left_spd = 0;
  int right_spd = 0;

  if (abs(y) > STEADY_TOL) {
    left_spd = right_spd = y;
  }
  if (abs(x) > STEADY_TOL) {
    right_spd -= x;
    left_spd += x;
  }

  int scaled_left_spd = left_spd * scale; // max(min(left_spd, 1024), -1024) * scale;
  int scaled_right_spd = right_spd  * scale; // max(min(left_spd, 1024), -1024) * scale;
  
  Serial.printf("l: %d (%d), r: %d (%d), s: %f \n", scaled_left_spd, left_spd, scaled_right_spd, right_spd, scale);
  if (scaled_left_spd == 0) {
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(LEFT_BACKWARD, 0);
  } else if (scaled_left_spd > 0) {
    analogWrite(LEFT_FORWARD, scaled_left_spd);
    analogWrite(LEFT_BACKWARD, 0);
  } else {
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(LEFT_BACKWARD, -scaled_left_spd);
  }

  if (scaled_right_spd == 0) {
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, 0);
  } else if (scaled_right_spd > 0) {
    analogWrite(RIGHT_FORWARD, scaled_right_spd);
    analogWrite(RIGHT_BACKWARD, 0);
  } else {
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, -scaled_right_spd);
  }

  // put your main code here, to run repeatedly:
  delay(50);
}
