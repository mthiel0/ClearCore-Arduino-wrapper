/*
 * Title: FollowDigitalVelocity
 *
 * Objective:
 *    This example demonstrates control of the ClearPath-MC operational mode
 *    Follow Digital Velocity Command, Unipolar PWM Command.
 *
 * Description:
 *    This example enables a ClearPath motor and executes a repeating pattern of
 *    bidirectional velocity moves. During operation, various move statuses are
 *    written to the USB serial port.
 *    This example does not use HLFB for motor feedback. It is possible your
 *    commanded velocity is not reached before a new velocity is commanded.
 *    The resolution for PWM outputs is 8-bit, meaning only 256 discrete speeds
 *    can be commanded in each direction. The motor's actual commanded speed may
 *    differ from what you input below because of this.
 *    Consider using Manual Velocity Control mode if greater velocity command
 *    resolution is required, or if HLFB is needed for "move done/at speed"
 *    status feedback.
 *
 * Requirements:
 * 1. A ClearPath motor must be connected to Connector M-0.
 * 2. The connected ClearPath motor must be configured through the MSP software
 *    for Follow Digital Velocity Command, Unipolar PWM Command mode (In MSP
 *    select Mode>>Velocity>>Follow Digital Velocity Command, then with
 *    "Unipolar PWM Command" selected hit the OK button).
 * 3. The ClearPath motor must be set to use the HLFB mode "ASG-Velocity"
 *    through the MSP software (select Advanced>>High Level Feedback [Mode]...
 *    then choose "All Systems Go (ASG) - Velocity" from the dropdown and hit
 *    the OK button).
 * 4. The ClearPath must have a defined Max Speed configured through the MSP
 *    software (On the main MSP window fill in the "Max Speed (RPM)" box with
 *    your desired maximum speed). Ensure the value of maxSpeed below matches
 *    this Max Speed.
 * 5. Ensure the "Invert PWM Input" checkbox found on the MSP's main window is
 *    unchecked.
 * 6. Ensure the Input A filter in MSP is set to 20ms, (In MSP
 *    select Advanced>>Input A, B Filtering... then in the Settings box fill in
 *    the textbox labeled "Input A Filter Time Constant (msec)" then hit the OK
 *    button).
 *
 *
 * Links:
 * ** web link to doxygen (all Examples)
 * ** web link to ClearCore Manual (all Examples)  <<FUTURE links to Getting started webpage/ ClearCore videos>>
 * ** web link to ClearPath Operational mode video (Only ClearPath Examples)
 * ** web link to ClearPath manual (Only ClearPath Examples)
 *
 * Last Modified: 2/10/2020
 * Copyright (c) 2020 Teknic Inc. This work is free to use, copy and distribute under the terms of
 * the standard MIT permissive software license which can be found at https://opensource.org/licenses/MIT
 */

#include "ClearCore.h"

// Defines the motor's connector as ConnectorM0
#define motor ConnectorM0

// The INPUT_A_FILTER must match the Input A filter setting in MSP
// (Advanced >> Input A, B Filtering...)
#define INPUT_A_FILTER 20

// Select the baud rate to match the target device.
#define baudRate 9600

// This is the commanded speed limit (must match the MSP value). This speed
// cannot actually be commanded, so use something slightly higher than your real
// max speed here and in MSP.
double maxSpeed = 510;

// Declares our user-defined helper function, which is used to command speed and
// direction. The definition/implementation of this function is at the bottom of
// the sketch.
bool CommandVelocity(int commandedVelocity);

void setup() {
    // Put your setup code here, it will only run once:

    // Sets all motor connectors to the correct mode for Follow Digital
    // Velocity, Unipolar PWM mode.
    MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL,
                          Connector::CPM_MODE_A_DIRECT_B_PWM);

    // Sets up serial communication and waits up to 5 seconds for a port to open.
    // Serial communication is not required for this example to run.
    Serial.begin(baudRate);
    uint32_t timeout = 5000;
    uint32_t startTime = millis();
    while (!Serial && millis() - startTime < timeout) {
        continue;
    }

    // Enables the motor
    motor.EnableRequest(true);
    Serial.println("Motor Enabled");

    // Waits for HLFB to assert (waits for homing to complete if applicable)
    Serial.println("Waiting for HLFB...");
    while (motor.HlfbState() != MotorDriver::HLFB_ASSERTED) {
        continue;
    }
    Serial.println("Motor Ready");
}


void loop() {
    // Put your main code here, it will run repeatedly:

    // Move at +100 RPM (CCW).
    CommandVelocity(100);    // See below for the detailed function definition.
    // Wait 5000ms.
    delay(5000);

    CommandVelocity(300); // Move at +300 RPM (CCW).
    delay(5000);

    CommandVelocity(-500); // Move at -500 RPM (CW).
    delay(5000);

    CommandVelocity(-300); // Move at -300 RPM (CW).
    delay(5000);

    CommandVelocity(100); // Move at +100 RPM (CCW).
    delay(5000);
}

/*------------------------------------------------------------------------------
 * CommandVelocity
 *
 *    Command the motor to move using a velocity of commandedVelocity
 *    Prints the move status to the USB serial port
 *    Returns when HLFB asserts (indicating the motor has reached the commanded
 *    velocity)
 *
 * Parameters:
 *    int commandedVelocity  - The velocity to command
 *
 * Returns: True/False depending on whether the velocity was successfully
 * commanded.
 */
bool CommandVelocity(int commandedVelocity) {
    if (abs(commandedVelocity) > abs(maxSpeed)) {
        Serial.println("Move rejected, requested velocity over the limit.");
        return false;
    }
    Serial.print("Commanding velocity: ");
    Serial.println(commandedVelocity);

    // Change ClearPath's Input A state to change direction
    if (commandedVelocity > 0) {
        motor.MotorInAState(false);
    }
    else {
        motor.MotorInAState(true);
    }

    // Delays to send the correct filtered direction.
    delay(INPUT_A_FILTER);

    // Find the scaling factor of our velocity range mapped to the PWM duty
    // cycle range (255 is the max duty cycle).
    double scaleFactor = 255 / maxSpeed;

    // Scale the velocity command to our duty cycle range.
    int dutyRequest = abs(commandedVelocity) * scaleFactor;

    // Command the move.
    motor.MotorInBDuty(dutyRequest);

    Serial.println("Move Done");
    return true;
}
//------------------------------------------------------------------------------