#include <Arduino.h>
#include <FEHIO.h>
#include <FEHLCD.h>
#include <FEHUtility.h>
#include <FEHServo.h>
#include <FEHMotor.h>

#define N 318 //Count of the IGWAN Motor
#define r 2.5/2 //Radius of Wheels
#define red 0.29 //Value for a red light with a red filter
#define blue 1.93 //Value for a blue light with a red filter
#define FULL_POWER 50.
#define TURN_POWER 40.
#define HALF_POWER 20.
#define SERVO_INIT 500
#define SERVO_FINAL 23858 
#define turn_count_90 255 //Count input needed to make a 90 degree turn
#define turn_count_45 125 //Count input needed to make a 25 degree turn
<<<<<<< HEAD
#define start_position 0 //Starting degree value for falling servo arm
#define end_position 90 //Starting degree value for falling servo arm
#define window_time 15 //Time for falling arm to be down
=======

//line following variables
#define RON 3.33 
#define ROFF .53 
#define LON 3.69 
#define LOFF 1.15 
#define MON 3.43 
#define MOFF 0.80 

>>>>>>> b8d018de52c6288b6599d30f0320fd2184088f86

//Declare Motors, Encoders & CdS Cell
FEHMotor left_motor(FEHMotor::Motor0,9.0);
FEHMotor right_motor(FEHMotor::Motor3,9.0);
DigitalEncoder right_encoder(FEHIO::Pin8);
DigitalEncoder left_encoder(FEHIO::Pin9);
AnalogInputPin CdS_cell(FEHIO::Pin14);
FEHServo servo_falling_arm(FEHServo::Servo6);


AnalogInputPin right_opto(FEHIO::Pin2); 
AnalogInputPin middle_opto(FEHIO::Pin3); 
AnalogInputPin left_opto(FEHIO::Pin4); 

int start(); 
void move_forward(int percent, float counts);
void turn_right(int percent, int counts);
void turn_left(int percent, int counts); 
float transitions_count (float s);
void compost_bin();
void turn_to_humidifier();
<<<<<<< HEAD
void drop_falling_arm ();
=======
void follow_line(float speed);
>>>>>>> b8d018de52c6288b6599d30f0320fd2184088f86

void ERCMain()
{ 
    int initiate=0;
    initiate = start();
    if (initiate==1)
    {
        LCD.WriteLine("initiated");
        move_forward(-FULL_POWER,(transitions_count(4)));
        turn_right(TURN_POWER,turn_count_45);
        move_forward(35.,(transitions_count(40.25)));
        turn_left(TURN_POWER,turn_count_90);
        move_forward(FULL_POWER,(transitions_count(14.5)));
        turn_to_humidifier();
        move_forward(-FULL_POWER,(transitions_count(18))); 
        turn_left(-TURN_POWER,turn_count_90);
        move_forward(-35.,(transitions_count(45)));
        turn_right(-TURN_POWER,turn_count_45);
    }

}
int start ()//Go after start light is detected to be ON or after 30 seconds
{
    int i=0;
    float start_time = TimeNow();
    LCD.WriteLine("Start Function");
    while (i==0)
    {
        float CdS = CdS_cell.Value();
        LCD.WriteLine(CdS);
        LCD.Clear();
        if ((CdS <= (red+1)))
        // ||((TimeNow()-start_time)>=5))
        {
            i=1;
            return(1);
        }
    };
}
void move_forward(int percent, float counts) //Drive Forward for a specified distance at a specified speed using encoders
{
    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    //Set both motors to desired percent
    right_motor.SetPercent(percent+2);
    left_motor.SetPercent(percent);


    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts);

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
};
void turn_right(int percent, int counts)//Turn Right for a specified distance at a specified speed using encoders
{
    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    //Set both motors to desired percent
    right_motor.SetPercent(-percent);
    left_motor.SetPercent(percent);

    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts);

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
};
void turn_left(int percent, int counts) //Turn Left for a specified distance at a specified speed using encoders
{
    //Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    //Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(-percent);

    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts);

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
};
float transitions_count (float s)//Calculate the number of transitions the encoder needs to count to tranvel a specified distance (s) in inches
{
    return ((s*N)/(2*PI*r));
};
//rotate the compost bin from 0 to 300 degrees, wait one second before rotating it back to it's original position
void compost_bin(){

    servo_arm.SetMin(SERVO_INIT); 
    servo_arm.SetMax(SERVO_FINAL); 

    int i=1; 
    while (i==1) { 
        servo_arm.SetDegree(0); 
        servo_arm.SetDegree(300);
        Sleep(1.0); 
        servo_arm.SetDegree(0);
    }; 
};
//turn to 
void turn_to_humidifier()
{
    int i=0;
    while (i==0){
    float CdS = CdS_cell.Value();
    LCD.WriteLine("CdS Value:");
    LCD.WriteLine(CdS);

    if (CdS <= (red + .82)) {
        LCD.WriteLine("Red Light Detected");
            turn_right(TURN_POWER, 50);
            move_forward(FULL_POWER,(transitions_count(2)));
            i=1;
            turn_right(TURN_POWER, 50);
        } 
    else if ((CdS > (blue - .82)) && (CdS < 2.5)) {
            LCD.WriteLine("Blue Light Detected");
            turn_left(TURN_POWER, 50);
            move_forward(FULL_POWER,(transitions_count(2)));
            i=1;
            turn_left(-TURN_POWER, 50);
        }
    else {
            LCD.WriteLine("No valid color detected");
            move_forward(FULL_POWER,(transitions_count(0.5)));
        }
    };
};
<<<<<<< HEAD
void drop_falling_arm()
{
    float StartTime=TimeNow;
    servo_falling_arm.SetDegree(start_position);  
    servo_falling_arm.SetDegree(end_position);
    while ((TimeNow - StartTime) < window_time) {};
    servo_falling_arm.SetDegree(start_position); 
=======

enum LineStates { 
    MIDDLE, 
    RIGHT, 
    LEFT,  
}; 

void follow_line(float speed){

    int state = MIDDLE;

    //read the value of the Optos and store in x 

    float right_val = right_opto.Value(); 
    float left_val = left_opto.Value(); 
    float middle_val = middle_opto.Value(); 

    while (true)  { // I will follow this line forever! 
        switch(state){ // If I am in the middle of the line... 

            // If the right sensor is on the line... 
            case RIGHT: 
                while(state==RIGHT) { 
                    // Set motor powers for right turn 
                    right_motor.SetPercent(speed*(8/5)); 
                    left_motor.SetPercent (speed*(4/5)); 

                    //Check Values 
                    right_val = right_opto.Value(); 
                    left_val = left_opto.Value(); 

                    if((right_val<=(ROFF+1))) { 
                        state=MIDDLE; 
                    } 
                }; 
                break; 

            // If the left sensor is on the line... 
            case LEFT: 
                while(state==LEFT) { 
                /* Mirror operation of RIGHT state */ 
                    right_motor.SetPercent(speed*(4/5)); 
                    left_motor.SetPercent(speed*(8/5)); 

                    //Check Values 
                    right_val = right_opto.Value(); 
                    left_val = left_opto.Value(); 

                    if((left_val<=(LOFF+1))) { 
                        state=MIDDLE; 
                    } 
                }; 
                break; 
            default: right_motor.Stop(); left_motor.Stop(); 
            break; 

        } 
        Sleep(1); 
    }
>>>>>>> b8d018de52c6288b6599d30f0320fd2184088f86
}