

#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56

#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38



const float DEGREES_PER_STEP = 1.8;
const int MICROSTEPS_PER_STEP = 32;
const int SMALL_XZ_GEAR = 23;
const int LARGE_XZ_GEAR = 43;

const bool MINMAX_ON = true;
const float XY_ANGLE_MINIMUM = -400;
const float XY_ANGLE_MAXIMUM = 400;

const float XZ_ANGLE_MINIMUM = -11;
const float XZ_ANGLE_MAXIMUM = 60;

float calibration_xy_angle_offset = 0.0;
float calibration_xz_angle_offset = 0.0;

float xy_absolute_step_minimum = 1.0 * XY_ANGLE_MINIMUM / DEGREES_PER_STEP * MICROSTEPS_PER_STEP;
float xy_absolute_step_maximum = 1.0 * XY_ANGLE_MAXIMUM / DEGREES_PER_STEP * MICROSTEPS_PER_STEP;
float xz_absolute_step_minimum = 1.0 * XY_ANGLE_MINIMUM / DEGREES_PER_STEP * MICROSTEPS_PER_STEP  / SMALL_XZ_GEAR * LARGE_XZ_GEAR; ;
float xz_absolute_step_maximum = 1.0 * XY_ANGLE_MAXIMUM / DEGREES_PER_STEP * MICROSTEPS_PER_STEP  / SMALL_XZ_GEAR * LARGE_XZ_GEAR; ;

int x_direction = 1;
int y_direction = 1;
int x_count = 1;
int y_count = 100;

// desired is what the human wants, assuming that laser is calibrated

float desired_xy_angle = 0.0;
float desired_xz_angle = 0.0;

// raw is un-calibrated angle target
 
float raw_xy_angle = 0.0;
float raw_xz_angle = 0.0;

// raw target

int raw_xy_absolute_step = 0;
int raw_xz_absolute_step = 0;

bool minmax_is_okay;
bool stepper_is_moving;

String incoming;


#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper_xy(1, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepper_xz(1, Y_STEP_PIN, Y_DIR_PIN);



String getValue(String data, char separator, int index)
{
 int found = 0;
  int strIndex[] = {
0, -1  };
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
  if(data.charAt(i)==separator || i==maxIndex){
  found++;
  strIndex[0] = strIndex[1]+1;
  strIndex[1] = (i == maxIndex) ? i+1 : i;
  }
 }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}




void setup() {  


  // Change these to suit your stepper if you want
  stepper_xy.setMaxSpeed(750.0);
  stepper_xy.setAcceleration(5000.0);
  
  stepper_xy.setEnablePin(X_ENABLE_PIN); 
  
  stepper_xy.setPinsInverted(false, false, true);
  stepper_xy.enableOutputs();

  stepper_xy.moveTo(0);

  // Change these to suit your stepper if you want
  stepper_xz.setMaxSpeed(2000.0);
  stepper_xz.setAcceleration(4000.0);
  stepper_xz.setEnablePin(Y_ENABLE_PIN); 
  
  stepper_xz.setPinsInverted(false, false, true);
  stepper_xz.enableOutputs();

  stepper_xz.moveTo(0);

  Serial.begin(9600);

  

}

void loop() {

    
     if (Serial.available() > 0) {
      
        // read the incoming byte:
        incoming = Serial.readStringUntil('\n');
        
        Serial.print("I received: ");
        Serial.println(incoming);

        
        if (stepper_xy.distanceToGo() == 0 && stepper_xz.distanceToGo() == 0) {
            stepper_is_moving = false;
        }


        if(stepper_is_moving) {
          Serial.println("STEPPER IS STILL MOVING");
        } else { 
          
          
          if(incoming == "CALIBRATE") {
             
              calibration_xy_angle_offset = raw_xy_angle;
              calibration_xz_angle_offset = raw_xz_angle;
              Serial.println("CALIBRATED");
              Serial.print("XY OFFSET: "); Serial.println(calibration_xy_angle_offset);
              Serial.print("XZ OFFSET: "); Serial.println(calibration_xz_angle_offset);
              Serial.println(stepper_xy.targetPosition());
          } else {
      
            String xy_val = getValue(incoming, ',', 0);
            String xz_val = getValue(incoming, ',', 1);

            desired_xy_angle = xy_val.toFloat();
            desired_xz_angle = xz_val.toFloat();
        
            raw_xy_angle = (xy_val.toFloat() + calibration_xy_angle_offset);
            raw_xz_angle = (xz_val.toFloat() + calibration_xz_angle_offset);
    
            stepper_is_moving = true;
            minmax_is_okay = false;
    
            // calculate status
    
            
            if(MINMAX_ON == false) { 
                minmax_is_okay = true;
            } else if(desired_xy_angle >= XY_ANGLE_MINIMUM && desired_xy_angle <= XY_ANGLE_MAXIMUM 
                  && desired_xz_angle >= XZ_ANGLE_MINIMUM && desired_xz_angle <= XZ_ANGLE_MAXIMUM) { 
              minmax_is_okay = true;
            }
    
            if (minmax_is_okay == false) {
                Serial.println("ANGLES OUT OF BOUNDS");
            } else {
                Serial.print("DESIRED XY: "); Serial.println(desired_xy_angle);
                Serial.print("DESIRED YZ: "); Serial.println(desired_xz_angle);
            
                raw_xy_absolute_step = raw_xy_angle / DEGREES_PER_STEP * MICROSTEPS_PER_STEP;
                raw_xz_absolute_step = raw_xz_angle / DEGREES_PER_STEP * MICROSTEPS_PER_STEP / SMALL_XZ_GEAR * LARGE_XZ_GEAR; 
                
                Serial.print("DESIRED XY ABS STEP:");
                Serial.println(raw_xy_absolute_step);
                Serial.print("DESIRED YZ ABS STEP:");
                Serial.println(raw_xz_absolute_step);
            }
          }
        }
     }


    // If at the end of travel go to the other end
    if (stepper_xy.distanceToGo() == 0) {
      stepper_xy.moveTo(raw_xy_absolute_step);
    }
    
      //stepper_xy.run();
    // If at the end of travel go to the other end
    if (stepper_xz.distanceToGo() == 0) {
      stepper_xz.moveTo(raw_xz_absolute_step);
    }
    
   stepper_xy.run();
   stepper_xz.run();
   

}




