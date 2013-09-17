#include "actionSearch.h"
#include "../extension/subsystems/rpk/rpkSubsystem.h"

ActionSearch::ActionSearch()
{
}

ActionSearch::ActionSearch(unsigned char _deviceAddress, Point2<unsigned int> _point1, Point2<unsigned int> _point2, int _numberSourcesFlicker)
	:	Action(_deviceAddress), phase(PHASE_COMMAND), point1(_point1), point2(_point2), numberSourcesFlicker(_numberSourcesFlicker)
{
	DEBUG_IF_PUT_METHOD("deviceAddress = %u, point1.x = %u, point1.y = %u, point2.x = %u, point2.y = %u, numberSourcesFlicker = %u\n", _deviceAddress, point1.x, point2.x, point1.y, point2.y, numberSourcesFlicker);
}

ActionSearch::~ActionSearch()
{
	DEBUG_IF_PUT_METHOD("deviceAddress = %u\n", deviceAddress)
}

void ActionSearch::timerHandler()
{
	if (timeCounter < TEST_TIME_OUT)
		timeCounter++;
}

void ActionSearch::step()
{
	unsigned char* pFrame = nullptr;
	unsigned char frame[20];

	IRpkDevice::FRAME_RESULT result;

	switch (phase)
	{
		case PHASE_COMMAND:

			point1.x = 360 - point1.x;
			point2.x = 360 - point2.x;

			frame[0] = deviceAddress;
			frame[1] = 0;
			frame[2] = RPK_COMMANDS_START_SEARCH;
			frame[3] = 9;
			frame[4] = point1.x >> 8;
			frame[5] = point1.x;
			frame[6] = point2.x >> 8;
			frame[7] = point2.x;
			frame[8] = point1.y >> 8;
			frame[9] = point1.y;
			frame[10] = point2.y >> 8;
			frame[11] = point2.y;
			frame[12] = numberSourcesFlicker;

			point1.x = 360 - point1.x;
			point2.x = 360 - point2.x;

			DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_COMMAND frame = %u %u %u %u %u %u %u %u %u %u %u %u", deviceAddress, frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9], frame[10], frame[11], frame[12])

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
					if (pFrame[5] == 0)
					{
						if (pFrame[6] == 0)
						{
							DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_TEST_WAIT", deviceAddress)
							DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " очагов не найдено\n")
							error();
						}
						else
						{
							DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_TEST_WAIT", deviceAddress)
							DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, " очаги найдены\n")
							phase = PHASE_GET_RESULT;
						}
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
		case PHASE_GET_RESULT:
			frame[0] = deviceAddress;
			frame[1] = 0;
			frame[2] = RPK_COMMANDS_GET_SEARCH_RESULT;
			frame[3] = 0;
			frameId = RpkSubsystem::getSingleton().write(frame);
			if (frameId == IRpkDevice::BAD_FRAME_ID)
			{
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
				DEBUG_IF_PUT("deviceAddress = %u PHASE_GET_RESULT", deviceAddress)
				DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " BAD_FRAME_ID \n")
				error();
			}
			else
				phase = PHASE_GET_RESULT_WAIT;
			break;
		case PHASE_GET_RESULT_WAIT:
			result = RpkSubsystem::getSingleton().read(frameId, &pFrame);
			switch (result)
			{
				case IRpkDevice::FRAME_RESULT_ID_NOT_FOUND:
					DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_GET_RESULT_WAIT", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " FRAME_RESULT_ID_NOT_FOUND \n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_ERROR:
					DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_GET_RESULT_WAIT", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_RED, " FRAME_RESULT_ERROR \n")
					error();
					break;
				case IRpkDevice::FRAME_RESULT_READY:
					resPoint1.x = (pFrame[5] << 8) + pFrame[6];
					resPoint1.y = (pFrame[7] << 8) + pFrame[8];
					resPoint2.x = (pFrame[9] << 8) + pFrame[10];
					resPoint2.y = (pFrame[11] << 8) + pFrame[12];

					DEBUG_IF_PUT_METHOD("deviceAddress = %u PHASE_GET_RESULT_WAIT", deviceAddress)
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_GREEN, " FRAME_RESULT_READY \n")
					DEBUG_IF_PUT_METHOD("deviceAddress = %u координаты очага: x1 = %i, y1 = %i, x2 = %i, y2 = %i\n", deviceAddress, resPoint1.x, resPoint1.y, resPoint2.x, resPoint2.y)

					resPoint1.x = 360 - resPoint1.x;
					resPoint2.x = 360 - resPoint2.x;
					DEBUG_IF_PUT_METHOD("deviceAddress = %u координаты очага после инверсии горизонтальных координат: x1 = %i, y1 = %i, x2 = %i, y2 = %i\n", deviceAddress, resPoint1.x, resPoint1.y, resPoint2.x, resPoint2.y)

					finish();
					break;
			}

			SAFE_DELETE_ARRAY(pFrame)

			break;
	}
}

void ActionSearch::error(int value)
{
	state = STATE_ERROR;
	phase = PHASE_STOP;
}

void ActionSearch::finish()
{
	state = STATE_READY;
	phase = PHASE_STOP;
}

bool ActionSearch::getResult(Point2<int>& _point1, Point2<int>& _point2)
{
	if (state == STATE_READY)
	{
		_point1.x = resPoint1.x;
		_point1.y = resPoint1.y;
		_point2.x = resPoint2.x;
		_point2.y = resPoint2.y;

		return true;
	}
	else
		return false;
}
