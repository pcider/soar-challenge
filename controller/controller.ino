#include <esp_now.h>
#include <WiFi.h>
#include <esp_err.h>
#include <Bluepad32.h>

#define RIGHT_BUTTON    1
#define UP_BUTTON       2
#define JOYSTICK_VRX     5
#define JOYSTICK_VRY     4
#define JOYSTICK_BUTTON     6
#define A_BUTTON        21
#define B_BUTTON        47

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


void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
        "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
        ctl->index(),        // Controller Index
        ctl->dpad(),         // D-pad
        ctl->buttons(),      // bitmask of pressed buttons
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
        ctl->miscButtons(),  // bitmask of pressed "misc" buttons
        ctl->gyroX(),        // Gyro X
        ctl->gyroY(),        // Gyro Y
        ctl->gyroZ(),        // Gyro Z
        ctl->accelX(),       // Accelerometer X
        ctl->accelY(),       // Accelerometer Y
        ctl->accelZ()        // Accelerometer Z
    );
}

//void processControllers() {
//    for (auto myController : myControllers) {
//        if (myController && myController->isConnected() && myController->hasData()) {
//            if (myController->isGamepad()) {
//                processGamepad(myController);
//            } else if (myController->isMouse()) {
//                processMouse(myController);
//            } else if (myController->isKeyboard()) {
//                processKeyboard(myController);
//            } else if (myController->isBalanceBoard()) {
//                processBalanceBoard(myController);
//            } else {
//                Serial.println("Unsupported controller");
//            }
//        }
//    }
//}

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

  
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But it might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();

  // Enables mouse / touchpad support for gamepads that support them.
  // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
  // - First one: the gamepad
  // - Second one, which is a "virtual device", is a mouse.
  // By default, it is disabled.
  BP32.enableVirtualDevice(false);

  delay(100);
//  pinMode(RIGHT_BUTTON, INPUT_PULLUP);    // this means the pin when
//  // pinMode(LEFT_BUTTON,  INPUT_PULLUP);    // not pressed will be
//  pinMode(UP_BUTTON,    INPUT_PULLUP);    // read as HIGH or 1
//  // pinMode(DOWN_BUTTON,  INPUT_PULLUP);
//  pinMode(A_BUTTON,     INPUT_PULLUP);    // to use properly other
//  pinMode(B_BUTTON,     INPUT_PULLUP);    // button side to gnd
//
//  pinMode(JOYSTICK_VRX, INPUT);
//  pinMode(JOYSTICK_VRY, INPUT);
//  pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);

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
