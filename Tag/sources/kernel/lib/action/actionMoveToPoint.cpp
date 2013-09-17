#include "actionMoveToPoint.h"
#include "../extension/subsystems/rpk/rpkSubsystem.h"

ActionMoveToPoint::ActionMoveToPoint()
{
}

ActionMoveToPoint::ActionMoveToPoint(unsigned char _deviceAddress, Point2<unsigned int> _point, unsigned int _nasadok)
	:	Action(_deviceAddress), phase(PHASE_COMMAND), point(_point), nasadok(_nasadok)
{
	DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
	DEBUG_IF_PUT("deviceAddress = %u x = %u, y = %u, nasadok = %u\n", _deviceAddress, _point.x, _point.y, nasadok)
}

ActionMoveToPoint::~ActionMoveToPoint()
{
	DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
	DEBUG_IF_PUT("deviceAddress = %u\n", deviceAddress)
}

void ActionMoveToPoint::timerHandler()
{
	if (timeCounter < TEST_TIME_OUT)
		timeCounter++;
}

void ActionMoveToPoint::step()
{
	unsigned char* pFrame = nullptr;
	unsigned char frame[20];

	unsigned int pointX = 360 - point.x;

	IRpkDevice::FRAME_RESULT result;

	switch (phase)
	{
		case PHASE_COMMAND:
			frame[0] = deviceAddress;
			frame[1] = 0;
			frame[2] = RPK_COMMANDS_MOVE_TO_POINT;
			frame[3] = 6;
			frame[4] = pointX >> 8;
			frame[5] = pointX;
			frame[6] = point.y >> 8;
			frame[7] = point.y;
			frame[8] = nasadok >> 8;
			frame[9] = nasadok;

			DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
			DEBUG_IF_PUT("deviceAddress = %u PHASE_COMMAND frame = %u %u %u %u %u %u %u %u %u", deviceAddress, frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9])

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
			frame[2] = RPK_COMMANDS_GET_STATUS_MOVE_TO_POINT;
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
					switch (pFrame[5])
					{
						case 3:
							DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
							DEBUG_IF_PUT("deviceAddress = %u PHASE_TEST_WAIT FRAME_RESULT_READY", deviceAddress)
							DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, " finish\n")
							finish();
							break;
						case 2:
							DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
							DEBUG_IF_PUT("deviceAddress = %u PHASE_TEST_WAIT FRAME_RESULT_READY", deviceAddress)
							DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " error\n")
							error();
							break;
						default:
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
			}

			SAFE_DELETE_ARRAY(pFrame)

			break;
	}
}

void ActionMoveToPoint::error()
{
	state = STATE_ERROR;
	phase = PHASE_STOP;
}

void ActionMoveToPoint::finish()
{
	state = STATE_READY;
	phase = PHASE_STOP;
}

Point2<unsigned int> ActionMoveToPoint::getPoint()
{
	return point;
}
