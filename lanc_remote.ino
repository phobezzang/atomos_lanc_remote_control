#define LANC_PIN 11
#define BUTTON_PIN 3
#define LED_PIN 13

#define DEBUG 0
unsigned int bitDuration = 104;

unsigned long frameCounter = 0;

// Plug-in frames (3 blank frames)
byte plugInFrames[3][8] = {
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

// Standby burst frames (30 frames) — replace with your actual capture if needed
byte standbyFrames[30][8] = {
  {0xFF,0xFF,0xB6,0xFF,0xEB,0xFF,0xFF,0xFF}
  // Repeat this frame 30 times or fill with captured data
};

// Record start frames (30 frames)
byte recordStartFrames[30][8] = {
  {0xFF,0xFF,0xB6,0xFF,0xFB,0xFF,0xFF,0xFF}
  // Fill with remaining captured frames
};

// Record stop frames (30 frames)
byte recordStopFrames[30][8] = {
  {0xFF,0xFF,0xB6,0xFF,0xEB,0xFF,0xFF,0xFF}
  // Fill with captured stop frames
};

// Send a single byte
void sendByte(byte b) {
  digitalWrite(LANC_PIN, LOW);
  delayMicroseconds(bitDuration);
  for (int i=0;i<8;i++){
    if (b & 0x01) digitalWrite(LANC_PIN, HIGH);
    else digitalWrite(LANC_PIN, LOW);
    b >>= 1;
    delayMicroseconds(bitDuration);
  }
  digitalWrite(LANC_PIN, HIGH);
  delayMicroseconds(bitDuration);
}

// Send a frame
void sendFrame(byte f[8]) {
  for (int i=0;i<8;i++) sendByte(f[i]);
  frameCounter++;
  if (DEBUG){
    Serial.print("TX Frame ");
    Serial.print(frameCounter);
    Serial.print(": ");
    for (int i=0;i<8;i++){
      if (f[i]<0x10) Serial.print("0");
      Serial.print(f[i], HEX); Serial.print(" ");
    }
    Serial.println();
  }
}

// Send array of frames
void sendFrameArray(byte frames[][8], int numFrames, unsigned int spacingMs){
  for (int i=0;i<numFrames;i++){
    sendFrame(frames[i]);
    delay(spacingMs);
  }
}

void setup(){
  pinMode(LANC_PIN, OUTPUT);
  digitalWrite(LANC_PIN, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  if (DEBUG) Serial.println("LANC Debug Sender Ready");
  frameCounter = 0;

  // Initial plug-in frames + standby
  if (DEBUG) Serial.println("Sending plug-in frames...");
  sendFrameArray(plugInFrames, 3, 20);
  if (DEBUG) Serial.println("Sending standby burst...");
  sendFrameArray(standbyFrames, 30, 33);
}

void loop(){
  static bool lastButtonState = HIGH;
  bool currentButton = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && currentButton == LOW){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));

    // Alternate start/stop each press
    static bool recording = false;
    if (!recording){
      if (DEBUG) Serial.println("Button pressed → sending record start frames...");
      sendFrameArray(recordStartFrames, 30, 33);
      recording = true;
    } else {
      if (DEBUG) Serial.println("Button pressed → sending record stop frames...");
      sendFrameArray(recordStopFrames, 30, 33);
      recording = false;
    }
    delay(500);
  }

  lastButtonState = currentButton;
}
