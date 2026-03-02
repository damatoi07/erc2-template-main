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
#define FULL_POWER 40.
#define HALF_POWER 20.

//Declare Motors, Encoders & CdS Cell
FEHMotor left_motor(FEHMotor::Motor0,9.0);
FEHMotor right_motor(FEHMotor::Motor3,9.0);
DigitalEncoder right_encoder(FEHIO::Pin8);
DigitalEncoder left_encoder(FEHIO::Pin9);
AnalogInputPin CdS_cell(FEHIO::Pin3);

void start(); 
void move_forward(int percent, int counts);
void turn_right(int percent, int counts);
void turn_left(int percent, int counts); 
int transitions_count (float s); 

void ERCMain()
{
    move_forward(FULL_POWER,(transitions_count(5)));
    TestGUI();
}
void start ()//Go after start light is detected to be ON or after 30 seconds
{
    int i=1;
    float start_time = TimeNow();
    float CdS = CdS_cell.Value();
    while ( (i==1) || ((TimeNow() - start_time) <= TimeNow()))
    {
        float Cds = CdS_cell.Value();
        if ((CdS <= (red+1))||(TimeNow()-start_time)>=30)
        {
            i=0;
            //Start Composter Funtcion
        }
    }
};
void move_forward(int percent, int counts) //Drive Forward for a specified distance at a specified speed using encoders
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
    right_motor.SetPercent(percent);
    left_motor.SetPercent(-percent);

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
    right_motor.SetPercent(-percent);
    left_motor.SetPercent(percent);

    //While the average of the left and right encoder is less than counts,
    //keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts);

    //Turn off motors
    right_motor.Stop();
    left_motor.Stop();
};
int transitions_count (float s)//Calculate the number of transitions the encoder needs to count to tranvel a specified distance (s) in inches
{
    return ((s*N)/(2*PI*r));
}
