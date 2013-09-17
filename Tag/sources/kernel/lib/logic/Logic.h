#pragma once

#include <Multithreading\Thread.h>
#include "../timer/Timer.h"
#include "../config/Config.h"
#include "../message/Messages.h"
#include <Containers\vector.h>
#include "../action/action.h"
#include "../action/actionStartProgramScanLine.h"
#include "../action/actionStartProgramScanPoint.h"
#include "../action/actionStopProgramScan.h"
#include "../action/actionGateOpen.h"
#include "../action/actionGateClose.h"
#include <Containers\vector.h>
#include <Property\property.h>

class Logic : public Fr::Thread<Logic>, public ITimer, public MessageReceiver, public MessageSender
{
	private:
		virtual void timerHandler();
		static const unsigned int TIMER_PERIOD = 1000;

		virtual void onMessage(Message message);

	public:
		Logic(MessageReceiver* _messageReceiver = nullptr);
		~Logic();

		enum LOGIC_MESSAGE
		{
			LOGIC_MESSAGE_GET_CONFIRMATION = 1,
			LOGIC_MESSAGE_GET_FINISH = 2
		};

	private:
		Fr::CPointer<Logic> init();
		Fr::CPointer<Logic> inputsControl();
		Fr::CPointer<Logic> inputsPauseAndConfirmation();
		Fr::CPointer<Logic> machineStart();
		Fr::CPointer<Logic> userStart();
		Fr::CPointer<Logic> start();
		Fr::CPointer<Logic> search_start();
		Fr::CPointer<Logic> search_startControl();
		Fr::CPointer<Logic> cooling_start();
		Fr::CPointer<Logic> cooling_startControl();
		Fr::CPointer<Logic> gateOpen();
		Fr::CPointer<Logic> gateOpenControl();
		Fr::CPointer<Logic> startPhase2();
		Fr::CPointer<Logic> phase2ControlOffSignal();
		Fr::CPointer<Logic> phase2StopPr();
		Fr::CPointer<Logic> phase2CloseGate();
		Fr::CPointer<Logic> phase2CloseGateControl();
		Fr::CPointer<Logic> phase2TimerUserFinish();
		Fr::CPointer<Logic> phase2StartNewPr();
		Fr::CPointer<Logic> phase2StartNewPrControl();
		Fr::CPointer<Logic> phase2GateOpen();
		Fr::CPointer<Logic> phase2GateOpenControl();
		Fr::CPointer<Logic> startDown();
		Fr::CPointer<Logic> prStopControl();
		Fr::CPointer<Logic> gateClose();
		Fr::CPointer<Logic> gateCloseControl();

		Fr::Vector<int> signals;

		unsigned int* listProgramIndex;
		unsigned int listProgramIndexCount;

		int timeOutBeforeStart;
		
		LOGIC_FUNCTION mainFunction;

		int getActiveSignal(LOGIC_FUNCTION ignorableFunction = LOGIC_FUNCTION_UNDEFINED);
		bool testInitSignal(int index);
		void setInitSignalIgnorable(int number, bool value);

		enum START_ACTOR
		{
			START_ACTOR_FULL_AUTO = 0,
			START_ACTOR_HALF_AUTO = 1
		};

		enum USER_START_CONFIRMATION_RESULT
		{
			USER_START_CONFIRMATION_RESULT_UNDEFINED,
			USER_START_CONFIRMATION_RESULT_TRUE,
			USER_START_CONFIRMATION_RESULT_FALSE
		};

		USER_START_CONFIRMATION_RESULT userStartConfirmationResult;

		static const char* CONFIRMATION_START_SEARCH_TEXT;
		static const char* CONFIRMATION_START_OROSHENIE_TEXT;
		static const char* CANCEL_SEARCH_LOG_TEXT;
		static const char* CANCEL_OROSHENIE_LOG_TEXT;
		static const char* START_SEARCH_LOG_TEXT;
		static const char* START_OROSHENIE_LOG_TEXT;
		static const char* FAULT_OROSHENIE_LOG_TEXT;
		static const char* FINISH_OROSHENIE_LOG_TEXT;
		struct Pr
		{
			INIT_PROPERTY
			PROPERTY(int, signalIndex)
			PROPERTY(unsigned int, number)
			unsigned char address;
			Action* action;
			LOGIC_FUNCTION function;


			Pr(int __signalIndex = -1, unsigned int __number = 0, LOGIC_FUNCTION _function = LOGIC_FUNCTION_UNDEFINED)
				:	_signalIndex(__signalIndex), _number(__number), address(Config::getSingleton().getConfigData()->getPRAddressByNumber(_number)), action(nullptr), function(_function)
			{
			}

			~Pr()
			{
				if (action != nullptr)
					delete action;
			}
		};

		Fr::PropertyVector<Pr> prList;

		class LogicTimer
		{
			private:
				int counter;
				static const int STOP_COUNTER = -1;
				static const int EXPIRED_COUNTER = 0;

			public:
				LogicTimer()
					:	counter(STOP_COUNTER)
				{}

				void start(unsigned int startCounter)
				{
					counter = startCounter;
				}

				void stop()
				{
					counter = STOP_COUNTER;
				}

				void action()
				{
					if (counter > EXPIRED_COUNTER)
						counter--;
				}

				enum STATE
				{
					STATE_STOP = 0,
					STATE_COUNT = 1,
					STATE_EXPIRED = 2
				};

				STATE getState()
				{
					switch (counter)
					{
						case STOP_COUNTER:
							return STATE_STOP;
						case EXPIRED_COUNTER:
							return STATE_EXPIRED;
						default:
							return STATE_COUNT;
					}
				}
		};

		LogicTimer logicTimer;

		bool fEnableAllPumpStationOutputs;
		bool fUserFinish;

		unsigned int getPrStartCount(LOGIC_FUNCTION function = LOGIC_FUNCTION_ALL);
		bool isMaximumPrStart();

		bool debug;

	private:
		// M061112
		static const int TIME_OUT_WAITING_UNDEFINED = -1;
		int timeOutWaiting;
		int getConfigTimeOutWaiting();
		void startWaitingConf();
		// M061112E
};
