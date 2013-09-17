#pragma once

#include "action.h"

class ActionGetNetworkControllerInfo : public Action
{
	private:
		virtual void timerHandler();
		unsigned int testTimeOut;
		static const unsigned int TEST_TIME_OUT = 3;

	public:
		ActionGetNetworkControllerInfo();
		virtual ~ActionGetNetworkControllerInfo();
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

		void error();
		void finish();

		static const unsigned char NETWORK_CONTROLLER_ADDRESS = 33;
};
