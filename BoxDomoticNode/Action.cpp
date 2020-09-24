#include "Action.h"
#include "Temperature.h"

unsigned long Answer(unsigned long aAction1, unsigned long aAction2)
{
  unsigned long aResult = 0;
  switch (aAction1)
  {
    case REQUEST_TEMPERATURE_ACTION:
       aResult = AnswerTemperature(aAction1, aAction2);
       break;
    default:
       break;
  }

	return aResult;
}

void Perform(unsigned long aAction1, unsigned long aAction2)
{
  switch (aAction1)
  {
    case REQUEST_TEMPERATURE_ACTION:
       PerformTemperature(aAction1, aAction2);
       break;
    default:
       break;
  }
}
