#include <SPI.h>
#include <SD.h>

// framerate is limited to 255fps

//#define DEBUG
#define INITIAL_DELAY 60 // seconds

#define DMX_CHANNELS 50
#define DATA_START 6

File dataFile;

uint8_t buffer[DMX_CHANNELS];
uint16_t numChannels;

uint32_t numRows, frameCounter;
uint32_t lastFrameTime;
uint32_t frameDelay;

const uint8_t greenLed = A4, redLed = A5;

class DMX {
public:
  static void init() {
    // 2 stop bits, 8 data bits
    UCSR0C = (1<<USBS0) | (1<<UCSZ00) | (1<<UCSZ01);
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
    if (version == 2) {
      framerate = 44;
    }

    frameDelay = 1000 / framerate;

  } else {
    // file open error
    while (true) {
      digitalWrite(redLed, LOW);
    }
  }

  reset();
  while (millis() / 1000 < INITIAL_DELAY) {
    digitalWrite(greenLed, millis() % 1000 < 500);
  }
  digitalWrite(greenLed, LOW);
}

void loop() {
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
#ifdef DEBUG
    Serial.println("---");
#endif
  }

  uint32_t took = millis() - start;
  delay(frameDelay - took);
}
