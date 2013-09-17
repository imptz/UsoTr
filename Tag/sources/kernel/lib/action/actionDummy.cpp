#include "actionDummy.h"
#include "actionStartProgramScanPoint.h"

ActionDummy::ActionDummy()
{
}

ActionDummy::~ActionDummy()
{
}

void ActionDummy::timerHandler()
{
}

void ActionDummy::step()
{
}

void ActionDummy::onMessage(Message message)
{
	switch (message.msg)
	{
		case SerialDebug::SERIAL_DEBUG_MESSAGE_RECV_COMMAND:
			switch (message.par1)
			{
				case SerialDebug::COMMAND_DEBUG_ACTIONS_NASADOK_ON_OFF:
					if (debug)
					{
						if (ActionStartProgramScanPoint::nasadokOnOff == SCAN_PROGRAM_FLAGS_NASADOK_OFF)
						{
							ActionStartProgramScanPoint::nasadokOnOff = SCAN_PROGRAM_FLAGS_NASADOK_ON;
							DEBUG_IF_PUT_METHOD("Движение насадка при работе по программе орошения: ");
							DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "включено\n");
						}
						else
						{
							ActionStartProgramScanPoint::nasadokOnOff = SCAN_PROGRAM_FLAGS_NASADOK_OFF;
							DEBUG_IF_PUT_METHOD("Движение насадка при работе по программе орошения: ");
							DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_RED, SerialDebug::TEXT_ATTR_BOLD, "выключено\n");
						}
					}
					break;
			}
			break;
	}

	Action::onMessage(message);
}
