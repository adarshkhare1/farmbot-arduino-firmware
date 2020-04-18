
/*
 * F12Handler.cpp
 *
 *  Created on: 2014/07/21
 *      Author: MattLech
 */

#include "F12Handler.h"

static F12Handler *instance;

F12Handler *F12Handler::getInstance()
{
  if (!instance)
  {
    instance = new F12Handler();
  };
  return instance;
};

F12Handler::F12Handler()
{
}

int F12Handler::execute(Command *command)
{

  if (LOGGING)
  {
    Serial.print("R99 HOME Y\r\n");
  }

  int homeIsUp = ParameterList::getInstance()->getValue(MOVEMENT_HOME_UP_Y);
  int stepsPerMM = ParameterList::getInstance()->getValue(MOVEMENT_STEP_PER_MM_Y);
  int missedStepsMax = ParameterList::getInstance()->getValue(ENCODER_MISSED_STEPS_MAX_Y);

  if (stepsPerMM <= 0)
  {
    missedStepsMax = 0;
    stepsPerMM = 1;
  }

  int A = 10; // move away coordinates
  int execution;
  bool emergencyStop;
  bool homingComplete = false;

  if (homeIsUp == 1)
  {
    A = -A;
  }

  int stepNr;

  long X = CurrentState::getInstance()->getX();
  long Y = CurrentState::getInstance()->getY();
  long Z = CurrentState::getInstance()->getZ();

  bool firstMove = true;
  int goodConsecutiveHomings = 0;

  // After the first home, keep moving away and home back
  // until there is no deviation in positions
  while (!homingComplete)
  {

    if (firstMove)
    {
      firstMove = false;

      // Move to home position. s
      Movement::getInstance()->moveToCoords(X, 0, Z, 0, 0, 0, false, false, false);
      //execution = CurrentState::getInstance()->getLastError();
      execution = 0;
      emergencyStop = CurrentState::getInstance()->isEmergencyStop();
      if (emergencyStop || execution != 0) { homingComplete = true; }
    }

    // Move away from the home position
    //posBeforeVerify = CurrentState::getInstance()->getY();
    Movement::getInstance()->moveToCoords(X, A, Z, 0, 0, 0, false, false, false);
    //execution = CurrentState::getInstance()->getLastError();
    execution = 0;
    emergencyStop = CurrentState::getInstance()->isEmergencyStop();
    if (emergencyStop || execution != 0) { break; }

    // Home again
    Movement::getInstance()->moveToCoords(X, 0, Z, 0, 0, 0, false, true, false);
    //execution = CurrentState::getInstance()->getLastError();
    execution = 0;
    emergencyStop = CurrentState::getInstance()->isEmergencyStop();
    if (emergencyStop || execution != 0) { break; }

    Serial.print("R99 homing displaced");
    Serial.print(" X ");
    Serial.print(CurrentState::getInstance()->getHomeMissedStepsX());
    Serial.print(" / ");
    Serial.print(CurrentState::getInstance()->getHomeMissedStepsXscaled());
    Serial.print(" Y ");
    Serial.print(CurrentState::getInstance()->getHomeMissedStepsY());
    Serial.print(" / ");
    Serial.print(CurrentState::getInstance()->getHomeMissedStepsYscaled());
    Serial.print(" Z ");
    Serial.print(CurrentState::getInstance()->getHomeMissedStepsZ());
    Serial.print(" / ");
    Serial.print(CurrentState::getInstance()->getHomeMissedStepsZscaled());
    Serial.print("\r\n");

    // Compare postition before and after verify homing, accounting for missed steps detecting stall
    if (CurrentState::getInstance()->getHomeMissedStepsYscaled() < (5 + missedStepsMax / stepsPerMM))
    {
      goodConsecutiveHomings++;
      if (goodConsecutiveHomings >= 3)
      {
        homingComplete = true;
        CurrentState::getInstance()->setY(0);
      }      
    }
    else
    {
      goodConsecutiveHomings = 0;
    }
  }

  if (LOGGING)
  {
    CurrentState::getInstance()->print();
  }
  return 0;
}
