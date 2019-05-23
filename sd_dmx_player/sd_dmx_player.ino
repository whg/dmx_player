#include <SPI.h>
#include <SD.h>

// framerate is limited to 255fps

//#define DEBUG
#define FILENAME "for.raw"
#define INITIAL_DELAY 60 // seconds
#define TRIGGER_PIN A0
#define NUM_RUNS 3
#define TIMEOUT_SECONDS 3.2

#define DMX_CHANNELS 50
#define DATA_START 6

///////////////////////////////////////////////////

enum state_t { STOPPED, PLAYING, PAUSED };
state_t state = PLAYING;

File dataFile;

uint8_t buffer[DMX_CHANNELS];
uint16_t numChannels;

uint32_t numRows, frameCounter, runCounter;
uint32_t lastFrameTime;
uint32_t frameDelay;

#ifdef TIMEOUT_SECONDS
uint32_t lastTriggerPinTime = 0;
uint8_t lastTriggerPinState;
#endif

#define NOT_SET -1
int triggerPin = NOT_SET;

const uint8_t greenLed = A4, redLed = A5;


class DMX {
  public:
    static void init() {
      // 2 stop bits, 8 data bits
      UCSR0C = (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01);
    }

    static void write(uint8_t *data, uint16_t len) {
      Serial.begin(125000);
      Serial.write(static_cast<uint8_t>(0));
      Serial.flush();

      Serial.begin(250000);
      Serial.write(static_cast<uint8_t>(0));
      Serial.write(data, len);
      Serial.flush();
    }
};

void reset() {
  frameCounter = 0;
  dataFile.seek(DATA_START);
}

void setup() {

#ifdef DEBUG
  Serial.begin(115200);
#else
  DMX::init();
#endif

  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

#ifdef TRIGGER_PIN
  pinMode(triggerPin, INPUT_PULLUP);
#endif

  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);


  if (!SD.begin(10)) {
#ifdef DEBUG
    Serial.println("SD fail");
#endif
    //init failed
    while (true) {
      digitalWrite(redLed, millis() % 1000 < 500);
    }
  }

#ifdef DEBUG
  Serial.println("SD inited");
#endif

  dataFile = SD.open("fog.raw");
  if (dataFile) {

    uint8_t u8v;
    uint16_t u16v;

    uint8_t magic = dataFile.read();
    uint8_t version = magic & 0xf;

    dataFile.read((uint8_t*) &numRows, 4);
#ifdef DEBUG
    Serial.println(int(numRows));
#endif

    dataFile.read((uint8_t*) &numChannels, 2);
#ifdef DEBUG
    Serial.println(int(numChannels));
#endif

    uint8_t framerate = dataFile.read();
    // intial versions of raw files had 23 has header
    if (version == 7) {
      framerate = 44;
    }

    frameDelay = 1000 / framerate;

  } else {
    // file open error
    while (true) {
      digitalWrite(redLed, LOW);
    }
  }

#ifdef TRIGGER_PIN
  triggerPin = TRIGGER_PIN;
  state = STOPPED;
#endif


  reset();
  runCounter = 0;

  while (millis() / 1000 < INITIAL_DELAY) {
    digitalWrite(greenLed, millis() % 1000 < 500);
  }
  digitalWrite(greenLed, LOW);
}

void loop() {
#ifdef TRIGGER_PIN
  uint8_t triggerState = digitalRead(triggerPin);
  if (triggerState == LOW) {
    if (state == STOPPED) {
      reset();
    }
    state = PLAYING;
  }

#ifdef TIMEOUT_SECONDS
  uint32_t now = millis();
  if (triggerState != lastTriggerPinState) {
    lastTriggerPinTime = now;
    lastTriggerPinState = triggerState;
  }

  if (now - lastTriggerPinTime >= static_cast<uint32_t>(TIMEOUT_SECONDS * 1000)) {
    state == PAUSED;
  }
#endif

#endif


  if (state != PLAYING) {
    return;
  }

  uint32_t start = millis();

  dataFile.read(buffer, numChannels);


#ifdef DEBUG
  Serial.print(buffer[0]);
  Serial.print(",");
  Serial.println(buffer[1]);
#else
  DMX::write(buffer, numChannels);
#endif


  if (frameCounter++ >= numRows) {
    reset();

#ifdef NUM_RUNS
    if (++runCounter >= NUM_RUNS) {
      state = STOPPED;
    }
#endif


#ifdef DEBUG
    Serial.println("---");
#endif
  }

  uint32_t took = millis() - start;
  delay(frameDelay - took);
}
