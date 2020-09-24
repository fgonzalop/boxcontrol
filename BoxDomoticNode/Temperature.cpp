#include "Temperature.h"

unsigned long theTemperature = 20;

unsigned long AnswerTemperature(unsigned long aAction1, unsigned long aAction2)
{
   return theTemperature;
}

void PerformTemperature(unsigned long aAction1, unsigned long aAction2)
{
  theTemperature = 25;//TBD
}
