#include "actionStopProgramScan.h"
#include "../extension/subsystems/rpk/rpkSubsystem.h"
#include "../log/Log.h"

ActionStopProgramScan::ActionStopProgramScan()
{
}

ActionStopProgramScan::ActionStopProgramScan(unsigned char _deviceAddress)
	:	Action(_deviceAddress), phase(PHASE_COMMAND)
{
	DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__);
	DEBUG_IF_PUT("deviceAddress = %u\n", _deviceAddress);
}

ActionStopProgramScan::~ActionStopProgramScan()
{
	DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__);
	DEBUG_IF_PUT("deviceAddress = %u\n", deviceAddress);
}

void ActionStopProgramScan::timerHandler()
{
}

void ActionStopProgramScan::step()
{
	unsigned char* pFrame = nullptr;
	unsigned char frame[20];

	IRpkDevice::FRAME_RESULT result;

	switch (phase)
	{
		case PHASE_COMMAND:
			frame [0] = deviceAddress;
			frame [1] = 0;
			frame [2] = RPK_COMMANDS_STOP_SCAN_PROGRAM;
			frame [3] = 0;
			frame [4] = 0;

			DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_COMMAND frame = %u %u %u %u %u", deviceAddress, frame[0], frame[1], frame[2], frame[3], frame[4])

			frameId = RpkSubsystem::getSingleton().write(frame);
			if (frameId == IRpkDevice::BAD_FRAME_ID)
			{
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " BAD_FRAME_ID\n");
				error();
			}
			else
			{
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, " done\n");
				phase = PHASE_COMMAND_WAIT;
			}
			break;
		case PHASE_COMMAND_WAIT:
			result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
			SAFE_DELETE_ARRAY(pFrame)
			switch (result)
			{
				case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__);
					DEBUG_IF_PUT("deviceAddress = %u PHASE_COMMAND_WAIT result = ", deviceAddress);
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, "FRAME_RESULT_ID_NOT_FOUND\n");
					error();
					break;
				case IRpkDevice::FRAME_RESULT_ERROR:
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__);
					DEBUG_IF_PUT("deviceAddress = %u PHASE_COMMAND_WAIT result = ", deviceAddress);
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, "FRAME_RESULT_ERROR\n");
					error();
					break;
				case IRpkDevice::FRAME_RESULT_READY:
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__);
					DEBUG_IF_PUT("deviceAddress = %u PHASE_COMMAND_WAIT result = ", deviceAddress);
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, "FRAME_RESULT_READY\n");
					finish();
					break;
			}
			break;
	}
}

void ActionStopProgramScan::error()
{
	state = STATE_ERROR;
	phase = PHASE_STOP;
}

void ActionStopProgramScan::finish()
{
	state = STATE_READY;
	phase = PHASE_STOP;
}
