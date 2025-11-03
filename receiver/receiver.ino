#include <esp_now.h>
//#include <esp_mac.h>
#include <WiFi.h>
#include <Bluepad32.h>



// FRONT  
#define RIGHT_FORWARD  9 // right wheel forward (IN_1)
#define RIGHT_BACKWARD 10 // right wheel reverse (IN_2)
#define LEFT_FORWARD   11 // left wheel forward (IN_3)
#define LEFT_BACKWARD  12 // left wheel reverse (IN_4)
#define GATE_OPEN_R    46 // right wheel forward (IN_1)
#define GATE_CLOSED_R  8 // right wheel reverse (IN_2),
#define GATE_OPEN_L    6 // left wheel forward (IN_3)
#define GATE_CLOSED_L  5 // left wheel reverse (IN_4)

// chas team
//#define RIGHT_BACKWARD 17 // right wheel reverse (IN_2)

// my team
//#define RIGHT_FORWARD  46 // right wheel forward (IN_1)
//#define RIGHT_BACKWARD 8 // right wheel reverse (IN_2)
//#define LEFT_FORWARD   6 // left wheel forward (IN_3)
//#define LEFT_BACKWARD  5 // left wheel reverse (IN_4)
//#define GATE_OPEN_R    9 // right wheel forward (IN_1)
//#define GATE_CLOSED_R  10 // right wheel reverse (IN_2)
//#define GATE_OPEN_L    11 // left wheel forward (IN_3)
//#define GATE_CLOSED_L  12 // left wheel reverse (IN_4)

// ethan's team
//#define SWAP_LEFT_RIGHT
//#define RIGHT_FORWARD  46 // right wheel forward (IN_1)
//#define RIGHT_BACKWARD 8 // right wheel reverse (IN_2)
//#define LEFT_FORWARD   6 // left wheel forward (IN_3)
//#define LEFT_BACKWARD  5 // left wheel reverse (IN_4)
//#define GATE_OPEN_R    10 // right wheel forward (IN_1)
//#define GATE_CLOSED_R  9 // right wheel reverse (IN_2),
//#define GATE_OPEN_L    12 // left wheel forward (IN_3)
//#define GATE_CLOSED_L  11 // left wheel reverse (IN_4)

#define OPEN_GATE_SPEED 450 // 900
#define CLOSED_GATE_SPEED 400 // 800

//#define OPEN_GATE_SPEED 900
//#define CLOSED_GATE_SPEED 800

//#define NO_AUTONOMOUS

//#define USE_ULTRASOUND

//#define USE_ESP_NOW

// swap left & right movement
//#define SWAP_LEFT_RIGHT

// swap forward & backward movement
//#define SWAP_FORWARD_BACKWARD
// ethan's robot

#define TRIG_PIN 17
#define ECHO_PIN 16

#ifdef USE_ESP_NOW
typedef struct {
  int vel_x; // 0 - 4095
  int vel_y; // 0 - 4095
  int button; // button bitmap
} message_data;

message_data curData;
#endif

#define STEADY_TOL 50
#define STEADY_X 0
#define STEADY_Y 0    
#define MAX_X (511)
#define MAX_Y (511)
#define MIN_X (-512)
#define MIN_Y (-512)


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
  BP32.enableNewBluetoothConnections(false);
}

void onDisconnectedController(ControllerPtr ctl) {
  Serial.printf("controller disconnected idx: %d\n", ctl->index());
//  BP32.forgetBluetoothKeys();
  BP32.enableNewBluetoothConnections(true);
  if (!btController) return;
  if (ctl->index() == btController->index()) btController = NULL;
}

#ifdef USE_ESP_NOW
void OnDataRecv(uint8_t * mac, message_data *data, uint8_t len) {
  memcpy(&curData, data, len);
  Serial.printf("x: %d, y: %d, button: 0x%x\n",
                curData.vel_x,
                curData.vel_y,
                curData.button
               );
}
#endif

void open_gate() {
  analogWrite(GATE_OPEN_R, OPEN_GATE_SPEED);
  analogWrite(GATE_OPEN_L, OPEN_GATE_SPEED);
  analogWrite(GATE_CLOSED_R, 0);
  analogWrite(GATE_CLOSED_L, 0);
}

void close_gate() {
  analogWrite(GATE_OPEN_R, 0);
  analogWrite(GATE_OPEN_L, 0) ;
  analogWrite(GATE_CLOSED_R, CLOSED_GATE_SPEED);
  analogWrite(GATE_CLOSED_L, CLOSED_GATE_SPEED);
}

void inactive_gate() {
  analogWrite(GATE_OPEN_R, 0);
  analogWrite(GATE_OPEN_L, 0) ;
  analogWrite(GATE_CLOSED_R, 0);
  analogWrite(GATE_CLOSED_L, 0);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Hello World");

#ifdef USE_ESP_NOW
  uint8_t mac_addr[6];
  esp_err_t ret = esp_base_mac_addr_get(mac_addr);

  if(ret == ESP_OK){
    Serial.println("Mac address: ");
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  }else{
    Serial.println("Failed to read Mac address");
  }
  
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

#else
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);
  Serial.println("using bluetooth");
#endif

  // pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  
  pinMode(GATE_OPEN_R, OUTPUT);
  pinMode(GATE_CLOSED_R, OUTPUT);
  pinMode(GATE_OPEN_L, OUTPUT);
  pinMode(GATE_CLOSED_L, OUTPUT);
  analogWriteResolution(10);
//  analogWriteFrequency(1000);
}


float distance, prev_distance, curr_time;
int forward_spd = 512;
int turn_spd = 512;
float rev_ms = 5000;
int curr_angle = 0;
bool at_home = true;

void loop() {
  static int vel_x = 0;
  static int vel_y = 0;
  static double scale = 1;
  static bool prevPressed = false;
  bool buttonPressed = false;

  static bool auto_mode = false;
  static bool driver_mode = true;
  static long initial_time = 0;
  static long noMoreAutonomous = false; 
#ifdef USE_ESP_NOW
  vel_x = curData.vel_x;
  vel_y = curData.vel_y;
  buttonPressed = curData.button & 1;
  if (curData.button & 2) open_gate();
  if (curData.button & 4) close_gate();
  if (!(curData.button & 2) && !(curData.button & 4)) inactive_gate();
#else
  bool dataUpdated = BP32.update();
  if (dataUpdated && btController && btController->isConnected() && btController->hasData()) {
    vel_x = btController->axisRX();
    vel_y = -btController->axisY(); // y axis is flipped for left joystick
    buttonPressed = btController->y();
    if (driver_mode) {
      if (btController->x()) open_gate();
      if (btController->b()) close_gate();
      if (!btController->x() && !btController->b()) inactive_gate();
    }
    if (!noMoreAutonomous && btController->a()) {
        noMoreAutonomous = true;
        driver_mode = false;
        auto_mode = true;
        initial_time = 0;
    }
  }
#endif

  if (!prevPressed && buttonPressed) {
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
    Serial.printf("changed scale to %f\n", scale);
  } else if (!buttonPressed) {
    prevPressed = false;
  }

  int x = vel_x * 2;
  int y = vel_y * 2;
  int left_spd = 0;
  int right_spd = 0;

  if (abs(y) > STEADY_TOL) {
#ifdef SWAP_FORWARD_BACKWARD
    left_spd = right_spd = y;
#else
    left_spd = right_spd = -y;
#endif
  }
  if (abs(x) > STEADY_TOL) {
#ifdef SWAP_LEFT_RIGHT
    right_spd -= x;
    left_spd += x;
#else
    right_spd += x;
    left_spd -= x;
#endif
  }

  int scaled_left_spd = left_spd * scale; // max(min(left_spd, 1024), -1024) * scale;
  int scaled_right_spd = right_spd  * scale; // max(min(left_spd, 1024), -1024) * scale;

#define T1 1000 // open gate
#define T2 (T1 + 1500) // move forward
#define T3 (T2 + 1000) // close gate
#define T4 (T3 + 3500) // move backward
#define T5 (T4 + 1000) // stop
#define T6 (T5 + 1000)

void backward();
void forward();
void stop();

#ifndef NO_AUTONOMOUS
  if (auto_mode) {
    if (initial_time == 0) {
      initial_time = millis();
//      close_gate();
    }

    curr_time = millis();
    if (curr_time - initial_time > T1 && curr_time - initial_time < T2) {
//      inactive_gate();
      backward();
    } 
    if (curr_time - initial_time > T2 && curr_time - initial_time < T3) {
//      open_gate();
      stop();
    }
    if (curr_time - initial_time > T3 && curr_time - initial_time < T4) {
//      inactive_gate();
//      forward();
    }
//    if (curr_time - initial_time > T4 && curr_time - initial_time < T5) {
//      inactive_gate();
//      turnAngle(15);
//    }
    if (curr_time - initial_time > T5) {
        stop();
        driver_mode = true;
        auto_mode = false;
        return;
    }
}
#endif
    
#ifdef USE_ULTRASOUND
    distance = getDistance();
    Serial.printf("Distance: %.2fcm\n", distance);

    if (at_home) {
        approachObject(1000);
        at_home = false;
    }
    else if (distance > 25 && distance <= 50) {
        if (prev_distance > 50) {
            at_home = true;
        }
        else {
            lookAround(45);
            approachObject(750);
        }
    }
    else if (distance - prev_distance >= 0.5) {
        findObject(30);
    }
    else if (distance >= 5) {
        approachObject(500);
    }
    else {
        if (isCollectable(250)) {
            eatObject();
            moveObjectHome();
        }
        else {
            avoidObject(500);
        }
    }

    prev_distance = distance;
  }
#endif
  if (driver_mode) {
    
    if (scaled_left_spd != 0 || scaled_right_spd != 0)
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

  }
  // put your main code here, to run repeatedly:
  delay(50);
}

// shiqi's code
//

float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    float duration = pulseIn(ECHO_PIN, HIGH);
    distance = (duration * .0343) / 2;
    return distance;
}


void forward() {
    analogWrite(LEFT_FORWARD, forward_spd);
    analogWrite(LEFT_BACKWARD, 0);
    analogWrite(RIGHT_FORWARD, forward_spd);
    analogWrite(RIGHT_BACKWARD, 0);
}

void backward() {
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(LEFT_BACKWARD, forward_spd);
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, forward_spd);
}

void turnLeft() {
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(LEFT_BACKWARD, turn_spd);
    // analogWrite(RIGHT_FORWARD, turn_spd);
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, 0);
}

void turnRight() {
    // analogWrite(LEFT_FORWARD, turn_spd);
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(LEFT_BACKWARD, 0);
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, turn_spd);
}

void stop() {
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(LEFT_BACKWARD, 0);
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, 0);
}



void turnAngle(float angle) {
    float turn_ms = abs(angle) * (rev_ms/360);
    // Serial.printf("Turn milliseconds: %.2fms\n", turn_ms);

    if (angle > 0 && angle <= 180) {
        turnRight();
    }
    else if (angle < 0 && angle >= -180) {
        turnLeft();
    }
    delay(turn_ms);
    // stop();
}


float getTurnAngle(float turn_ms) {
    return turn_ms * (360/rev_ms);
}

void updateCurrAngle(float angle) {
    curr_angle = curr_angle + angle;
    if (curr_angle >= 360) {
        curr_angle = curr_angle - 360;
    }
    Serial.printf("Current Angle: %.2f degrees\n", angle);
}


void lookAround(int angle) {
    Serial.printf("Looking around\n");

    turnAngle(angle);
    float right_distance = getDistance();
    turnAngle(-angle * 2);
    float left_distance = getDistance();
    
    if (right_distance < left_distance && right_distance < distance) {
        turnAngle(angle * 2);
        updateCurrAngle(angle);
    }
    else if (distance < left_distance && distance < right_distance) {
        turnAngle(angle);
    }
    else {
        updateCurrAngle(360 - angle);
    }
}


void findObject(int step_angle) {
    float start_turn = millis();
    float total_turn_ms = start_turn;
    
    while (millis() - start_turn < rev_ms) {
        turnAngle(step_angle);

        if (getDistance() - prev_distance < 0.5) {
            stop();
            total_turn_ms = millis();
            break;
        }
    }

    float angle = getTurnAngle(total_turn_ms - start_turn);
    Serial.printf("Turned %.2f degrees\n", angle);
    updateCurrAngle(angle);
}

void approachObject(float move_ms) {
    forward();
    delay(move_ms);
    // stop();
    Serial.printf("Moved forward\n");
}

void avoidObject(float move_ms) {
    backward();
    delay(move_ms);
    stop();
    Serial.printf("Moved backward\n");
}



bool isCollectable(int push_ms) {
    approachObject(push_ms);
    stop();
    if (getDistance() > distance) {
        Serial.printf("Detected collectable\n");
        return true;
    }
    return false;
}

void eatObject() {
    distance = getDistance();
    lookAround(15);
    approachObject(250);
    // CLOSE ARM
}

void moveObjectHome() {
    turnAngle(180 - curr_angle);
    approachObject(1000);
    // OPEN ARM
    avoidObject(250);
    turnAngle(180);
    curr_angle = 0;
    at_home = true;
}

//void moveRandom() {
//    approachObject(random(250, 1500));
//    turnAngle(random(-180, 180));
//}

// bool test = true;
// if (test) {
//         Serial.printf("test");
//         turnRight();
//         delay(1500);
//         stop();
//         test = false;
//     }
