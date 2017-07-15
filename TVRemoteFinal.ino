
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
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("TRANSMISSION");
  
  btSerial.begin(38400); //begin bluetooth serial port
  
  //set mode to idle
  isLearning = false;
  isSending = false;
  isAvailable = false;

  irrecv.enableIRIn(); //enable IR receiver to receive data
}

void loop() {

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
    Serial.println(input);
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
    int type = input.substring(pos1+1, pos2).toInt();
    int bits = input.substring(pos2+1).toInt();
    
    //for debugging
    Serial.println("==== Input ====")
    Serial.print("Data: ")
    Serial.println(data);
    Serial.print("Type: ")
    Serial.println(type);
    Serial.print("Bits: ")
    Serial.println(bits);

    //emit ir according to the type
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
      //unknown encoding types
      Serial.println("Other");
    }

  }

  //ir receiving
  if (isLearning) {
    if (irrecv.decode(&results)) {
      //make string to send
      String output = (String)results.value + "," +
                      (String)results.decode_type + "," +
                      (String)results.bits;
      btSerial.println(output);
      Serial.println(output; //for debugging
      irrecv.resume(); //resume ir receiver
      delay(100);
    }
  }
}
