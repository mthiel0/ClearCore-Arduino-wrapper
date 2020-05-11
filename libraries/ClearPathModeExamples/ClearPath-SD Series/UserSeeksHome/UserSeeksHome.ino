/*
 * Title: UserSeeksHome
 *
 * Objective:
 *    This example demonstrates control of the "User Seeks Home" homing feature
 *    of a ClearPath-SD motor (this feature is also available in ClearPath-MCPV
 *    Pulse Burst Positioning Mode, but this example is only for Step and
 *    Direction mode). "User Seeks Home" should be used when more flexilibty is
 *    required during a homing sequence (e.g. to move at multiple velocities,
 *    stopping to perform other tasks, manually exiting a homing sequence, or
 *    using sensor-based homing).
 *
 * Description:
 *    This example enables the motor, moves towards a hardstop at a velocity for
 *    2 seconds, then with a slower velocity until clamping into the hardstop.
 *    An offset position move is then commanded away from the hardstop to
 *    complete the homing sequence. Homing status is printed to the USB serial
 *    port.
 *
 * Requirements:
 * 1. A ClearPath motor must be connected to Connector M-0.
 * 2. The connected ClearPath motor must be configured through the MSP software
 *    for Step and Direction mode (In MSP select Mode>>Step and Direction).
 * 3. The ClearPath motor must be set to use the HLFB mode "ASG-Position"
 *    through the MSP software (select Advanced>>High Level Feedback [Mode]...
 *    then choose "All Systems Go (ASG) - Position" from the dropdown and hit
 *    the OK button).
 * 4. The ClearPath must have homing enabled and configured. To configure, look
 *    under the "Homing" label on the MSP's main window, check the "Enabled"
 *    radial button, then click the "Setup..." button. Set the Homing Style set
 *    to "User seeks home; ClearPath ASG signals when homing is complete" then
 *    hit the OK button.
 * 5. A hardstop for homing must be installed on your mechanics. Set the homing
 *    torque limit accordingly. This torque will be used to move toward and
 *    clamp up against the hardstop.
 *
 * ** IMPORTANT: This examples homes in the Positive (CCW) direction, assuming
 *    the hardstop is on the positive end of travel
 *
 * ** Note: Set the Input Resolution in MSP the same as your motor's Positioning
 *    Resolution spec if you'd like the pulses sent by ClearCore to command a
 *    move of the same number of Encoder Counts, a 1:1 ratio.
 *
 * Links:
 * ** web link to doxygen (all Examples)
 * ** web link to ClearCore Manual (all Examples)  <<FUTURE links to Getting started webpage/ ClearCore videos>>
 * ** web link to ClearPath Operational mode video (Only ClearPath Examples)
 * ** web link to ClearPath manual (Only ClearPath Examples)
 *
 * Last Modified: 1/21/2020
 * Copyright (c) 2020 Teknic Inc. This work is free to use, copy and distribute under the terms of
 * the standard MIT permissive software license which can be found at https://opensource.org/licenses/MIT
 */

#include "ClearCore.h"

// Specify which motor to move.
// Options are: ConnectorM0, ConnectorM1, ConnectorM2, or ConnectorM3.
#define motor ConnectorM0

// Selects the baud rate to match the target serial device
#define baudRate 9600

void setup() {
    // Put your setup code here, it will only run once:

    // Sets the input clocking rate. This normal rate is ideal for ClearPath
    // step and direction applications.
    MotorMgr.MotorInputClocking(MotorManager::CLOCK_RATE_NORMAL);

    // Sets all motor connectors into step and direction mode.
    MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL,
                          Connector::CPM_MODE_STEP_AND_DIR);

    // Sets the maximum velocity in step pulses/sec.
    motor.VelMax(10000);
    // Sets the maximum acceleration in step pulses/sec^2.
    motor.AccelMax(100000);

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

    // Commands a speed of 5000 pulses/sec towards the hardstop for 2 seconds
    Serial.println("Moving toward hardstop... Waiting for HLFB");
    motor.MoveVelocity(5000);
    delay(2000);
    // Then slows down to 1000 pulses/sec until clamping into the hard stop
    motor.MoveVelocity(1000);

    // Delay so HLFB has time to deassert
    delay(10);
    // Waits for HLFB to assert again, meaning the hardstop has been reached
    while (motor.HlfbState() != MotorDriver::HLFB_ASSERTED) {
        continue;
    }

    // Stop the velocity move now that the hardstop is reached
    motor.MoveStopAbrupt();

    // Move away from the hard stop. Any move away from the hardstop will
    // conclude the homing sequence.
    motor.Move(-1000);

    // Delay so HLFB has time to deassert
    delay(10);
    // Waits for HLFB to assert, meaning homing is complete
    Serial.println("Moving away from hardstop... Waiting for HLFB");
    while (motor.HlfbState() != MotorDriver::HLFB_ASSERTED) {
        continue;
    }
    Serial.println("Homing Complete. Motor Ready.");
}

void loop() {
    // This example doesn't have any ongoing tasks to perform.
}
