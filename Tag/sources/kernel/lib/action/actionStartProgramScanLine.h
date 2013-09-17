#pragma once

#include "action.h"
#include "../math/math.h"

class ActionStartProgramScanLine : public Action
{
	private:
		virtual void timerHandler();
		static const unsigned int TEST_TIME_OUT = 10;
		ActionStartProgramScanLine();

	public:
		ActionStartProgramScanLine(Point2<int> point1, Point2<int> point2, unsigned int nasadok, SCAN_PROGRAM_BALLISTICS _ballistic, unsigned char _deviceAddress);
		virtual ~ActionStartProgramScanLine();
		virtual void step();

	private:
		enum PHASE
		{
			PHASE_COMMAND,
			PHASE_COMMAND_WAIT,
			PHASE_TEST,
			PHASE_TEST_WAIT,
			PHASE_STOP
		};
		PHASE phase;

		int frameId;

		void error();
		void finish();

		Point2<int> point1;
		Point2<int> point2;
		unsigned int nasadok;

		SCAN_PROGRAM_BALLISTICS ballistic;
};
