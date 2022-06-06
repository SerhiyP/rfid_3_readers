
/**
   --------------------------------------------------------------------------------------------------------------------
   Example sketch/program showing how to read data from more than one PICC to serial.
   --------------------------------------------------------------------------------------------------------------------
   This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid

   Example sketch/program showing how to read data from more than one PICC (that is: a RFID Tag or Card) using a
   MFRC522 based RFID Reader on the Arduino SPI interface.

   Warning: This may not work! Multiple devices at one SPI are difficult and cause many trouble!! Engineering skill
            and knowledge are required!

   @license Released into the public domain.

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS 1    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required *
   SPI SS 2    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required *
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

*/

#include <SPI.h>
#include <MFRC522.h>

// PIN Numbers : RESET + SDAs
#define RST_PIN         9
#define SS_1_PIN        10
#define SS_2_PIN        8
#define SS_3_PIN        7

#define relayIN         3

// Number of readers
#define NR_OF_READERS   3

byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN};

// Create an MFRC522 instance :
MFRC522 mfrc522[NR_OF_READERS];
void(* resetFunc) (void) = 0;

/**
   Initialize.
*/
void setup() {

  Serial.begin(9600);           // Initialize serial communications with the PC
  while (!Serial);              // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  SPI.begin();                  // Init SPI bus

  /* Initializing Inputs and Outputs */
  pinMode(relayIN, OUTPUT);
  digitalWrite(relayIN, HIGH);

  /* looking for MFRC522 readers */
  for (int reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
    Serial.println();
  }
}

int delayTime = 2000;
int ITERATIONS = 0;
int possibleIteration = (30 * 600000) / delayTime;

void loop() {
  ITERATIONS++;

  Serial.println(ITERATIONS);
  Serial.println(possibleIteration);

  if (ITERATIONS > possibleIteration) {
    Serial.println("RESET");
    resetFunc();
  }

  int cards = 0;

  for (int reader = 0; reader < NR_OF_READERS; reader++) {
    Serial.print(F("Reader "));
    Serial.print(reader);

    // Looking for new cards
    if ( mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);

      cards++;
      mfrc522[reader].PICC_IsNewCardPresent();
    }
    Serial.println();
  }

  if (cards == NR_OF_READERS) {
    OpenDoor();
  } else {
    CloseDoor();
  }

  // Set some delay to don't do checks to often
  delay(delayTime);
}

void OpenDoor()
{
  Serial.println("Welcome! the door is now open");
  digitalWrite(relayIN, LOW);
}

void CloseDoor()
{
  Serial.println("Door closed");
  digitalWrite(relayIN, HIGH);
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
