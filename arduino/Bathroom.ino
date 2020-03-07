#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
// https://github.com/sui77/rc-switch
#include <RCSwitch.h>
#include <EEPROM.h>
// https://github.com/rweather/arduinolibs/tree/master/libraries/Crypto
#include <Crypto.h>
#include <SHA256.h>

#define CLOUD_FUNCTION_URL "http://region-project.cloudfunctions.net/"
#define KEY "very secret"
#define WIFI_NETWORK "network"
#define WIFI_PASSWORD "password"

#define BUFSIZE 256
#define HASH_SIZE 32
// we only persist the sequence number every N requests to preserve the EEPROM
#define WRITE_INTERVAL 1024

RCSwitch mySwitch;
SHA256 sha256;

unsigned long lastReceivedValue = 0;
unsigned long sequence;

void setup() {
  Serial.begin(9600);
  Serial.println("HELLO");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

  EEPROM.begin(sizeof(unsigned long));
  sequence = 0;
  for (int i = 0; i < sizeof(unsigned long); i++) {
    sequence += EEPROM.read(i) << i * 8;
  }
  sequence += WRITE_INTERVAL;
  for (int i = 0; i < sizeof(unsigned long); i++) {
    EEPROM.write(i, (sequence >> i * 8) & 0xff);
  }
  EEPROM.commit();

  mySwitch.enableReceive(4);
}

void loop() {
  if (mySwitch.available()) {
    unsigned long receivedValue = mySwitch.getReceivedValue();
    Serial.println(receivedValue);
    if (WiFi.status() == WL_CONNECTED && receivedValue != 0 && receivedValue != lastReceivedValue) {
      lastReceivedValue = receivedValue;

      sequence++;
      if (sequence % WRITE_INTERVAL == 0) {
        EEPROM.begin(sizeof(unsigned long));
        for (int i = 0; i < sizeof(unsigned long); i++) {
          EEPROM.write(i, (sequence >> i * 8) & 0xff);
        }
        EEPROM.commit();
      }

      char payload[BUFSIZE];
      snprintf(payload, BUFSIZE, "%lu,%lu", receivedValue, sequence);

      uint8_t hmac[HASH_SIZE];
      sha256.resetHMAC(KEY, strlen(KEY));
      sha256.update(payload, strlen(payload));
      sha256.finalizeHMAC(KEY, strlen(KEY), hmac, sizeof(hmac));

      char hmac_str[HASH_SIZE * 2 + 1];
      for (int i = 0; i < HASH_SIZE; i++) {
        sprintf(hmac_str + i * 2, "%02x", hmac[i]);
      }

      char buf[BUFSIZE];
      snprintf(buf, BUFSIZE, CLOUD_FUNCTION_URL "report?payload=%s&hmac=%s", payload, hmac_str);
      Serial.println(buf);

      HTTPClient http;
      http.begin(buf);
      int httpCode = http.POST("");
      Serial.println(httpCode);
      http.end();
    }

    mySwitch.resetAvailable();
  }
}
