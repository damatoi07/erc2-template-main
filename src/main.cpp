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
#define turn_count_90 250 //Count input needed to make a 90 degree turn
#define turn_count_45 125 //Count input needed to make a 25 degree turn

//line following variables
#define RON 3.33 
#define ROFF .53 
#define LON 3.69 
#define LOFF 1.15 
#define MON 3.43 
#define MOFF 0.80 


//Declare Motors, Encoders & CdS Cell
FEHMotor left_motor(FEHMotor::Motor0,9.0);
FEHMotor right_motor(FEHMotor::Motor3,9.0);
DigitalEncoder right_encoder(FEHIO::Pin8);
DigitalEncoder left_encoder(FEHIO::Pin9);
AnalogInputPin CdS_cell(FEHIO::Pin14);
FEHServo servo_arm(FEHServo::Servo7);

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
void follow_line(float speed);

void ERCMain()
{ 
    int initiate=0;
    initiate = start();
    if (initiate==1)
    {
        LCD.WriteLine("initiated");
        move_forward(-FULL_POWER,(transitions_count(3)));
        turn_right(TURN_POWER,turn_count_45);
        move_forward(35.,(transitions_count(42)));
        turn_left(TURN_POWER,turn_count_90);
        move_forward(FULL_POWER,(transitions_count(16)));
        turn_to_humidifier();
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
         Sleep(2.0);
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

    if (CdS <= (red + 1)) {
        LCD.WriteLine("Red Light Detected");
            move_forward(FULL_POWER,(transitions_count(2)));
            turn_right(TURN_POWER, 100);
            move_forward(FULL_POWER,(transitions_count(3)));
            i=1;
        } 
    else if (CdS >= (blue - 1)) {
            LCD.WriteLine("Blue Light Detected");
            move_forward(FULL_POWER,(transitions_count(2)));
            turn_left(TURN_POWER, 100);
            move_forward(FULL_POWER,(transitions_count(3)));
            i=1;
        }
    else {
            LCD.WriteLine("No valid color detected");
            i=1;
        }
    };
};

enum LineStates { 
    MIDDLE, 
    RIGHT, 
    LEFT,  
}; 

void follow_line(float speed){

    while (true){

        float right_val = right_opto.Value(); 
        float left_val = left_opto.Value(); 

        // Adjust based on your threshold
        bool right_on = (right_val > ROFF);
        bool left_on  = (left_val  > LOFF);

        //Turn Left
        if (left_on && !right_on){
            right_motor.SetPercent(speed);
            left_motor.SetPercent(speed * 0.5);
        }
        //Turn right
        else if (right_on && !left_on){
            right_motor.SetPercent(speed * 0.5);
            left_motor.SetPercent(speed);
        }
        //Go straight
        else if (!left_on && !right_on){
            right_motor.SetPercent(speed);
            left_motor.SetPercent(speed);
        }
        else {
            right_motor.Stop();
            left_motor.Stop();
        }

        Sleep(1);
    }
}