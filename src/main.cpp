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
#define FULL_POWER 40. 
#define RAMP_POWER 65. 
#define TURN_POWER 40.
#define HALF_POWER 20.
#define turn_count_90 230//Count input needed to make a 90 degree turn
#define turn_count_45 105 //Count input needed to make a 45 degree turn
#define UP 0 //Starting degree value for falling arm
#define DOWN 90 //Starting degree value for falling arm
#define ON 3 //Value for case switch with falling arm
// #define SERVO_MIN 500
// #define SERVO_MAX 23858 
#define DOWN_Percentage 20  //Falling arm percentage for going down
#define UP_Percentage -35 //Falling arm percentage for going up
#define DOWN_Percentage_Lever -30  //Lever arm percentage for going down
#define UP_Percentage_Lever 45 //Lever arm percentage for going up
#define RCS_WAIT_TIME_IN_SEC 1.0 //RCS Delay Time
#define PULSE_TIME 0.17 //Time to pulse towards a direction to make corrections in position
#define PULSE_POWER 25 //Power to the motors when pulsing towards a direction to make corrections in position
#define PULSE_COUNT 10 //Turn count for heading check
#define FORWARDS 0 //Orientation of AruCo Code
#define BACKWARDS 1 //Orientation of AruCo Code


//Declare Motors, Encoders & CdS Cell0
FEHMotor left_motor(FEHMotor::Motor0,9.0); //IGWAN
FEHMotor right_motor(FEHMotor::Motor3,9.0); //IGWAN
DigitalEncoder right_encoder(FEHIO::Pin8); 
DigitalEncoder left_encoder(FEHIO::Pin9); 
AnalogInputPin CdS_cell(FEHIO::Pin14);
FEHServo compost_rotator (FEHServo::Servo7);
FEHMotor falling_arm_motor(FEHMotor::Motor1, 9.0); //TT Motor
FEHMotor lever_arm_motor (FEHMotor::Motor2, 9.0); //TT Motor


int start(); 
void move_forward(int percent, float counts);
void up_ramp (int percent, float counts);
void turn_right(int percent, int counts);
void turn_left(int percent, int counts); 
float transitions_count (float s);
void compost_bin();
int check_humidifier();
void turn_to_humidifier (int light_color);
void move_falling_arm (int percent);
void lever_arm(int position);
void lever_arm_start();
void flip_fertilizer();
void RCS_heading_check ();
void check_x(float x_coordinate, int orientation);
void check_y(float x_coordinate, int orientation);
void pulse_forward(int percent, float seconds);
void RCS_heading_check (float heading_angle);

void ERCMain()
{
    //initialize the RCS
    RCS.InitializeTouchMenu("1130D7LFS");

    //Store Coordinate values to RCS
    int touch_x,touch_y;
    float light_x, light_y, B_x, B_y, C_x, C_y, D_x, D_y;
    float light_heading, B_heading, C_heading, D_heading;

    LCD.WriteLine("RCS & Data Logging");
    LCD.WriteLine("Press Screen To Start");
    while(!LCD.Touch(&touch_x,&touch_y));
    while(LCD.Touch(&touch_x,&touch_y));

    FEHFile* fptr = SD.FOpen("RCS_TEST.txt", "r");
    
    SD.FScanf(fptr, "%f%f", &light_x, &light_y);
    // SD.FScanf(fptr, "%f%f", &B_x, &B_y);
    // SD.FScanf(fptr, "%f%f", &C_x, &C_y);
    // SD.FScanf(fptr, "%f%f", &D_x, &D_y);

    SD.FClose(fptr);

    WaitForFinalAction();

    int initiate=start();
    if (initiate==1){

    //Apple Bucket to Top of Ramp
    move_forward(-FULL_POWER,(transitions_count(4)));
    move_forward(FULL_POWER,(transitions_count(18)));
    turn_left(TURN_POWER,turn_count_45);
    lever_arm_start();
    RCS_heading_check(180.0);
    Sleep(3.0);
    move_forward(FULL_POWER,(transitions_count(9)));
    lever_arm(UP);
    move_forward(-FULL_POWER,(transitions_count(9)));
    turn_right(TURN_POWER,turn_count_45);
    move_forward(-FULL_POWER,(transitions_count(8)));
    turn_left(TURN_POWER,turn_count_45);
    move_forward(-FULL_POWER,(transitions_count(10)));
    move_forward(FULL_POWER,(transitions_count(2.25)));
    turn_right(TURN_POWER,turn_count_90);
    move_falling_arm(ON);
    move_forward(RAMP_POWER,(transitions_count(45)));

    //Wall Alignment
    move_forward(-FULL_POWER,(transitions_count(8)));
    turn_left(TURN_POWER,(turn_count_45/2));
    move_forward(FULL_POWER,(transitions_count(8)));
    turn_left(TURN_POWER,(turn_count_45+(turn_count_45/2)));
    move_forward(-FULL_POWER,(transitions_count(10)));

    //Wall to Crate
    move_forward(FULL_POWER,(transitions_count(6)));
    turn_right(TURN_POWER,turn_count_90);
    move_forward(FULL_POWER,(transitions_count(20)));
    lever_arm(DOWN);
    
    //Crate to Humidifier Light
    check_x(light_x, BACKWARDS);
    // move_forward(-FULL_POWER,(transitions_count(14.5)));
    turn_left(TURN_POWER,turn_count_90);
    lever_arm(UP);
    RCS_heading_check(180.0);
    Sleep(3.0);
    move_forward(FULL_POWER,(transitions_count(10)));
    Sleep(3.0); //Check Humidifier Light Position 
    move_forward(FULL_POWER,(transitions_count(12)));
    Sleep(5.0); //Check Humidifier Light Position 
    int light_color = check_humidifier();
    move_forward(-FULL_POWER,(transitions_count(1.5)));
    turn_to_humidifier(light_color); //Test Code with Lever Arm
    lever_arm(DOWN);
    move_falling_arm(DOWN);

    // //Humidifier Light to Levers
    // move_forward(-FULL_POWER,(transitions_count(15)));
    // move_falling_arm(UP);
    // turn_left(TURN_POWER,turn_count_45);
    // move_forward(FULL_POWER,(transitions_count(17)));
    // Sleep(3.0); //Check position for levers
    // lever_arm(DOWN);
    // move_forward(-FULL_POWER,(transitions_count(5))); 
    // move_forward(FULL_POWER,(transitions_count(5))); 
    // lever_arm(UP);

    // //Levers to Top of Ramp
    // move_forward(-FULL_POWER,(transitions_count(28)));
    // turn_right(TURN_POWER,turn_count_45);
    // move_forward(-FULL_POWER,(transitions_count(45)));

    // //Bottom of Ramp to Compost
    // turn_right(TURN_POWER,turn_count_45); 
    // move_forward(-FULL_POWER,(transitions_count(15)));
    // turn_left(TURN_POWER,turn_count_45);
    // move_forward(-FULL_POWER,(transitions_count(5)));
    // move_forward(FULL_POWER,(transitions_count(5)));
    // turn_right(TURN_POWER,turn_count_90); 
    // move_forward(-FULL_POWER,(transitions_count(5)));
    // Sleep(3.0); //Check compost position
    // compost_bin();
    }
    
   
};

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
    right_motor.SetPercent(percent);
    left_motor.SetPercent(percent);


    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts);

        LCD.WriteLine(left_encoder.Counts());
        LCD.WriteLine(right_encoder.Counts());

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
float transitions_count (float s)//Calculate the number of transitions the encoder needs to count to travel a specified distance (s) in inches
{
    return ((s*N)/(2*PI*r));
};
void compost_bin(){ //rotate the compost bin from 0 to 300 degrees, wait one second before rotating it back to it's original position


    float rotateTime = 2.0; //this will need testing: (time it takes for 1 rev)*300/360
    Sleep(0.5);

    //rotate clockwise
    compost_rotator.SetDegree(180); 
    Sleep(rotateTime);

    //rotate counterclockwise
    compost_rotator.SetDegree(0); 
    Sleep(rotateTime); 

    Sleep(0.5);
   
};
int check_humidifier(){
    
    int i=0;
    while (i==0){
    float CdS = CdS_cell.Value();
    LCD.WriteLine("CdS Value:");
    LCD.WriteLine(CdS);

    if (CdS <= (red + .82)) {
        LCD.WriteLine("Red Light Detected");
        return(i=0);
        } 
    else if ((CdS > (blue - .82)) && (CdS < 2.5)) {
        LCD.WriteLine("Blue Light Detected");
        return(i=1);
        }
    else {
        LCD.WriteLine("No valid color detected");
        move_forward(HALF_POWER,(transitions_count(0.5)));
        }
    }
}
void turn_to_humidifier(int light)
{
    int i=0;
    while (i==0){
    float CdS = CdS_cell.Value();
    move_forward(-FULL_POWER,(transitions_count(3)));
    lever_arm(DOWN);

    if (CdS <= (red + .82)) {
        LCD.WriteLine("Red Light Detected");
            turn_right(TURN_POWER, 95);
            move_forward(FULL_POWER,(transitions_count(2)));
            i=1;
            turn_right(TURN_POWER, 50);
        } 
    else if ((CdS > (blue - .82)) && (CdS < 2.5)) {
            LCD.WriteLine("Blue Light Detected");
            turn_left(TURN_POWER, turn_count_45);
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
        Sleep (2.0);
        lever_arm_motor.Stop();
        break;
    }
};
void lever_arm_start(){
        LCD.WriteLine("DOWN"); 
        lever_arm_motor.SetPercent(-10);
        lever_arm_motor.SetPercent(-10);
        Sleep (0.55);
        lever_arm_motor.Stop();
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
        falling_arm_motor.SetPercent(UP_Percentage);
        Sleep (0.3);
        lever_arm_motor.Stop();
        break;

        case (DOWN):
        LCD.WriteLine("DOWN"); 
        falling_arm_motor.SetPercent(DOWN_Percentage);
        falling_arm_motor.SetPercent(DOWN_Percentage);
        Sleep (0.25);
        lever_arm_motor.Stop();
        break;

        case (ON):
        LCD.WriteLine("ON & UP"); 
        lever_arm_motor.SetPercent(UP_Percentage_Lever);
        break;
    }
};
void RCS_heading_check (float heading_angle){
    RCS.RequestPosition(false);
    Sleep(1.0);
    RCSPose* pose = RCS.Position();
    int i = 0;
    int RIGHT_COUNT = PULSE_COUNT;
     int LEFT_COUNT = PULSE_COUNT;

    LCD.Clear();
    LCD.WriteLine(pose->heading);

        while(pose->heading != heading_angle && ((heading_angle - 1) < pose->heading < (heading_angle + 1)) && i !=5)
        {
            if(pose->heading > heading_angle + 1)
            {
                turn_right(PULSE_POWER,RIGHT_COUNT);
                LEFT_COUNT=PULSE_COUNT-3;
            }
            else if(pose->heading < heading_angle - 1)
            {
                turn_left(PULSE_POWER,LEFT_COUNT);
                RIGHT_COUNT=PULSE_COUNT-3;
            }
            Sleep(RCS_WAIT_TIME_IN_SEC);

            pose = RCS.RequestPosition();
            i++;

            LCD.WriteLine(pose->heading);

            //Add if it pulses in one direction it pulses less in the other
    }
};
void check_x(float x_coordinate, int orientation){
    
    // Determine the direction of the motors based on the orientation of the AruCo code 
    int power = PULSE_POWER;
    if(orientation == BACKWARDS){
        power = -PULSE_POWER;
    }

    int i=0;

    RCSPose* pose = RCS.RequestPosition();

    // Check if receiving proper RCS coordinates and whether the robot is within an acceptable range
        if(pose->x != -1 && ((x_coordinate + 1) < pose->x < (x_coordinate - 1)) && i !=5)
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

            i++;
        }
};
void check_y(float y_coordinate, int orientation)
{
    // Determine the direction of the motors based on the orientation of the QR code
    int power = PULSE_POWER;
    if(orientation == BACKWARDS){
        power = -PULSE_POWER;
    }

    int i=0;


    RCSPose* pose = RCS.RequestPosition();

    // Check if receiving proper RCS coordinates and whether the robot is within an acceptable range
        while(pose->y != -1 && ((y_coordinate + 1) < pose->y < (y_coordinate - 1)) && i !=5)
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

            i++;
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