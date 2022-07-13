#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

void setup(void) {
  Serial.begin(115200);
  Serial.println("System initialized");
  nfc.begin();
}

void loop() {
  readNFC();
}

void readNFC() {
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();

    if (tag.hasNdefMessage()) {
      NdefMessage message = tag.getNdefMessage();
      int recordCount = message.getRecordCount();
      for (int i = 0; i < recordCount; i++) {
        NdefRecord record = message.getRecord(i);

        Serial.print("  TNF: ");
        Serial.println(record.getTnf());
        Serial.print("  Type: ");
        Serial.println(record.getType()); // will be "" for TNF_EMPTY

        // The TNF and Type should be used to determine how your application processes the payload
        // There's no generic processing for the payload, it's returned as a byte[]
        int payloadLength = record.getPayloadLength();
        byte payload[payloadLength];
        record.getPayload(payload);
        Serial.print("  Payload (HEX): ");
        PrintHexChar(payload, payloadLength);

        // Skip characters because the payload contains headers
        int skipCharacterCount = (int) payload[0] + 1;
        String payloadAsString = "";
        for (int characterIndex = skipCharacterCount; characterIndex < payloadLength; characterIndex++) {
          payloadAsString += (char)payload[characterIndex];
        }
        Serial.print("  Payload (as String): ");
        Serial.println(payloadAsString);
      }
    }
  }
  delay(500);
}
