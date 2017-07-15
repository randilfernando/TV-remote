
#include <SoftwareSerial.h>
#include <IRremote.h>

SoftwareSerial btSerial(10, 11);

int RECV_PIN = 12;
IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results;

bool isLearning;
bool isSending;
bool isAvailable;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // for debug purpuses
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("===Transmitting===");
  btSerial.begin(38400);
  isLearning = false;
  isSending = false;
  isAvailable = false;
  irrecv.enableIRIn();
}

void loop() {

  String input;

  if (btSerial.available()) {
    input = btSerial.readString();
    input.trim();
    if (input == "LEARN") {
      Serial.println("LEARN MODE");
      isLearning = true;
      isSending = false;
    } else if (input == "SEND") {
      Serial.println("SEND MODE");
      isSending = true;
      isLearning = false;
    } else {
      isAvailable = true;
    }
    btSerial.read();
    btSerial.read();
  } else {
    isAvailable = false;
  }

  if (isAvailable && isSending) {
    Serial.println(input);
    int count = 0;
    int pos1, pos2, pos3;
    for (int i = 0; i < input.length(); i++) {
      if (input[i] == ',') {
        if (count == 0) {
          pos1 = i;
        } else if (count == 1) {
          pos2 = i;
        } else {
          break;
        }
        count ++;
      }
    }

    Serial.println(input.substring(0, pos1));

    long data = input.substring(0, pos1).toInt();
    int type = input.substring(pos1+1, pos2).toInt();
    int bits = input.substring(pos2+1).toInt();
    
    //debug
    Serial.println(data);
    Serial.println(type);
    Serial.println(bits);

    if (type == NEC) {
      irsend.sendNEC(data, bits);
    }
    else if (type == SONY) {
      irsend.sendSony(data, bits);
    }
    else if (type == RC5) {
      irsend.sendRC5(data, bits);
    }
    else if (type == RC6) {
      irsend.sendRC6(data, bits);
    }else{
      Serial.println("Other");
    }

  }

  if (isLearning) {
    if (irrecv.decode(&results)) {
      Serial.println("Learning");
      String output = (String)results.value + "," +
                      (String)results.decode_type + "," +
                      (String)results.bits;
      btSerial.println(output);
      Serial.println(results.value, HEX);
      irrecv.resume(); // Receive the next value
      delay(100);
    }
  }
}
