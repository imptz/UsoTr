#include "ActionStartProgramScanPoint.h"
#include "../extension/subsystems/rpk/rpkSubsystem.h"
#include "../log/Log.h"
#include "../DEBUG/serialDebug.h"

SCAN_PROGRAM_FLAGS ActionStartProgramScanPoint::nasadokOnOff = SCAN_PROGRAM_FLAGS_NASADOK_ON;

ActionStartProgramScanPoint::ActionStartProgramScanPoint()
{
}

ActionStartProgramScanPoint::ActionStartProgramScanPoint(unsigned int _nPointProgram, unsigned char _deviceAddress)
	:	Action(_deviceAddress), phase(PHASE_COMMAND), nPointProgram(_nPointProgram)
{
	DEBUG_IF_PUT_METHOD("deviceAddress = %u, nPointProgram = %u\n", _deviceAddress, _nPointProgram);
}

ActionStartProgramScanPoint::~ActionStartProgramScanPoint()
{
	DEBUG_IF_PUT_METHOD("deviceAddress = %u\n", deviceAddress)
}

void ActionStartProgramScanPoint::timerHandler()
{
	if (timeCounter < TEST_TIME_OUT)
		timeCounter++;
}

void ActionStartProgramScanPoint::step()
{
	unsigned char* pFrame = nullptr;
	unsigned char frame[20];

	IRpkDevice::FRAME_RESULT result;

	switch (phase)
	{
		case PHASE_COMMAND:
			frame[0] = deviceAddress;
			frame[1] = 0; 
			frame[2] = RPK_COMMANDS_START_SCAN_PROGRAM_OP; 
			frame[3] = 2;
			frame[4] = nPointProgram;
			frame[5] = nasadokOnOff;

			DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_COMMAND frame = %u %u %u %u %u %u %u", deviceAddress, frame[0], frame[1], frame[2], frame[3], frame[4], frame[5])

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
			SAFE_DELETE_ARRAY(pFrame)
			switch (result)
			{
				case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
					DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_COMMAND_WAIT result = ", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, "FRAME_RESULT_ID_NOT_FOUND\n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_ERROR:
					DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_COMMAND_WAIT result = ", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, "FRAME_RESULT_ERROR\n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_READY:
					DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_COMMAND_WAIT result = ", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, "FRAME_RESULT_READY\n")
					phase = PHASE_TEST;
					break;
			}
			break;
		case PHASE_TEST:
			frame[0] = deviceAddress;
			frame[1] = 0;
			frame[2] = RPK_COMMANDS_FLAGS;
			frame[3] = 0;
			frame[4] = 0;
			frameId = RpkSubsystem::getSingleton().write(frame);
			if (frameId == IRpkDevice::BAD_FRAME_ID)
			{
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
				DEBUG_IF_PUT("deviceAddress = %u PHASE_TEST", deviceAddress)
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " BAD_FRAME_ID \n")
				error();
			}
			else
				phase = PHASE_TEST_WAIT;
			break;
		case PHASE_TEST_WAIT:
			result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
			switch (result)
			{
				case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
					DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_TEST_WAIT", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " FRAME_RESULT_ID_NOT_FOUND \n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_ERROR:
					DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_TEST_WAIT", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " FRAME_RESULT_ERROR \n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_READY:
					if (pFrame[5] == 1)
					{
						DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_TEST_WAIT FRAME_RESULT_READY", deviceAddress)
						DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, " finish\n")
						finish();
					}
					else
						if (timeCounter < TEST_TIME_OUT)
							phase = PHASE_TEST;
						else
						{
							DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_TEST_WAIT FRAME_RESULT_READY", deviceAddress)
							DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " TIME_OUT\n")
							error();
						}
					break;
			}

			SAFE_DELETE_ARRAY(pFrame)

			break;
	}
}

void ActionStartProgramScanPoint::error()
{
	state = STATE_ERROR;
	phase = PHASE_STOP;
}

void ActionStartProgramScanPoint::finish()
{
	state = STATE_READY;
	phase = PHASE_STOP;
}
