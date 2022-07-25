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
struct Command {
    String action;
    String value;
};

// MP3 Player
#define PLAYER_RX_PIN 6
#define PLAYER_TX_PIN 7
uint8_t volume = 17;
int fileCount = 0;
uint8_t fileIndex = 0;
bool isPlaying = false;
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

void loop() {
  Command command = readCommandFromNFC();
  processCommand(command);

  if (player.available() && player.readType() == DFPlayerPlayFinished) {
    isPlaying = false;
  }

  delay(500);
}

void blinkLed() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW); 
}

Command readCommandFromNFC() {
  struct Command command;
  
  if (!nfc.tagPresent()) {
    return { "unknown", "unknown" };
  }
  
  NfcTag tag = nfc.read();

  if (!tag.hasNdefMessage()) {
    return { "unknown", "unknown" };
  }

  
  NdefMessage ndefMessage = tag.getNdefMessage();
  int recordCount = ndefMessage.getRecordCount();
  String messages[2];
  for (int messageIndex = 0; messageIndex < recordCount && messageIndex < 2; messageIndex++) {
    NdefRecord record = ndefMessage.getRecord(messageIndex);

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
    messages[messageIndex] = payloadAsString;
  }

  
  if (messages[0] && messages[1]) {
    return { messages[0], messages[1] };
  }
  
  return { "unknown", "unknown" };
}

void processCommand(Command command) {
  if (command.action == "unknown") {
    return;
  }
  Serial.print("Command: ");
  Serial.print(command.action);
  Serial.print(" > ");
  Serial.println(command.value);

  if (command.action == "play") {
    play(command.value.toInt());
  } else if (command.action == "volume") {
    setVolume(command.value.toInt());
  }
}

void play(uint8_t index) {
  Serial.print("Play index: ");
  Serial.println(index);
  Serial.println("is playing? ");
  Serial.println(isPlaying);
  Serial.println("Current index: ");
  Serial.println(fileIndex);

  if (!isPlaying || index != fileIndex) {
    blinkLed();
    player.playMp3Folder(index);
    fileIndex = index;
    isPlaying = true;
    delay(500);  
  }
}

void setVolume(uint8_t newVolume) {
  Serial.print("New volume: ");
  Serial.println(newVolume);

  blinkLed();
  player.volume(newVolume);
  volume = newVolume;
}
