#include "ActionStartProgramScanLine.h"
#include "../extension/subsystems/rpk/rpkSubsystem.h"

#include "../extension/subsystems/monitoring/monitoringSubsystem.h"

ActionStartProgramScanLine::ActionStartProgramScanLine()
{
}

ActionStartProgramScanLine::ActionStartProgramScanLine(Point2<int> _point1, Point2<int> _point2, unsigned int _nasadok, 
	SCAN_PROGRAM_BALLISTICS _ballistic, unsigned char _deviceAddress)
	:	Action(_deviceAddress), phase(PHASE_COMMAND), point1(_point1), point2(_point2), nasadok(_nasadok), ballistic(_ballistic)
{
	DEBUG_IF_PUT_METHOD("deviceAddress = %u x1 = %u, y1 = %u, x2 = %u, y2 = %u, nasadok = %u ", _deviceAddress, _point1.x, _point1.y, _point2.x, _point2.y, nasadok);
	if (ballistic == SCAN_PROGRAM_BALLISTICS_OFF)
		DEBUG_IF_PUT("SCAN_PROGRAM_BALLISTICS_OFF\n")
	else
		DEBUG_IF_PUT("SCAN_PROGRAM_BALLISTICS_ON\n")
}

ActionStartProgramScanLine::~ActionStartProgramScanLine()
{
	DEBUG_IF_PUT_METHOD("deviceAddress = %u\n", deviceAddress)
}

void ActionStartProgramScanLine::timerHandler()
{
	if (timeCounter < TEST_TIME_OUT)
		timeCounter++;
}

void ActionStartProgramScanLine::step()
{
	unsigned char* pFrame = nullptr;
	unsigned char frame[20];

	IRpkDevice::FRAME_RESULT result;

	unsigned int point1X = 360 - point1.x;
	unsigned int point2X = 360 - point2.x;

	switch (phase)
	{
		case PHASE_COMMAND:
			frame[0] = deviceAddress;
			frame[1] = 0;
			frame[2] = RPK_COMMANDS_START_SCAN_PROGRAM; 
			frame[3] = 12;
			frame[4] = SCAN_PROGRAM_STEP_LOW;
			frame[5] = point1X >> 8;
			frame[6] = point1X;
			frame[7] = point2X >> 8;
			frame[8] = point2X;
			frame[9] = point1.y >> 8;
			frame[10] = point1.y;
			frame[11] = point2.y >> 8;
			frame[12] = point2.y;
			frame[13] = ballistic;
			frame[14] = nasadok >> 8;
			frame[15] = nasadok;

			DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_COMMAND frame = %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u", deviceAddress, frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9], frame[10], frame[11], frame[12], frame[13], frame[14], frame[15])

			frameId = RpkSubsystem::getSingleton().write(frame);
			if (frameId == IRpkDevice::BAD_FRAME_ID)
			{
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " BAD_FRAME_ID\n")
				error();
			}
			else
			{
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, " done\n")

				unsigned char par2 = 0;
				unsigned char par3 = 0;
				unsigned char par4 = 0;

				par2 = (point1X & 0x0ff0) >> 4;
				par3 = (point1X & 0x000f) << 4;
				par3 |= (point2X & 0x0f00) >> 8;
				par4 = point2X & 0x00ff;

				MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_START_PROGRAMMI_SCANIROVANIJA, deviceAddress, par2, par3, par4);

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
				DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_TEST", deviceAddress)
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

void ActionStartProgramScanLine::error()
{
	state = STATE_ERROR;
	phase = PHASE_STOP;
}

void ActionStartProgramScanLine::finish()
{
	state = STATE_READY;
	phase = PHASE_STOP;
}
