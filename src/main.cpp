#include <Arduino.h>
#include <FEHIO.h>
#include <FEHLCD.h>
#include <FEHUtility.h>
#include <FEHServo.h>
#include <FEHMotor.h>
#include <FEHRCS.h>
#include <FEHSD.h>

#define N 318 //Count of the IGWAN Motor
#define r 2.5/2 //Radius of Wheels
#define red 0.29 //Value for a red light with a red filter
#define blue 1.93 //Value for a blue light with a red filter
#define FULL_POWER 50.
#define TURN_POWER 40.
#define HALF_POWER 20.
#define SERVO_INIT 500
#define SERVO_FINAL 23858 
#define turn_count_90 245 //Count input needed to make a 90 degree turn
#define turn_count_45 100 //Count input needed to make a 45 degree turn
#define UP 0 //Starting degree value for falling servo arm
#define DOWN 90 //Starting degree value for falling servo arm
#define HALF 45.0
#define SERVO_MIN 500
#define SERVO_MAX 23858 
#define DOWN_Percentage 20  //Falling arm percentage for going down
#define UP_Percentage -30 //Falling arm percentage for going up
#define DOWN_Percentage_Lever -15  //Lever arm percentage for going down
#define UP_Percentage_Lever 50 //Lever arm percentage for going up
#define RCS_WAIT_TIME_IN_SEC 0.35 //RCS Delay Time
#define PULSE_TIME 0.17
#define PULSE_POWER 25
#define PULSE_COUNT 10 //Turn count for heading check
#define PLUS 0 // Orientation of AruCo Code
#define MINUS 1 // Orientation of AruCo Code


//Declare Motors, Encoders & CdS Cell
FEHMotor left_motor(FEHMotor::Motor0,9.0);
FEHMotor right_motor(FEHMotor::Motor3,9.0);
DigitalEncoder right_encoder(FEHIO::Pin8);
DigitalEncoder left_encoder(FEHIO::Pin9);
AnalogInputPin CdS_cell(FEHIO::Pin14);
FEHMotor falling_arm_motor(FEHMotor::Motor1, 9.0);
FEHMotor lever_arm_motor (FEHMotor::Motor2, 9.0);
FEHServo compost_rotator (FEHServo::Servo7);


int start(); 
void move_forward(int percent, float counts);
void turn_right(int percent, int counts);
void turn_left(int percent, int counts); 
float transitions_count (float s);
void compost_bin();
void turn_to_humidifier();
void move_falling_arm (int percent);
void lever_arm(int position);
void flip_fertilizer();
void RCS_heading_check ();
void check_x(float x_coordinate, int orientation);
void check_y(float x_coordinate, int orientation);
void pulse_forward(int percent, float seconds);
void pulse_counterclockwise(int percent, float seconds);
void RCS_heading_check (int heading_angle);

void ERCMain()
{
    //Apple Bucket to Bottom of Ramp
    move_forward(-FULL_POWER,(transitions_count(4)));
    move_forward(FULL_POWER,(transitions_count(18)));
    turn_left(TURN_POWER,turn_count_45);
    move_forward(FULL_POWER,(transitions_count(6.75)));
    lever_arm(UP);
    move_forward(-FULL_POWER,(transitions_count(6)));
    turn_left(TURN_POWER,(turn_count_90+turn_count_90));
    move_forward(FULL_POWER,(transitions_count(25)));
    turn_left(TURN_POWER,(turn_count_90+turn_count_45));

    //Bottom Of Ramp to Crate
    move_forward(FULL_POWER,(transitions_count(27)));
    turn_left(TURN_POWER,turn_count_90);
    move_forward(FULL_POWER,(transitions_count(6)));
    turn_right(TURN_POWER,turn_count_90);
    move_forward(FULL_POWER,(transitions_count(17.5)));
    lever_arm(DOWN);
    move_forward(-FULL_POWER,(transitions_count(17.5)));

    //Crate to Levers
    turn_left(TURN_POWER,turn_count_90);
    move_forward(-FULL_POWER,(transitions_count(4)));
    turn_left(TURN_POWER,turn_count_45);
    move_forward(FULL_POWER,(transitions_count(12)));



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
        if (((CdS <= (red+1)))||((TimeNow()-start_time)>=5))
        {
            i=1;
            return(1);
        }
    };
};
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

        LCD.WriteLine(left_encoder.Counts());
        LCD.WriteLine(right_encoder.Counts());
    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
};
float transitions_count (float s)//Calculate the number of transitions the encoder needs to count to tranvel a specified distance (s) in inches
{
    return ((s*N)/(2*PI*r));
};
void compost_bin(){ //rotate the compost bin from 0 to 300 degrees, wait one second before rotating it back to it's original position


    compost_rotator.SetDegree(0); 
    Sleep(3.0); //temporary guess
    compost_rotator.SetDegree(90);

    /*test run

    float rotateTime = 2.0; //this will need testing: (time it takes for 1 rev)*300/360

    compost_rotator.SetDegree(90); //set the initial point
    Sleep(0.5);

    //move 300 degrees
    compost_rotator.SetDegree(0); 
    Sleep(rotateTime); 

    //stopping point
    compost_rotator.SetDegree(90); 
    Sleep(1.0);

    //rotate back to original position
    compost_rotator.SetDegree(180); 
    Sleep(rotateTime);

    //stop at the original position
    compost_rotator.SetDegree(90); 

    Sleep(0.5);
    
    */
   
};
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
void lever_arm(int position){
    switch (position){
        case (UP):
        LCD.WriteLine("UP"); 
        lever_arm_motor.SetPercent(UP_Percentage_Lever);
        break;

        case (DOWN):
        lever_arm_motor.Stop();
        LCD.WriteLine("DOWN"); 
        lever_arm_motor.SetPercent(DOWN_Percentage_Lever);
        Sleep (0.35);
        lever_arm_motor.Stop();
        break;
    }
};
void flip_correct_lever(){
    int lever = RCS.GetLever();
    switch(lever){
        case(0):
        turn_left(TURN_POWER, 100);
        move_forward(FULL_POWER,(transitions_count(2)));
        lever_arm(UP);
        break;

        case(1):
        move_forward(FULL_POWER,(transitions_count(5)));
        lever_arm(UP);
        break;

        case(2):
        turn_right(TURN_POWER, 100);
        move_forward(FULL_POWER,(transitions_count(2)));
        lever_arm(UP);
        break;

    }
}
void move_falling_arm(int position)
{
    switch (position){
        case (UP):
        LCD.WriteLine("UP"); 
        falling_arm_motor.SetPercent(UP_Percentage);
        Sleep (0.3);
        lever_arm_motor.Stop();
        break;

        case (DOWN):
        LCD.WriteLine("DOWN"); 
        falling_arm_motor.SetPercent(DOWN_Percentage);
        Sleep (0.25);
        lever_arm_motor.Stop();
        break;
    }
};
void RCS_heading_check (int heading_angle){
    RCS.RequestPosition(false);
    Sleep(0.1);
    RCSPose* pose = RCS.Position();

    for (int i = 0; i < 10; i++) {
        if(pose->heading != heading_angle && (pose->heading < heading_angle - 1 || pose->heading > heading_angle + 1))
        {
            if(pose->heading > heading_angle + 1)
            {
                turn_right(PULSE_POWER,PULSE_COUNT);
            }
            else if(pose->heading < heading_angle - 1)
            {
                turn_left(PULSE_POWER,PULSE_COUNT);
            }
            Sleep(RCS_WAIT_TIME_IN_SEC);

            pose = RCS.RequestPosition();
        }
    }
};
void check_x(float x_coordinate, int orientation){
    
    // Determine the direction of the motors based on the orientation of the AruCo code 
    int power = PULSE_POWER;
    if(orientation == MINUS){
        power = -PULSE_POWER;
    }

    RCSPose* pose = RCS.RequestPosition();

    // Check if receiving proper RCS coordinates and whether the robot is within an acceptable range
    for (int i = 0; i < 10; i++) {
        if(pose->x != -1 && (pose->x < x_coordinate - 1 || pose->x > x_coordinate + 1))
        {
            if(pose->x > x_coordinate + 1)
            {
                // Pulse the motors for a short duration in the correct direction
                pulse_forward(-power, PULSE_TIME);
            }
            else if(pose->x < x_coordinate - 1)
            {
                // Pulse the motors for a short duration in the correct direction
                pulse_forward(power, PULSE_TIME);
            }
            Sleep(RCS_WAIT_TIME_IN_SEC);

            pose = RCS.RequestPosition();
        }
    }
};
void check_y(float y_coordinate, int orientation)
{
    // Determine the direction of the motors based on the orientation of the QR code
    int power = PULSE_POWER;
    if(orientation == MINUS){
        power = -PULSE_POWER;
    }

    RCSPose* pose = RCS.RequestPosition();

    // Check if receiving proper RCS coordinates and whether the robot is within an acceptable range
    for (int i = 0; i < 10; i++) {
        while(pose->y != -1 && (pose->y < y_coordinate - 1 || pose->y > y_coordinate + 1))
        {
            if(pose->y > (y_coordinate + 1))
            {
                // Pulse the motors for a short duration in the correct direction
                pulse_forward(-power, PULSE_TIME);
            }
            else if(pose->y < (y_coordinate - 1))
            {
                // Pulse the motors for a short duration in the correct direction
            pulse_forward(power, PULSE_TIME);
            }
            Sleep(RCS_WAIT_TIME_IN_SEC);
            
            pose = RCS.RequestPosition();
        }
    }   
};
void pulse_forward(int percent, float seconds) 
{
    // Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(percent);

    // Wait for the correct number of seconds
    Sleep(seconds);

    // Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}
void pulse_counterclockwise(int percent, float seconds) 
{
    // Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(-percent);

    // Wait for the correct number of seconds
    Sleep(seconds);

    // Turn off motors
    right_motor.Stop();
    left_motor.Stop();
};
