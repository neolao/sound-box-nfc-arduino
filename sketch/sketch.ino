#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// LED
#define LED_PIN 9

// NFC reader with I2C
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

// MP3 Player
#define PLAYER_RX_PIN 6
#define PLAYER_TX_PIN 7
uint8_t volume = 15;
int fileCount = 0;
uint8_t fileIndex = 0;
SoftwareSerial playerSerial(PLAYER_RX_PIN, PLAYER_TX_PIN);
DFRobotDFPlayerMini player; 

void setup(void) {
  setupLed();
  setupPlayer();

  Serial.begin(115200);

  Serial.println("Initialization");
  initialize();
  Serial.println("Initialized");

  nfc.begin();
}

void setupLed() {
  pinMode(LED_PIN, OUTPUT);
}

void setupPlayer() {
  playerSerial.begin(9600);
  player.begin(playerSerial);
  player.setTimeOut(500);
  player.reset();
  player.disableDAC();
  player.volume(volume); 
}

void initialize() {
  while (fileCount < 2) {
    if (player.available()) {
      fileCount = player.readFileCounts(DFPLAYER_DEVICE_SD);
    }
  }
  Serial.print("File count: ");
  Serial.println(fileCount);
}

String command = "";
void loop() {
  command = readCommandFromNFC();
  if (command != "") {
    Serial.print("Command: ");
    Serial.println(command);
  
    fileIndex = command.toInt();
    Serial.print("File index: ");
    Serial.println(fileIndex);

    blinkLed();
    player.playMp3Folder(fileIndex);
  }

  delay(500);
}

void blinkLed() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW); 
}

String readCommandFromNFC() {
  if (!nfc.tagPresent()) {
    return "";
  }
  
  NfcTag tag = nfc.read();

  if (!tag.hasNdefMessage()) {
    return "";
  }

  
  NdefMessage message = tag.getNdefMessage();
  int recordCount = message.getRecordCount();
  for (int i = 0; i < recordCount; i++) {
    NdefRecord record = message.getRecord(i);

    /*
    Serial.print("  TNF: ");
    Serial.println(record.getTnf());
    Serial.print("  Type: ");
    Serial.println(record.getType()); // will be "" for TNF_EMPTY
    */

    // The TNF and Type should be used to determine how your application processes the payload
    // There's no generic processing for the payload, it's returned as a byte[]
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);
    //Serial.print("  Payload (HEX): ");
    //PrintHexChar(payload, payloadLength);

    // Skip characters because the payload contains headers
    int skipCharacterCount = (int) payload[0] + 1;
    String payloadAsString = "";
    for (int characterIndex = skipCharacterCount; characterIndex < payloadLength; characterIndex++) {
      payloadAsString += (char)payload[characterIndex];
    }
    //Serial.print("  Payload (as String): ");
    //Serial.println(payloadAsString);
    return payloadAsString;
  }
}
