#include "actionGateClose.h"
#include "../extension/subsystems/rpk/rpkSubsystem.h"

ActionGateClose::ActionGateClose()
{
}

ActionGateClose::ActionGateClose(unsigned char _deviceAddress)
	:	Action(_deviceAddress), phase(PHASE_COMMAND)
{
	DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
	DEBUG_IF_PUT("deviceAddress = %u\n", _deviceAddress)
}

ActionGateClose::~ActionGateClose()
{
	DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
	DEBUG_IF_PUT("deviceAddress = %u\n", deviceAddress)
}

void ActionGateClose::timerHandler()
{
	if (timeCounter < TEST_TIME_OUT)
		timeCounter++;
}

void ActionGateClose::step()
{
	unsigned char* pFrame = nullptr;
	unsigned char frame[20];

	IRpkDevice::FRAME_RESULT result;

	switch (phase)
	{
		case PHASE_COMMAND:
			frame[0] = deviceAddress;
			frame[1] = 0;
			frame[2] = RPK_COMMANDS_GATE_CLOSE;
			frame[3] = 0;

			DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
			DEBUG_IF_PUT("deviceAddress = %u PHASE_COMMAND frame = %u %u %u %u", deviceAddress, frame[0], frame[1], frame[2], frame[3])

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
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
					DEBUG_IF_PUT("deviceAddress = %u PHASE_COMMAND_WAIT result = ", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, "FRAME_RESULT_ID_NOT_FOUND\n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_ERROR:
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
					DEBUG_IF_PUT("deviceAddress = %u PHASE_COMMAND_WAIT result = ", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, "FRAME_RESULT_ERROR\n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_READY:
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
					DEBUG_IF_PUT("deviceAddress = %u PHASE_COMMAND_WAIT result = ", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, "FRAME_RESULT_READY\n")
					phase = PHASE_TEST;
					break;
			}
			break;
		case PHASE_TEST:
			frame[0] = deviceAddress;
			frame[1] = 0;
			frame[2] = RPK_COMMANDS_GATE_INFO;
			frame[3] = 0;
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
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
					DEBUG_IF_PUT("deviceAddress = %u PHASE_TEST_WAIT", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " FRAME_RESULT_ID_NOT_FOUND \n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_ERROR:
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
					DEBUG_IF_PUT("deviceAddress = %u PHASE_TEST_WAIT", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " FRAME_RESULT_ERROR \n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_READY:
					if ((pFrame[5] & 0x0c) == 0x04)
					{
						DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
						DEBUG_IF_PUT("deviceAddress = %u PHASE_TEST_WAIT FRAME_RESULT_READY", deviceAddress)
						DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, " finish\n")
						finish();
					}
					else
						if (timeCounter < TEST_TIME_OUT)
							phase = PHASE_TEST;
						else
						{
							DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
							DEBUG_IF_PUT("deviceAddress = %u PHASE_TEST_WAIT FRAME_RESULT_READY", deviceAddress)
							DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " TIME_OUT\n")
							error();
						}
					break;
			}

			SAFE_DELETE_ARRAY(pFrame)

			break;
	}
}

void ActionGateClose::error()
{
	state = STATE_ERROR;
	phase = PHASE_STOP;
}

void ActionGateClose::finish()
{
	state = STATE_READY;
	phase = PHASE_STOP;
}
