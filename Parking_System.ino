#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield
   ();

#include <TimeLib.h>

#include <MemoryFree.h>


//time variable
unsigned long time;

//States of Finite State Machine
enum State { 
  Sync,
  Main,
  AddCar,
  PaymentStatus,
  ChangeType,
  ChangeLocation,
  RemoveCar,
  SelectViewing
};

//Vehicle Class defines attributes of vehicles
class Vehicle { 
  public:
    String registrationNumber;
    String type;
    String location;
    String paymentStatus;
    unsigned long timeOfEntry;
    unsigned long timeOfPayment;
};

//Max tested was 19 vehicles -> 12 chosen for long term stability
const int arraysize = 12;
Vehicle vehicleArray[arraysize];
int vehicleCount = 0;

//====Check Methods====

//Checks if valid vehicle type - C, M, V, L, B
bool checkType(String type) {
  type.trim();
  if (type.equals("C") || type.equals("M") || type.equals("V") || type.equals("L") || type.equals("B")){
    // Serial.println(F("DEBUG: Vehicle type valid"));
    return true;
  } else {
    Serial.println(F("ERROR: Vehicle Type Invalid"));
    return false;
  }
}

//Checks if valid registration number - length and format
bool checkReg(String reg) {
  if (reg.length()==7) {
    // Serial.println("DEBUG: Reg is valid length");

    // check each character is valid or not
    for (int i=0; i<reg.length(); i++){
      if (isAlpha(reg.charAt(0)) && isUpperCase(reg.charAt(0)) && isAlpha(reg.charAt(1)) && isUpperCase(reg.charAt(1))
      && isDigit(reg.charAt(2)) && isDigit(reg.charAt(3)) && isAlpha(reg.charAt(4)) && isUpperCase(reg.charAt(4)) && isAlpha(reg.charAt(5)) && isUpperCase(reg.charAt(5))
       && isAlpha(reg.charAt(6)) && isUpperCase(reg.charAt(6))) {
        // Serial.println("DEBUG: Reg Character Valid");
      } else {
        Serial.println(F("ERROR: Reg Invalid"));
        return false;
      }
    }

    // Serial.println(F("DEBUG: Entire Reg valid"));
    return true;  
  } else {
    Serial.println(F("ERROR: Wrong Reg Length"));
    return false;
  }
}

bool checkLocation(String location) {
  if (location.length()>=1 && location.length()<=11) {
    // Serial.println("DEBUG: Location correct length");

     for (int i=0; i<location.length(); i++){
      if (isAlpha(location.charAt(i))) {
        // Serial.println("DEBUG: Location Alpha Character Valid");
      } else if (isDigit(location.charAt(i))) {
          // Serial.println("DEBUG: Location Digit Character Valid");
      } else if (location.charAt(i)=='.') {
          // Serial.println("DEBUG: Location Punctuation Allowed");
      } else {
          Serial.println(F("ERROR: Location Characters Invalid"));
          return false;
      }
    }
  // Serial.println(F("DEBUG: Entire Location Valid"));
  return true;
  } else {
    Serial.println(F("ERROR: Location Wrong Length"));
    return false;
  }
}

bool checkPayment(String payment) {
  payment.trim();
  if (payment.equals("PD") || payment.equals("NPD")) {
    // Serial.println(F("DEBUG: Payment Status Valid"));
    return true;
  } else {
    Serial.println(F("ERROR: Payment Status Invalid"));
    return false;
  }
}

//Main Methods
String input;
void addVehicle(String input) {
  int dashNum = 0;
  int spaceNum = 0;

  // checks if spaces or too many dashes
  for (int i=0; i<input.length();i++) {
    if (input[i]=='-') {
      dashNum++; 
    }
    if (input[i]==' ') {
      spaceNum++; 
    }
  }
  int splitNum = dashNum + 1;
    
  if (spaceNum != 0 && dashNum != 3) {
    Serial.println(F("ERROR: Input Invalid, Check for Spaces and Extra Dashes"));
    return;
  } else if (spaceNum != 0) {
      Serial.println(F("ERROR: Input Invalid, No Spaces Allowed"));
    return;
  } else if (dashNum > 3) {
      Serial.println(F("ERROR: Input Invalid, Remove Extra Dashes"));
    return;
  } else if (dashNum != 3 && splitNum != 4) {
    Serial.println(F("ERROR: Input Missing Field"));
    return;
  }

  if (vehicleCount >= arraysize) {
      Serial.println(F("ERROR: At Capacity"));
      return;
  }
   
  String splits[4]; //split 
  split(input, splits);

  //Updates Vehicle:
    for (int i=0; i<arraysize; i++) {
      if ((splits[1] == vehicleArray[i].registrationNumber)) {
        // Serial.println(F("DEBUG: Reg Num Exists"));

        if (checkType(splits[2]) && checkLocation(splits[3])) {
          if (vehicleArray[i].paymentStatus == "PD") {
            if (vehicleArray[i].type != splits[2]) {
              vehicleArray[i].type = splits[2];
              Serial.println(F("DEBUG: Updating Type"));
              Serial.println(F("DONE!"));
            } else {
              Serial.println(F("ERROR: Type Unchanged"));
            }
            if (vehicleArray[i].location != splits[3]) {
              lcd.setCursor(9, 0);
              lcd.print("       ");
              vehicleArray[i].location = splits[3];
              Serial.println(F("DEBUG: Updating Location"));
              Serial.println(F("DONE!"));
            } else {
              Serial.println(F("ERROR: Location Unchanged"));
            }
            
          } else {
            Serial.println(F("ERROR: Payment NPD, Cannot Update"));
            // return;
          }
        } else {
          Serial.println(F("ERROR: Type or Location Invalid"));
        } return;
      } else {
        // Serial.println(F("DEBUG: Reg Num Doesn't Exist, So Add As New"));
      } 
    } 

    //Serial.println("DEBUG: Can accept vehicles");
    //Adds new vehicle
    if (checkReg(splits[1]) && checkType(splits[2]) && checkLocation(splits[3])) {
        vehicleArray[vehicleCount].registrationNumber = splits[1];
        vehicleArray[vehicleCount].type = splits[2];
        vehicleArray[vehicleCount].location = splits[3];
        vehicleArray[vehicleCount].paymentStatus = "NPD";
        vehicleArray[vehicleCount].timeOfEntry = now();
        vehicleCount++;
        Serial.println(F("DEBUG: Adding Vehicle"));
        Serial.println(F("DONE!"));
    } else {
      Serial.println(F("ERROR: Add Vehicle Invalid Input"));
    }

}

/* NOTE: If change from PD to NPD, the vehicle moves to the bottom of the array, to keep 
 it being in order of time of entry */
void changePayment(String input) {

  //Checks for too many dashes or spaces
  int dashNum = 0;
  int spaceNum = 0;
  for (int i=0; i<input.length();i++) {
    if (input[i]=='-') {
      dashNum++; 
    }
    if (input[i]==' ') {
      spaceNum++; 
    }
  }

 int splitNum = dashNum + 1;
    
  if (spaceNum != 0 && dashNum != 2) {
    Serial.println(F("ERROR: Input Invalid, Check for Spaces and Extra Dashes"));
    return;
  } else if (spaceNum != 0) {
      Serial.println(F("ERROR: Input Invalid, No Spaces Allowed"));
    return;
  } else if (dashNum > 2) {
      Serial.println(F("ERROR: Input Invalid, Remove Extra Dashes"));
    return;
  } else if (dashNum != 2 && splitNum != 3) {
    Serial.println(F("ERROR: Input Missing Field"));
    return;
  }
  
  String splits[3];
  split(input, splits);

  //sets payment and adds timestamp
  if (checkPayment(splits[2])) {
    for (int i=0; i<arraysize; i++) {
      if (splits[1] == vehicleArray[i].registrationNumber) {
        // Serial.println(F("DEBUG: Reg Num exists"));

        if (vehicleArray[i].paymentStatus == splits[2]) {
          Serial.println(F("ERROR: Payment Status Unchanged"));
        } else if (vehicleArray[i].paymentStatus != splits[2]) {
          Serial.println(F("DEBUG: Updating Payment Status"));
          vehicleArray[i].paymentStatus = splits[2];
          Serial.println(F("DONE!"));

          if (vehicleArray[i].paymentStatus == "PD") {
            vehicleArray[i].timeOfPayment = now();
          }
          if (splits[2] == "NPD") {
            vehicleArray[i].paymentStatus = "NPD";
            vehicleArray[i].timeOfEntry = now();
          }
        }
        return;

      } 
    } Serial.println(F("ERROR: Reg Num doesn't exist")); 
  } else {
    // Serial.println(F("ERROR: Invalid Payment Status"));
  }


}

void changeType(String input) {

  //checks for too many dashes and spaces
  int dashNum = 0;
  int spaceNum = 0;
  for (int i=0; i<input.length();i++) {
    if (input[i]=='-') {
      dashNum++; 
    }
    if (input[i]==' ') {
      spaceNum++; 
    }
  }
  
 int splitNum = dashNum + 1;
    
  if (spaceNum != 0 && dashNum != 2) {
    Serial.println(F("ERROR: Input Invalid, Check for Spaces and Extra Dashes"));
    return;
  } else if (spaceNum != 0) {
      Serial.println(F("ERROR: Input Invalid, No Spaces Allowed"));
    return;
  } else if (dashNum > 2) {
      Serial.println(F("ERROR: Input Invalid, Remove Extra Dashes"));
    return;
  } else if (dashNum != 2 && splitNum != 3) {
    Serial.println(F("ERROR: Input Missing Field"));
    return;
  }
  
  //splits
  String splits[3];
  split(input, splits);

  //updates type if paid 
  if (checkType(splits[2])) {
    for (int i=0; i<arraysize; i++) {
      if (splits[1] == vehicleArray[i].registrationNumber) {
        // Serial.println(F("DEBUG: Reg Num Exists"));

        if (vehicleArray[i].type == splits[2]) {
          Serial.println(F("ERROR: Type Unchanged"));
        } else if (vehicleArray[i].type != splits[2]) {
          if (vehicleArray[i].paymentStatus == "NPD") {
            Serial.println(F("ERROR: Payment Not Paid, Cannot Change Type"));
          } 
          else if (vehicleArray[i].paymentStatus == "PD") {
            Serial.println(F("DEBUG: Updating Type"));
            vehicleArray[i].type = splits[2];
            Serial.println(F("DONE!"));
          }
        }
        return;
      }
    } Serial.println(F("ERROR: Reg Num Doesn't Exist"));
  } else {
    Serial.println(F("ERROR: Type Invalid"));
  }

}


void changeLocation(String input) {
  //checks for too many dashes and spaces
  int dashNum = 0;
  int spaceNum = 0;
  for (int i=0; i<input.length();i++) {
    if (input[i]=='-') {
      dashNum++; 
    }
    if (input[i]==' ') {
      spaceNum++; 
    }
  }
  

 int splitNum = dashNum + 1;
    
  if (spaceNum != 0 && dashNum != 2) {
    Serial.println(F("ERROR: Input Invalid, Check for Spaces and Extra Dashes"));
    return;
  } else if (spaceNum != 0) {
      Serial.println(F("ERROR: Input Invalid, No Spaces Allowed"));
    return;
  } else if (dashNum > 2) {
      Serial.println(F("ERROR: Input Invalid, Remove Extra Dashes"));
    return;
  } else if (dashNum != 2 && splitNum != 3) {
    Serial.println(F("ERROR: Input Missing Field"));
    return;
  }

  
  //splits
  String splits[3];
  split(input, splits);
  //updates location if paid 
  if (checkLocation(splits[2])) {
    for (int i=0; i<arraysize; i++) {
      if (splits[1] == vehicleArray[i].registrationNumber) {
        // Serial.println(F("DEBUG: Reg Num Exists"));

        if (vehicleArray[i].location == splits[2]) {
          Serial.println(F("ERROR: Location Unchanged"));
        } else if (vehicleArray[i].location != splits[2]) {
          if (vehicleArray[i].paymentStatus == "NPD") {
            Serial.println(F("ERROR: Payment Not Paid, Cannot Change Location"));
          } 
          else if (vehicleArray[i].paymentStatus == "PD") {
            Serial.println(F("DEBUG: Updating Location"));
            lcd.setCursor(9, 0);
            lcd.print("       ");
            vehicleArray[i].location = splits[2];
            Serial.println(F("DONE!"));
          }
        }
        return;
      }
    } Serial.println(F("ERROR: Reg Num Doesn't Exist"));
  } else {
    Serial.println(F("ERROR: Location Invalid"));
  }



}

//Custom Arrow designs
int i = 0;
int arrayIndex = 0;
void printVehicle(int arrayIndex) {
  byte customChar1[8] = { //upper
    0b00100,
    0b01110,
    0b10101,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00000
  };
  byte customChar2[8] = { //lower
    0b00000,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b10101,
    0b01110,
    0b00100
  };

  //Custom arrows placement
  if (vehicleCount == 1) {
    lcd.setCursor(0,0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
  }
   else if (vehicleCount == 0) {
    lcd.setCursor(0,0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
  }
  else if (arrayIndex == 0) { // top of list
      lcd.createChar(1, customChar2);
      lcd.setCursor(0,1);
      lcd.write((uint8_t)1);
  } else if (arrayIndex == vehicleCount - 1) { // bottom of list
      lcd.createChar(0, customChar1);
      lcd.setCursor(0,0);
      lcd.write((uint8_t)0);
  } else { //anywhere inbetween
      lcd.setCursor(0,0);
      lcd.createChar(0, customChar1);
      lcd.createChar(1, customChar2);
      lcd.write((uint8_t)0);
      lcd.setCursor(0,1);
      lcd.write((uint8_t)1);
  }

  //Prints Reg and Type
  lcd.setCursor(1, 0);
  lcd.print(vehicleArray[arrayIndex].registrationNumber);
  lcd.setCursor(1, 1);
  lcd.print(vehicleArray[arrayIndex].type);
  
    //Location scroll
      if (vehicleArray[arrayIndex].location.length() > 7) {
        if (millis() - time > 500) {
          time = millis();
          lcd.setCursor(9, 0);
          lcd.print(vehicleArray[arrayIndex].location.substring(i,i+7));
          i++;
          if (i > vehicleArray[arrayIndex].location.length() - 7) {
            i=0;
          }
        }

    } else {
        lcd.setCursor(9, 0);
        lcd.print(vehicleArray[arrayIndex].location);
        
      
      
    }

  //Payment status and timestamps
  if (vehicleArray[arrayIndex].paymentStatus == "PD") {
    lcd.setCursor(4, 1);
    lcd.print(vehicleArray[arrayIndex].paymentStatus + " ");
    lcd.setCursor(3, 1);
    lcd.print(" ");

    
    lcd.setCursor(7, 1);
    char timeOfEntry[4];
    sprintf(timeOfEntry, "%02d%02d", hour(vehicleArray[arrayIndex].timeOfEntry), minute(vehicleArray[arrayIndex].timeOfEntry));
    lcd.print(timeOfEntry);

    lcd.setCursor(12, 1);
    char timeOfPayment[4];
    sprintf(timeOfPayment, "%02d%02d", hour(vehicleArray[arrayIndex].timeOfPayment), minute(vehicleArray[arrayIndex].timeOfPayment));
    lcd.print(timeOfPayment);
    lcd.setBacklight(2);


 
  }

  else if (vehicleArray[arrayIndex].paymentStatus == "NPD") {
    //clears overflow print
    lcd.setCursor(12,1);
    lcd.print(" ");
    lcd.setCursor(13,1);
    lcd.print(" ");
    lcd.setCursor(14,1);
    lcd.print(" ");
    lcd.setCursor(15, 1);
    lcd.print(" ");

    //Prints payment status and timestamp
    lcd.setCursor(3, 1);
    lcd.print(vehicleArray[arrayIndex].paymentStatus + " ");

    lcd.setCursor(7, 1);
    char timeOfEntry[4];
    sprintf(timeOfEntry, "%02d%02d", hour(vehicleArray[arrayIndex].timeOfEntry), minute(vehicleArray[arrayIndex].timeOfEntry));
    lcd.print(timeOfEntry);
    lcd.setBacklight(3);
    }

  //DEBUG Message 
  // Serial.print("DEBUG: Vehicle "); 
  // Serial.print(arrayIndex); 
  // Serial.print("Printed to LCD");

}

void sort() {
  static bool needsPrinting = false;
  for (int i=0; i< vehicleCount - 1; i++) { //loops through array
    for (int j=0; j < vehicleCount - i - 1; j++) { //restricted array
      if (vehicleArray[j].timeOfEntry > vehicleArray[j+1].timeOfEntry) {
        //swaps
        Vehicle swap = vehicleArray[j];
        vehicleArray[j] = vehicleArray[j + 1];
        vehicleArray[j+1] = swap;
        needsPrinting = true; 
      } 
    }
  } 
  if (needsPrinting) {
    Serial.println(F("DEBUG: Array Resorted due to Time of Entry Change"));
    needsPrinting = false; //resets flag
  }
}

void removeVehicle(String input) {
  int dashNum = 0;
  int spaceNum = 0;
  for (int i=0; i<input.length();i++) {
    if (input[i]=='-') {
      dashNum++; 
    }
    if (input[i]==' ') {
      spaceNum++; 
    }
  }
  

 int splitNum = dashNum + 1;
    
  if (spaceNum != 0 && dashNum != 1) {
    Serial.println(F("ERROR: Input Invalid, Check for Spaces and Extra Dashes"));
    return;
  } else if (spaceNum != 0) {
      Serial.println(F("ERROR: Input Invalid, No Spaces Allowed"));
    return;
  } else if (dashNum > 1) {
      Serial.println(F("ERROR: Input Invalid, Remove Extra Dashes"));
    return;
  } else if (dashNum != 1 && splitNum != 2) {
    Serial.println(F("ERROR: Input Missing Field"));
    return;
  }



  String splits[2];
  split(input, splits);

  
  int removeIndex;
  for (int i=0; i<arraysize; i++) {
    if (vehicleArray[i].registrationNumber == splits[1]) {
      // Serial.println(F("DEBUG: Reg Num Exists"));
      removeIndex = i; //finds index to remove
    }
  }
  if (vehicleArray[removeIndex].paymentStatus == "PD") {
    for (int i=removeIndex; i<arraysize - 1; i++) {
      vehicleArray[i] = vehicleArray[i+1];
      lcd.setBacklight(7); //if PD, shifts to elements to left
    }

    vehicleArray[vehicleCount-1] = Vehicle(); //sets last to null
    //decrements num of vehicles and array index to print
    vehicleCount--;
    if (arrayIndex > vehicleCount-1 && removeIndex != 0) {
      arrayIndex--;
    }
    
    Serial.println(F("DEBUG: Removing Vehicle"));
    Serial.println(F("DONE!"));
  } else {
    Serial.println(F("ERROR: Payment Not Paid, Cannot Remove"));
  }

}

void setup() {
   //Basic Setup
  Serial.begin(9600);
  lcd.begin(16,2);
  setTime(0,0,0,1,1,2023); //sets time to midnight 
  time = millis(); // starts timer
}

void loop() {
  static bool notCleared = true; //flag for clearing (select)
  static State state = Sync;

  switch(state) {
    case Sync: {
      syncronisation();
      state = Main;
    }
    break;

    case Main: {
      sort();
      printVehicle(arrayIndex);
      
      //conditions for transfer
      if (Serial.available()>0) {
        input = Serial.readStringUntil('\n');
        // Serial.println(input);

        if (input.substring(0,2) == ("A-")) {
          state = AddCar;
        }
        else if (input.substring(0,2) == "S-") {
          state = PaymentStatus;
        }
        else if (input.substring(0,2) == "T-") {
          state = ChangeType;
        } 
        else if (input.substring(0,2) == "L-") {
          state = ChangeLocation;
        }
        else if (input.substring(0,2) == "R-") {
          state = RemoveCar;
        } else {
          Serial.println(F("ERROR: Invalid Command"));
        }
      }

      //scroll and select state transfer
      uint8_t buttons = lcd.readButtons();
      if (buttons & BUTTON_DOWN) {
        if (arrayIndex < vehicleCount - 1) {
          arrayIndex++;
          lcd.clear();
        }
      } 
      else if (buttons & BUTTON_UP) {
        if (arrayIndex > 0) {
          arrayIndex--;
          lcd.clear();
        }
      }

      if (buttons & BUTTON_SELECT) {
          time = millis(); //starts timer
          state = SelectViewing;
        }

      
    }
    break;

    case AddCar: {
      addVehicle(input);
      state = Main;
    }
    break;

    case PaymentStatus: { 
      changePayment(input);
      state = Main;
    }
    break;


    case ChangeType: {
      changeType(input);
      state = Main;
    }
    break;

    case ChangeLocation: {
      changeLocation(input);
      state = Main;
    }
    break;

    case RemoveCar: {
      removeVehicle(input);
      lcd.clear();
      // lcd.setBacklight(7);
      state = Main;
    }
    break;

    case SelectViewing: {
      
      if (notCleared) { //changes flag once cleared
        lcd.clear();
        notCleared = false;
      }

      if (millis() - time > 1000) { //if 1s pass
          lcd.setBacklight(5);
          lcd.setCursor(0, 0);
          lcd.print("ID: F313427");
          lcd.setCursor(0, 1);
          lcd.print("FREERAM: ");
          lcd.setCursor(9, 1);
          lcd.print(freeMemory()); //gets free memory
        }
       uint8_t buttons = lcd.readButtons();
      if (!(buttons & BUTTON_SELECT)) {
        // Serial.println(F("DEBUG: Transferring to Main State"));
        lcd.clear();
        lcd.setBacklight(7);
        notCleared = true; //resets flag
        state = Main;
      }
      
    }
    break;
  } //END OF SWITCH
} //END OF LOOP


void syncronisation() {
  time = millis();
  lcd.setBacklight(5);

  while(true) { //Creates loop
    if (millis()-time>1000) { // Prints Q per sec 
        Serial.print(F("Q"));
        time = millis();
      }
    //Checks conditions for Input
    if (Serial.available()>0) { 
      String stopQ = Serial.readString(); //checks for input
        if (stopQ.substring(1)== "\n" || stopQ.substring(1)== "\r"){ // Checks for NL or CR
          Serial.println(F("\nERROR: No New Line or Carriage Return Allowed"));
      } else if (stopQ=="X" && stopQ.substring(1)!="\n" && stopQ.substring(1)!="\r") { //Checks for correct "X"
          break; //if input is X, leaves loop
      } 
    } 
  }

  Serial.print(F(" UDCHARS, SCROLL, FREERAM\n"));
  lcd.setBacklight(7); //Changes colour to white
}

//Split method
String* split(String command, String* commandArray) {
  int dashNum = 0;
  int lengthCommand = command.length();

  for (int i=0; i<command.length();i++) {
    if (command[i]=='-') {
      dashNum++; //finds number of dashes in string
    }
  }


  int count = 0;
  int lastPosition = 0;

  for (int i=0; i<command.length();i++) { //adds characters to array 
    if (command[i]!='-') {
      commandArray[count] += command[i];
    } else {
      count+=1;
      lastPosition = i+1;
    }
  }
  
  commandArray[count] = command.substring(lastPosition,lengthCommand); //adds last split section

  // for (int i=0; i<=count; i++) { //prints split (for DEBUG purpose)
  //   Serial.println("DEBUG: " + commandArray[i]);
  // }

  return commandArray;

}
