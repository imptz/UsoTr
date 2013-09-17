#include "LogicCooling.h"
#include "../DEBUG/serialDebug.h"
#include "../extension/subsystems/monitoring/monitoringSubsystem.h"

const char* LogicCooling::CONFIRMATION_TEXT = LOCAL_CONFIRMATION_USO_LOGIC_COOLING_TEXT;
const char* LogicCooling::CANCEL_LOG_TEXT = LOCAL_LOGIC_COOLING_CANCEL_LOG_TEXT;
const char* LogicCooling::START_LOG_TEXT = LOCAL_LOGIC_COOLING_START_LOG_TEXT;
const char* LogicCooling::FINISH_LOG_TEXT = LOCAL_LOGIC_COOLING_FINISH_LOG_TEXT;
const char* LogicCooling::LOG_FAULT_TEXT = LOCAL_LOGIC_COOLING_LOG_FAULT_TEXT;

bool ffll = false;

LogicCooling::LogicCooling(MessageReceiver* _messageReceiver)
	: Logic(), phase(PHASE_INPUT_CONTROL)
{
	addReceiver(_messageReceiver);
	dialogText = const_cast<char*>(CONFIRMATION_TEXT);
	cancelLogText = const_cast<char*>(CANCEL_LOG_TEXT);
	startLogText = const_cast<char*>(START_LOG_TEXT);
}

LogicCooling::~LogicCooling()
{

}

void LogicCooling::onMessage(Message message)
{
	switch (message.msg)
	{
		case MainConfirmation::CONFIRMATION_MESSAGE_RESULT:
			if (message.par1 == MainConfirmation::CONFIRMATION_RESULT_YES)
			{
				if (start())
				{
					Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, startLogText, START_ACTOR_HALF_AUTO, initSignal);
					MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_START_OROSHENIA, START_ACTOR_HALF_AUTO, initSignal);
				}
			}
			else
			{
				stop();
				MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_OTMENA_SIGNALA_O_VOZGORANII, 0, initSignal);
				Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, cancelLogText, 0, initSignal);
			}
			break;
		case MainFinish::FINISH_MESSAGE_RESULT:
			finish(FINISH_ACTOR_BUTTON);
			break;
	}
}

bool LogicCooling::start()
{
	if (listProgramIndexCount != 0)
	{
		if (phaseStopProgram_Start())
		{
			sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, LOGIC_MESSAGE_GET_FINISH, 0, 0));
			
			IOSubsystem::getSingleton().enableAllFireAlarmOutputs();
			IOSubsystem::getSingleton().enableAllHardwareOutputs();
			phase = PHASE_STOP_PROGRAM;
			pumpOutputEnable = false;
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("LogicCooling::start\n"); 
			return true;
		}
		else
		{
			stop();
			return false;
		}
	}
	else
	{
		stop();
		return false;
	}
}

void LogicCooling::stop(bool msg)
{
	timeOutBeforeStart = -1;
	finishTimer = -1;
	SAFE_DELETE_ARRAY(listProgramIndex)
	listProgramIndexCount = 0;
	setInitSignalIgnorable(initSignal, true);
	sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, MainTabControl::MAIN_TAB_MESSAGE_SET_MAIN_TAB, 0, 0));
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("LogicCooling::stop: \n"); 
	if (msg)
	{
		Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(FINISH_LOG_TEXT), finishActor, 0);
		MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_STOP_POISKA_OROSHENIA, finishActor, 0);

//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING(const_cast<char*>(FINISH_LOG_TEXT)); 
	}
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("\n"); 

	if (actionCount != 0)
	{
		for (unsigned int i = 0; i < actionCount; i++)
			SAFE_DELETE(actionList[i])
	}

	actionCount = 0;

	SAFE_DELETE_ARRAY(actionList)

	UI::getSingleton().getUsoModeControl()->unLock();
	phase = PHASE_INPUT_CONTROL;
}

void LogicCooling::action()
{
	for (unsigned int i = 0; i < actionCount; i++)
		if (actionList[i] != nullptr)
			actionList[i]->step();

	switch (phase)
	{
		case PHASE_INPUT_CONTROL:
			initSignal = getActiveInitialSignal(LOGIC_FUNCTION_COOLING_POINT, LOGIC_FUNCTION_COOLING_LINE);
			if (initSignal != -1)
			{
				if (UI::getSingleton().getUsoModeControl()->getMode() != UsoModeControl::USO_MODE_TOOLS)
				{
					phase = PHASE_INPUT_WAITING_CONTROL;
					timeOutBeforeStart = Config::getSingleton().getConfigData()->getConfigDataStructConst()->timeOutBeforeStart;
				}
			}
			break;
		case PHASE_INPUT_WAITING_CONTROL:
			if (timeOutBeforeStart == 0)
			{
				timeOutBeforeStart = -1;	
				if (testInitSignal(initSignal))
				{
					MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_SIGNAL_O_VOZGORANII, 0, initSignal);
					phase = PHASE_INPUT_ACTION;
				}
				else
				{
					initSignal = -1;
					phase = PHASE_INPUT_CONTROL;
				}
			}
			break;
		case PHASE_INPUT_ACTION:
			if (UI::getSingleton().getUsoModeControl()->lock())
			{
				if (UI::getSingleton().getUsoModeControl()->getMode() == UsoModeControl::USO_MODE_HALF_AUTO)
				{
					sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, LOGIC_MESSAGE_GET_CONFIRMATION, reinterpret_cast<unsigned int>(dialogText), 0));
					phase = PHASE_WAITING_CONFIRMATION;
					return;
				}
				else
					if (UI::getSingleton().getUsoModeControl()->getMode() == UsoModeControl::USO_MODE_FULL_AUTO)
					{
						if (start())
						{
							Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, startLogText, START_ACTOR_FULL_AUTO, initSignal);
							MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_START_OROSHENIA, START_ACTOR_FULL_AUTO, initSignal);
						}
						return;
					}
			}
			else
			{
				initSignal = -1;
				phase = PHASE_INPUT_CONTROL;
			}
			break;
		case PHASE_STOP_PROGRAM:
			if (phaseStopProgram_Execution())
			{
				phaseStartProgram_Start();
				phase = PHASE_START_PROGRAM;
			}
			break;
		case PHASE_START_PROGRAM:
			if (phaseStartProgram_Execution())
			{
				if (testTotalError())
				{
					stop();
					Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(LOG_FAULT_TEXT), finishActor, 0);
				}
				else
				{
//stop();
//Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(LOG_FAULT_TEXT), finishActor, 13);
					phaseGateOpen_Start();
					phase = PHASE_OPEN_GATE;
				}
			}
			break;
		case PHASE_OPEN_GATE:
			if (phaseGateOpen_Execution())
			{
				phaseWaitingStop_Start();
				phase = PHASE_WAITING_STOP;
			}
			break;
		case PHASE_WAITING_STOP:
			if (phaseWaitingStop_Execution())
			{
				phaseGateClose_Start();
				phase = PHASE_CLOSE_GATE;
			}
			break;
		case PHASE_CLOSE_GATE:
			if (phaseGateClose_Execution())
			{
				phase = PHASE_STOP_PROGRAM_END;
				phaseStopProgramEnd_Start();
			}
			break;
		case PHASE_STOP_PROGRAM_END:
			if (phaseStopProgramEnd_Execution())
			{
				stop(true);
			}
			break;
	}
}

void LogicCooling::finish(FINISH_ACTOR _finishActor)
{
	_asm cli
	if (phase == PHASE_WAITING_STOP)
	{
		finishTimer = -1;
		phase = PHASE_CLOSE_GATE;
		finishActor = _finishActor;
		sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, MainFinish::FINISH_MESSAGE_LABEL, MainFinish::FINISH_MESSAGE_PARAM_FINISH, 0));
		_asm sti
		phaseGateClose_Start();
	}
	_asm sti
}

bool LogicCooling::phaseStopProgram_Start()
{
	actionCount = listProgramIndexCount; 
	actionList = new Action*[actionCount];

	ConfigDataStructProgram** sp = Config::getSingleton().getConfigData()->getConfigDataStructPrograms();

	for (unsigned int i = 0; i < actionCount; i++)
	{
		actionList[i] = new ActionStopProgramScan(Config::getSingleton().getConfigData()->getPRAddressByNumber(sp[listProgramIndex[i]]->prNumber));
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseStopProgram_Start address: ", 
//	sp[listProgramIndex[i]]->prNumber,
//	Config::getSingleton().getConfigData()->getPRAddressByNumber(sp[listProgramIndex[i]]->prNumber)); 
	}

	return true;
}

bool LogicCooling::phaseStopProgram_Execution()
{
	for (unsigned int i = 0; i < actionCount; i++)
		if (actionList[i]->getState() == Action::STATE_UNDEFINED)
			return false;

	for (unsigned int i = 0; i < actionCount; i++)
		switch (actionList[i]->getState())
		{
			case Action::STATE_ERROR:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseStopProgram_ERROR: ", i, actionCount); 
				break;
			case Action::STATE_READY:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseStopProgram_READY: ", i, actionCount); 
				break;
		}	
	return true;
}

bool LogicCooling::phaseStartProgram_Start()
{
	ConfigDataStructProgram** sp = Config::getSingleton().getConfigData()->getConfigDataStructPrograms();

	for (unsigned int i = 0; i < actionCount; i++)
	{
		if (actionList[i] != nullptr)
			delete actionList[i];

		unsigned char addr = Config::getSingleton().getConfigData()->getPRAddressByNumber(sp[listProgramIndex[i]]->prNumber);

		switch (sp[listProgramIndex[i]]->function)
		{
			case LOGIC_FUNCTION_COOLING_LINE:
				actionList[i] = new ActionStartProgramScanLine(sp[listProgramIndex[i]]->point1,
					sp[listProgramIndex[i]]->point2, sp[listProgramIndex[i]]->nasadok, SCAN_PROGRAM_BALLISTICS_OFF, addr);
SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING4("phaseStartProgram_LINE: ", i, actionCount, sp[listProgramIndex[i]]->prNumber, addr); 
				break;
			case LOGIC_FUNCTION_COOLING_POINT:
				actionList[i] = new ActionStartProgramScanPoint(sp[listProgramIndex[i]]->nPointProgram, addr);
SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseStartProgram_POINT: ", i, actionCount); 
				break;
		}
	}

	return true;
}

bool LogicCooling::phaseStartProgram_Execution()
{
	for (unsigned int i = 0; i < actionCount; i++)
		if (actionList[i]->getState() == Action::STATE_UNDEFINED)
			return false;

	unsigned int _actionCount = actionCount;
	for (unsigned int i = 0; i < _actionCount; i++)
	{
		switch (actionList[i]->getState())
		{
			case Action::STATE_ERROR:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseStartProgram_ERROR: ", i, actionCount); 
				deleteActionInList(i);
				break;
			case Action::STATE_READY:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseStartProgram_READY: ", i, actionCount); 
				break;
		}	
	}
	return true;
}

bool LogicCooling::phaseGateOpen_Start()
{
	for (unsigned int i = 0; i < actionCount; i++)
	{
		unsigned char deviceAddress;
		if (actionList[i] != nullptr)
		{
			deviceAddress = actionList[i]->getDeviceAddress();
			delete actionList[i];
		}

		actionList[i] = new ActionGateOpen(deviceAddress);
	}

	return true;
}

bool LogicCooling::phaseGateOpen_Execution()
{
	bool isUndefined = false;
	for (unsigned int i = 0; i < actionCount; i++)
	{
		switch (actionList[i]->getState())
		{
			case Action::STATE_UNDEFINED:
				isUndefined = true;
				break;
			case Action::STATE_READY:
				if (!pumpOutputEnable)
				{
					pumpOutputEnable = true;
					IOSubsystem::getSingleton().enableAllPumpStationOutputs();
				}
				break;
		}	
	}

	if (isUndefined)
		return false;

	unsigned int _actionCount = actionCount;
	for (unsigned int i = 0; i < _actionCount; i++)
	{
		switch (actionList[i]->getState())
		{
			case Action::STATE_ERROR:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseGateOpen_ERROR: ", i, actionCount); 
				//deleteActionInList(i);
				break;
			case Action::STATE_READY:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseGateOpen_READY: ", i, actionCount); 
				break;
		}	
	}

	return true;
}

bool LogicCooling::phaseWaitingStop_Start()
{
	sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, MainFinish::FINISH_MESSAGE_LABEL, MainFinish::FINISH_MESSAGE_PARAM_START, 0));
	return true;
}

bool LogicCooling::phaseWaitingStop_Execution()
{
	if (UI::getSingleton().getUsoModeControl()->getMode() == UsoModeControl::USO_MODE_FULL_AUTO)
	{
		if (!testInitSignal(initSignal))
		{
			if (finishTimer == -1)
				finishTimer = Config::getSingleton().getConfigData()->getConfigDataStructConst()->timeOutBeforeFinish;
		}
	}
	return false;
}

bool LogicCooling::phaseGateClose_Start()
{
	IOSubsystem::getSingleton().disableAllFireAlarmOutputs();
	IOSubsystem::getSingleton().disableAllHardwareOutputs();
	IOSubsystem::getSingleton().disableAllPumpStationOutputs();
	ConfigDataStructProgram** sp = Config::getSingleton().getConfigData()->getConfigDataStructPrograms();

	for (unsigned int i = 0; i < actionCount; i++)
	{
		if (actionList[i] != nullptr)
			delete actionList[i];
	}

	actionCount = listProgramIndexCount; 
	for (unsigned int i = 0; i < actionCount; i++)
	{
		unsigned char addr = Config::getSingleton().getConfigData()->getPRAddressByNumber(sp[listProgramIndex[i]]->prNumber);
		actionList[i] = new ActionGateClose(addr);
	}
	return true;
}

bool LogicCooling::phaseGateClose_Execution()
{
	for (unsigned int i = 0; i < actionCount; i++)
		if (actionList[i]->getState() == Action::STATE_UNDEFINED)
			return false;

	for (unsigned int i = 0; i < actionCount; i++)
		switch (actionList[i]->getState())
		{
			case Action::STATE_ERROR:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseGateClose_ERROR: ", i, actionCount); 
				break;
			case Action::STATE_READY:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseGateClose_READY: ", i, actionCount); 
				break;
		}	
	return true;
}

bool LogicCooling::phaseStopProgramEnd_Start()
{
	ConfigDataStructProgram** sp = Config::getSingleton().getConfigData()->getConfigDataStructPrograms();

	for (unsigned int i = 0; i < actionCount; i++)
	{
		if (actionList[i] != nullptr)
			delete actionList[i];
	}

	actionCount = listProgramIndexCount; 
	for (unsigned int i = 0; i < actionCount; i++)
	{
		unsigned char addr = Config::getSingleton().getConfigData()->getPRAddressByNumber(sp[listProgramIndex[i]]->prNumber);
		actionList[i] = new ActionStopProgramScan(addr);
	}

	return true;
}

bool LogicCooling::phaseStopProgramEnd_Execution()
{
	for (unsigned int i = 0; i < actionCount; i++)
		if (actionList[i]->getState() == Action::STATE_UNDEFINED)
			return false;

	for (unsigned int i = 0; i < actionCount; i++)
	{
		if (actionList[i] == nullptr)
			break;

		switch (actionList[i]->getState())
		{
			case Action::STATE_ERROR:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseStopProgramEnd_ERROR: ", i, actionCount); 
				break;
			case Action::STATE_READY:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseStopProgramEnd_READY: ", i, actionCount); 
				break;
		}	
	}

	return true;
}
