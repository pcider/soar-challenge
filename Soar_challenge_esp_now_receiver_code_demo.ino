#include <esp_now.h>
#include <WiFi.h>

uint8_t myData[250];

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, len);
  Serial.print("Length received:");
  Serial.println(len);
  Serial.println("Bytes received: ");
  for(int i = 0; i < len; i++){
    Serial.printf("%d: %02x | ",i, myData[i]);
  }
  Serial.println();
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
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}
