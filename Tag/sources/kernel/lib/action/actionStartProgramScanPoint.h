#pragma once

#include "action.h"
#include "../math/math.h"

class ActionStartProgramScanPoint : public Action
{
	private:
		virtual void timerHandler();
		static const unsigned int TEST_TIME_OUT = 10;
		ActionStartProgramScanPoint();

	public:
		ActionStartProgramScanPoint(unsigned int _nPointProgram, unsigned char _deviceAddress);
		virtual ~ActionStartProgramScanPoint();
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

		unsigned int nPointProgram;

		static SCAN_PROGRAM_FLAGS nasadokOnOff;

		friend class ActionDummy;
};
