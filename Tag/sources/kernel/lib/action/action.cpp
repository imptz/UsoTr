#include "action.h"

bool Action::isDebugRegistered = false;
bool Action::debug = false;

void Action::onMessage(Message message)
{
	switch (message.msg)
	{
		case SerialDebug::SERIAL_DEBUG_MESSAGE_RECV_COMMAND:
			switch (message.par1)
			{
				case SerialDebug::COMMAND_DEBUG_ACTIONS_ON:
					if (!debug)
					{
						debug = true;
						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "Actions DEBUG ON\n");
					}
					break;
				case SerialDebug::COMMAND_DEBUG_ACTIONS_OFF:
					if (debug)
					{
						debug = false;
						DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "Actions DEBUG OFF\n");
					}
					break;
			}
			break;
	}
}
