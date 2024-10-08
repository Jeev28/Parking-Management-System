# Parking Management System
#### Arduino Development - Grade: 89%

## Table of Contents
- [Summary](#summary)
- [Specification](#specification)
- [Finite State Machine](#finite-state-machine)
- [Installation](#installation)
- [Usage](#usage)

  
## Summary
An Arduino LCD-based parking management system that reads custom input over the Serial Interface, processes them, and then displays the result on screen. The Arduino's buttons allow for interaction with the information displayed on the LCD. There are strict rules with how details are displayed on the screen and the format on valid inputs.


## Specification
**Input Restrictions:**
- Registration Numbers are made up of 7 alphanumeric characters in the format: 2 letters from A-Z; 2 numbers in range 0-9; and 3 letters in range A-Z. Spaces are not allowed.
- Each vehicle has a single letter denoting its type (C: Car, M: Motorcycle, V: Van, L: Lorry, B: Bus).
- Each vehicle has a location text to indicate the name of the parking lot. This is a string between 1-11 alphanumeric characters. Spaces are not allowed.
- Each vehicle has a entry timestamp to indicate when they entered the lot, in the format HHMM. This is done automatically based on how long the Arduino has been running.
- Vehicles that have paid the parking fee are given an exit timestamp at the time of payment.
- Each vehicle has a payment indicator (Paid or Not Paid). If they have paid, the display colour is green, otherwise yellow.

**Arduino Display Restrictions:**
- The LCD display has two lines and shows the information of a single vehicle providing an up-down scrolling facility to see others.
- Display of the format: <br>
 <img width="174" alt="Screenshot 2024-10-08 at 18 32 38" src="https://github.com/user-attachments/assets/00553b92-23fe-4da0-87f9-741f5875aa12">

Note: ^ and v are up and down arrows. ABCDEFG is the reg num, HIJKLMN is the location (and this may flow over), S is the vehicle type, TUV is the payment status and HHMM are entry/payment times. 
- Up and Down buttons move up or down through the vehicles list in time of arrival order. On reaching the first or last vehicle, the ^ or v is removed to indicate there are no more vehicles to display.

**Main Functions:**
The input message will start with a defining character:
1. **A** - indicates adding a new vehicle that entered. In the format A-RegNum-VehicleType-Location. If the user sends an "A" message for an existing registration number, the new vehicle type or location is used, only if the payment status is paid.
2. **S** - Indicates a payment status for the vehicle. In the format S-RegNum-PaymentStatus. Default payment status is NPD. When updating to PD, the time of payment is saved and displayed. If a vehicle's payment is changed from PD to NPD, the payment timestamp is removed, and the entry timestamp is changed to the time of current message receipt.
3. **T** - Indicates changing vehicle type, only if payment status is PD. In the format T-RegNum-NewType. 
4. **L** - Indicates changing the vehicle's parking location, only if payment status is PD.
5. **R** - Indicates removing an existing vehicle, only if the registration number exists and the payment status is PD. In the format R-RegNum.

**Additional Features**
1. UDCHARS - Implemented better looking arrows
2. FREERAM - Read the free SRAM in the Arduino.
3. SCROLL - If a vehicle's location is longer than 7 characters, the location scrolls to the left at 2 characters per second, returning to the start when the full location has been displayed.

  
## Finite State Machine
<img width="591" alt="Screenshot 2024-10-08 at 18 55 27" src="https://github.com/user-attachments/assets/35b63677-a72c-4863-9153-77384d2ad7c9">

##### You may need to enlarge your screen or download this image to view properly


## Installation
You will require:
- Arduino IDE installed on your computer
- Arduino Uno connected to a 16x2 LCD Display with 5 buttons.
The following libraries are required:
```
– Wire.h
– Adafruit\_RGBLCDShield.h
– TimeLib.h
– TimerOne.h
– MemoryFree.h
```

To install and run this Java project, follow the instructions below:
1. **Clone this repository (or download the ZIP file)**:
   ```bash
   git clone https://github.com/Jeev28/Parking-Management-System.git```
   
2. **Open in Arduino IDE**:
   Open the ```.ino``` file in the project folder to open teh sketch in the IDE.

3. **Connect Arduino UNO**
   Connect your Arduino Uno to your computer and choose it as an output source. Use the       Serial Monitor as input source.

## Usage
1. Input a upper case "X" to stop initialisation.
2. To navigate through the list of vehicles, use the up and down buttons.
3. To reset the Arduino, press the reset button. If you press and hold this button, a purple screen displays developer details and additional features implemented.

Try the following commands. Some will return errors which will be dealt with on the command line, and no impact is made on the LCD in that case.
1. A-GR04XFB-C-GranbySt.
2. A-GF57XWD-B-BeehiveLane
3. A-BD51SMR-V-BrownsLane
4. S-GR04XFB-PD
5. T-GR04XFB-B
6. L-GF57XWD-Southfield (ERROR)
7. S-GF57XWD-1 (ERROR)
8. L-CU57ABC-BeehiveLane (ERROR)
9. L-GR04XFB-BrownsLane
10. S-BD51SMR-PD
11. L-BD51SMR-BrownsLane (ERROR)
12. A-CU57ABC-C-BedfordSquare (ERROR)
13. A-CU57ABC-BedfordSq. (ERROR)
14. A-CU57ABC-C-BedfordSquare (ERROR)
15. A-CU57ABC-C- (ERROR)
16. A-CU57ABC-M-BedfordSq.
17. A-GR04XFB-B-BrownsLane (ERROR)
18. Z-GR04XFB-BrownsLane (ERROR)
19. ACU57ABC-V-BrownsLane (ERROR)
20. A-GF23WSN- L- BrownsLane

   
