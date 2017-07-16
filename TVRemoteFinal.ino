
#include <SoftwareSerial.h>
#include <IRremote.h>

SoftwareSerial btSerial(11, 10); //make virtual serial port

int RECV_PIN = 12; //IR receiving pin
IRrecv irrecv(RECV_PIN); //make IRRemote library object to receive IR
IRsend irsend; //make IRRemote library object to send IR
decode_results results; //save decoded value (data, decode_type, bits)

//flags to maintain mode
bool isLearning;
bool isSending;
bool isAvailable;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // for debug purpuses
  while (!Serial) {
    ; // wait for serial port to connect. needed for native USB port only
  }
  Serial.println("TV REMOTE");

  btSerial.begin(38400); //begin bluetooth serial port

  //set mode to idle
  isLearning = false;
  isSending = false;
  isAvailable = false;

  irrecv.enableIRIn(); //enable IR receiver to receive data
}

void loop() {
  // put your main code here, to run repeatedly:
  String input; //store the input of bluetooth module

  //if ther are an input in bluetooth module
  if (btSerial.available()) {
    input = btSerial.readString(); //read as a string
    input.trim();
    if (input == "LEARN") {
      //enable ir receive mode mode, disable ir send mode
      Serial.println("LEARN MODE");
      isLearning = true;
      isSending = false;
    } else if (input == "SEND") {
      //enable ir send mode, disable ir receive mode
      Serial.println("SEND MODE");
      isSending = true;
      isLearning = false;
    } else {
      //resume ir send mode
      isAvailable = true;
    }
    btSerial.read();
    btSerial.read();
  } else {
    //pause ir send mode
    isAvailable = false;
  }

  //ir emitting
  if (isAvailable && isSending) {
    int count = 0;
    int pos1, pos2, pos3;

    //divide into data, decode_type, bits
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
    long data = input.substring(0, pos1).toInt();
    int decode_type = input.substring(pos1 + 1, pos2).toInt();
    int bits = input.substring(pos2 + 1).toInt();

    //emit ir according to the type
    switch (decode_type) {
      case NEC  :
        irsend.sendNEC(data, bits);
        Serial.print("NEC ("); //for debugging
        break;
      case SONY :
        irsend.sendSony(data, bits);
        Serial.print("SONY ("); //for debugging
        break;
      case RC5  :
        irsend.sendRC5(data, bits);
        Serial.print("RC5 ("); //for debugging
        break;
      case RC6  :
        irsend.sendRC6(data, bits);
        Serial.print("RC6 ("); //for debugging
        break;
      default   :
        Serial.print("UNKNOWN ("); //type not equal to above declared types (ignore signal)
    }

    //for debugging
    Serial.print("data_long: ");
    Serial.print(data);
    Serial.print(" data_hex: ");
    Serial.print(data, HEX);
    Serial.print(" bits: ");
    Serial.print(bits);
    Serial.print(" type: ");
    Serial.print(decode_type);
    Serial.println(")");
  }

  //ir receiving
  if (isLearning) {
    if (irrecv.decode(&results)) {
      //make string to send
      int decode_type = results.decode_type;
      String output = (String)results.value + "," +
                      (String)results.decode_type + "," +
                      (String)results.bits;

      //for debugging
      switch (decode_type) {
        case NEC  : Serial.print("NEC: "); break;
        case SONY : Serial.print("SONY: "); break;
        case RC5  : Serial.print("RC5: "); break;
        case RC6  : Serial.print("RC6: "); break;
        default   :
          output = "-1"; //if the encode type is not a one defined above. send unknown signal via bluetooth
          Serial.print("UNKNOWN: "); break;
      }

      btSerial.println(output);
      irrecv.resume(); //resume ir receiver
      delay(200); //delay program
    }
  }
}
