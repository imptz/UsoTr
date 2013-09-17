#include "actionGetNetworkControllerInfo.h"
#include "../extension/subsystems/rpk/rpkSubsystem.h"

ActionGetNetworkControllerInfo::ActionGetNetworkControllerInfo()
	:	phase(PHASE_COMMAND)
{
	DEBUG_IF_PUT_METHOD("\n")
}

ActionGetNetworkControllerInfo::~ActionGetNetworkControllerInfo()
{
	DEBUG_IF_PUT_METHOD("\n")
}

void ActionGetNetworkControllerInfo::timerHandler()
{
	if (timeCounter < TEST_TIME_OUT)
		timeCounter++;
}

void ActionGetNetworkControllerInfo::step()
{
	unsigned char* pFrame = nullptr;
	unsigned char frame[20];

	IRpkDevice::FRAME_RESULT result;

	switch (phase)
	{
		case PHASE_COMMAND:
			frame[0] = NETWORK_CONTROLLER_ADDRESS;
			frame[1] = 0;
			frame[2] = RPK_COMMANDS_GET_NETWORK_CONTROLLER_INFO;
			frame[3] = 0;

			DEBUG_IF_PUT_METHOD("frame = %u %u %u %u", frame[0], frame[1], frame[2], frame[3])

			frameId = RpkSubsystem::getSingleton().write(frame);
			if (frameId == IRpkDevice::BAD_FRAME_ID)
			{
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " BAD_FRAME_ID\n")
				error();
			}
			else
			{
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, " done\n")
				phase = PHASE_COMMAND_WAIT;
			}
			break;
		case PHASE_COMMAND_WAIT:
			result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
			
			switch (result)
			{
				case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
					DEBUG_IF_PUT_METHOD("PHASE_COMMAND_WAIT result = ")
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, "FRAME_RESULT_ID_NOT_FOUND\n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_ERROR:
					DEBUG_IF_PUT_METHOD("PHASE_COMMAND_WAIT result = ")
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, "FRAME_RESULT_ERROR\n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_READY:
					DEBUG_IF_PUT_METHOD("PHASE_COMMAND_WAIT result = ")
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, "FRAME_RESULT_READY\n")
					
					DEBUG_IF_PUT_METHOD("frame = %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u \n", pFrame[0], pFrame[1], pFrame[2], pFrame[3], pFrame[4], pFrame[5], pFrame[6], pFrame[7], pFrame[8], pFrame[9], pFrame[10], pFrame[11], pFrame[12], pFrame[13], pFrame[14], pFrame[15], pFrame[16], pFrame[17], pFrame[18], pFrame[12])
					
					finish();
					break;
			}
			break;
	}

	SAFE_DELETE_ARRAY(pFrame)
}

void ActionGetNetworkControllerInfo::error()
{
	state = STATE_ERROR;
	phase = PHASE_STOP;
}

void ActionGetNetworkControllerInfo::finish()
{
	state = STATE_READY;
	phase = PHASE_STOP;
}
