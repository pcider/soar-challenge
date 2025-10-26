// #include <esp_wifi.h>
// #include <esp_now.h>
#include <esp_mac.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Hello World!");

  uint8_t mac_addr[6];
  esp_err_t ret = esp_base_mac_addr_get(mac_addr);

  if(ret == ESP_OK){
    Serial.println("Mac address: ");
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  }else{
    Serial.println("Failed to read Mac address");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}
