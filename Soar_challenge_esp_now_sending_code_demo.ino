#include <esp_now.h>
#include <WiFi.h>
#include <esp_err.h>

#define RIGHT_BUTTON    1
#define UP_BUTTON       2
#define LEFT_BUTTON     4
#define DOWN_BUTTON     5
#define A_BUTTON        21
#define B_BUTTON        47

uint8_t receiver_address[] = {0xd0, 0xcf, 0x13, 0x1d, 0x7f, 0x64};  // replace with your MAC address found with the other code
esp_now_peer_info_t receiver_peer;

typedef struct {
  bool forward        : 1 ;
  bool backward       : 1 ;
  bool left           : 1 ;
  bool right          : 1 ;
  bool a_button       : 1 ;
  bool b_button       : 1 ;
} message_data;

message_data myData;
message_data oldData;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Hello World");

  pinMode(RIGHT_BUTTON, INPUT_PULLUP);    // this means the pin when
  pinMode(LEFT_BUTTON,  INPUT_PULLUP);    // not pressed will be
  pinMode(UP_BUTTON,    INPUT_PULLUP);    // read as HIGH or 1
  pinMode(DOWN_BUTTON,  INPUT_PULLUP);
  pinMode(A_BUTTON,     INPUT_PULLUP);    // to use properly other
  pinMode(B_BUTTON,     INPUT_PULLUP);    // button side to gnd

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
  receiver_peer.channel = 0;
  receiver_peer.encrypt = false;
  // receiver_peer.lmk;   // used for encryption
  receiver_peer.ifidx = WIFI_IF_STA;

  Serial.println("registering esp_now peer");
  Serial.flush();
  esp_now_add_peer(&receiver_peer);

  Serial.println("Finished initialising ESP-now");
  Serial.flush();
}

void loop() {
  // put your main code here, to run repeatedly:
  myData.a_button = !digitalRead(A_BUTTON);       // putting a ! (not operator)
  myData.b_button = !digitalRead(B_BUTTON);       // to flip the logic of the
  myData.backward = !digitalRead(DOWN_BUTTON);    // button as the default 
  myData.forward =  !digitalRead(UP_BUTTON);      // value of the pins are
  myData.left =     !digitalRead(LEFT_BUTTON);    // HIGH due to the internal
  myData.right =    !digitalRead(RIGHT_BUTTON);   // pullup set in setup()

  if(
  myData.a_button != oldData.a_button ||
  myData.b_button != oldData.b_button ||
  myData.backward != oldData.backward ||
  myData.forward  != oldData.forward ||
  myData.left     != oldData.left ||
  myData.right    != oldData.right
  ){
    memcpy(&oldData, &myData, sizeof(myData));
    esp_err_t error = esp_now_send(receiver_address, (uint8_t*)&myData, sizeof(myData));    // works for every primitive datatype
    Serial.println(esp_err_to_name(error));
    if(error == ESP_OK){
      Serial.printf("sent: %d bytes\n", sizeof(myData));
    }
    if(error == ESP_ERR_ESPNOW_NO_MEM){
      Serial.printf("error occured, increasing delay time next time");
    }
  }


  delay(50);   // decrease for a higher polling rate, recommended to stay above 30ms due to ESP_NOW limitation
  
}


