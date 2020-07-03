

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         D1          // Configurable, see typical pin layout above
#define SS_PIN          D2         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

unsigned long previousMillis = 0; 
const long clearDelay = 60000;
const long minInterval = 5000;

void setupRfid() {
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
}

void loopRfid() {
  unsigned long currentMillis = millis();
  if (previousMillis != 0 && currentMillis - previousMillis >= clearDelay) {
      Serial.println("Clearing last card state!");
      publish2MQTT("rfid", "");
      previousMillis = 0;
  }
  if (previousMillis != 0 && currentMillis - previousMillis <= minInterval) {
      return;
  }
  
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  Serial.println("Found a card!");

  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
    cardUID += String(mfrc522.uid.uidByte[i], HEX);

  if (cardUID == "2253f934" || cardUID == "1930efb8") {
    Serial.println("Success!");
    publish2MQTT("rfid", "success");
    previousMillis = currentMillis;
  } else {
    Serial.println("Invalid card - ignoring!");
  }

  // Sleep after a successful read to prevent re-reading the same card many times
  
}
