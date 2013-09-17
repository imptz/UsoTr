#include "LogicSearching.h"
#include "../math/math.h"
#include "../DEBUG/serialDebug.h"
#include "../extension/subsystems/monitoring/monitoringSubsystem.h"

const char* LogicSearching::CONFIRMATION_TEXT = LOCAL_CONFIRMATION_USO_LOGIC_SEARCH_TEXT;
const char* LogicSearching::CONFIRMATION_EXTINGUISHING_TEXT = LOCAL_CONFIRMATION_USO_LOGIC_SEARCH_EXTINGUISHING_TEXT;
const char* LogicSearching::CANCEL_LOG_TEXT = LOCAL_LOGIC_SEARCHING_CANCEL_LOG_TEXT;
const char* LogicSearching::START_LOG_TEXT = LOCAL_LOGIC_SEARCHING_START_LOG_TEXT;
const char* LogicSearching::FINISH_LOG_TEXT = LOCAL_LOGIC_SEARCHING_FINISH_LOG_TEXT;
const char* LogicSearching::LOG_FAULT_TEXT = LOCAL_LOGIC_SEARCHING_LOG_FAULT_TEXT;
const char* LogicSearching::LOG_NOT_FIRE_DETECT_TEXT = LOCAL_LOGIC_SEARCHING_LOG_NOT_FIRE_DETECT_TEXT;
const char* LogicSearching::LOG_CANCEL_EXTINGUISHING_TEXT = LOCAL_LOGIC_SEARCHING_LOG_CANCEL_EXTINGUISHING_TEXT;
const char* LogicSearching::LOG_START_EXTINGUISHING_TEXT = LOCAL_LOGIC_SEARCHING_LOG_START_EXTINGUISHING_TEXT;

LogicSearching::LogicSearching(MessageReceiver* _messageReceiver)
	: Logic(), phase(PHASE_INPUT_CONTROL), fireCount(0), programs(nullptr), 
	  coolingSignal(-1), listCoolingStartIndex(nullptr), 
	  listCoolingStartCount(0), listOff(nullptr), listOffCount(0), actionStartList(nullptr), actionStopList(nullptr), isCoolingStart(false), localFires(nullptr)
{
	addReceiver(_messageReceiver);
	dialogText = const_cast<char*>(CONFIRMATION_TEXT);
	cancelLogText = const_cast<char*>(CANCEL_LOG_TEXT);
	startLogText = const_cast<char*>(START_LOG_TEXT);

	DetectionSubsystem::getSingleton().addReceiver(this);
}

LogicSearching::~LogicSearching()
{

}


void LogicSearching::onMessage(Message message)
{
	switch (message.msg)
	{
		case MainConfirmation::CONFIRMATION_MESSAGE_RESULT:
			if (message.par2 == MainConfirmation::CONFIRMATION_OWNER_1)
			{
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
					MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_OTMENA_SIGNALA_O_VOZGORANII, 0, initSignal);
					stop();
					Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, cancelLogText, START_ACTOR_HALF_AUTO, initSignal);
				}
			}
			else
			{
				if (message.par1 == MainConfirmation::CONFIRMATION_RESULT_YES)
				{
					Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(LOG_START_EXTINGUISHING_TEXT), START_ACTOR_HALF_AUTO, 0);
					IOSubsystem::getSingleton().enableAllFireAlarmOutputs();
					IOSubsystem::getSingleton().enableAllHardwareOutputs();

					//SAFE_DELETE_ARRAY(programs)

					calcProgram(&fireCount, localFires, &fire, &programs);



//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("PHASE_SEND_PRESSURE_TABLE\n");	

					phaseSendPressureTable_Start(fireCount, programs);
					phase = PHASE_SEND_PRESSURE_TABLE;
					SAFE_DELETE_ARRAY(listProgramIndex);
				}
				else
				{
					SAFE_DELETE_ARRAY(listProgramIndex);
					finishActor = FINISH_ACTOR_BUTTON;
					Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(LOG_CANCEL_EXTINGUISHING_TEXT), finishActor, 0);
					stop();
				}
			}
			break;
		case MainFinish::FINISH_MESSAGE_RESULT:
			finish(FINISH_ACTOR_BUTTON);
			break;
		case DetectionSubsystem::DETECTION_MANAGER_MESSAGE_CHANNELS_UPDATED:
			if (phase == PHASE_WAITING_FIRE)
			{
				SAFE_DELETE_ARRAY(listProgramIndex);
				listProgramIndex = 0;

				if (message.par1 == DetectionSubsystem::DETECTION_FAULT)
				{
					finishActor = FINISH_ACTOR_LOGIC;
					Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(LOG_NOT_FIRE_DETECT_TEXT), finishActor, 0);
					stop();
				}
				else
				{
					fireCount = DetectionSubsystem::getSingleton().getFire(&localFires, &fire);
					if (fireCount != 0)
					{
						if (UI::getSingleton().getUsoModeControl()->getMode() == UsoModeControl::USO_MODE_HALF_AUTO)
						{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("PHASE_WAITING_CONFIRMATION _ FIRE\n");	
							sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, LOGIC_MESSAGE_GET_CONFIRMATION, reinterpret_cast<unsigned int>(CONFIRMATION_EXTINGUISHING_TEXT), MainConfirmation::CONFIRMATION_OWNER_2));
							phase = PHASE_WAITING_CONFIRMATION;
						}
						else
						{
							Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(LOG_START_EXTINGUISHING_TEXT), START_ACTOR_FULL_AUTO, 0);
							IOSubsystem::getSingleton().enableAllFireAlarmOutputs();
							IOSubsystem::getSingleton().enableAllHardwareOutputs();

							//SAFE_DELETE_ARRAY(programs)
							calcProgram(&fireCount, localFires, &fire, &programs);
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("PHASE_SEND_PRESSURE_TABLE\n");	

							phaseSendPressureTable_Start(fireCount, programs);
							phase = PHASE_SEND_PRESSURE_TABLE;
							SAFE_DELETE_ARRAY(listProgramIndex);
						}
					}			
					else
					{
						finishActor = FINISH_ACTOR_LOGIC;
						Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(LOG_NOT_FIRE_DETECT_TEXT), finishActor, 0);
						stop();
					}
				}
			}
			break;
	}
}

bool LogicSearching::start()
{
	DetectionSubsystem::getSingleton().searchFire(listProgramIndex, listProgramIndexCount);
	phase = PHASE_WAITING_FIRE;

//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("LogicSearching::start\n"); 
	
	return true;
}

void LogicSearching::stop(bool msg)
{
	timeOutBeforeStart = -1;
	finishTimer = -1;
	SAFE_DELETE_ARRAY(listProgramIndex)
	listProgramIndexCount = 0;
	setInitSignalIgnorable(initSignal, true);
	sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, MainTabControl::MAIN_TAB_MESSAGE_SET_MAIN_TAB, 0, 0));
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("LogicSearching::stop: \n"); 
	if (msg)
	{
		Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(FINISH_LOG_TEXT), finishActor, 0);
		MonitoringSubsystem::getSingleton().createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER_STOP_POISKA_OROSHENIA, finishActor, 0);
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING(const_cast<char*>(FINISH_LOG_TEXT)); 
	}
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("\n"); 

	for (unsigned int i = 0; i < listCoolingStartCount * 2; i++)
		if (actionStartList[i] != nullptr)
			SAFE_DELETE(actionStartList[i])

	listCoolingStartCount = 0;
	SAFE_DELETE_ARRAY(actionStartList)

	for (unsigned int i = 0; i < listOffCount * 2; i++)
		if (actionStopList[i] != nullptr)
			SAFE_DELETE(actionStopList[i])

	listOffCount = 0;
	SAFE_DELETE_ARRAY(actionStopList)

	SAFE_DELETE_ARRAY(listCoolingStartIndex)
	SAFE_DELETE_ARRAY(listOff)
	
	SAFE_DELETE_ARRAY(listProgramIndex)

	if (actionCount != 0)
	{
		for (unsigned int i = 0; i < actionCount; i++)
			SAFE_DELETE(actionList[i])
	}

	actionCount = 0;

	SAFE_DELETE_ARRAY(actionList)

	isCoolingStart = false;

	UI::getSingleton().getUsoModeControl()->unLock();
	phase = PHASE_INPUT_CONTROL;
}

void LogicSearching::action()
{
	for (unsigned int i = 0; i < actionCount; i++)
		if (actionList[i] != nullptr)
			actionList[i]->step();

	switch (phase)
	{
		case PHASE_INPUT_CONTROL:
			initSignal = getActiveInitialSignal(LOGIC_FUNCTION_SEARCHING);
			if (initSignal != -1)
			{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("initSignal", initSignal);
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
		case PHASE_SEND_PRESSURE_TABLE:
			if (phaseSendPressureTable_Execution())
			{
				if (testTotalError())
				{
					finishActor = FINISH_ACTOR_LOGIC;
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2(const_cast<char*>(LOG_FAULT_TEXT), finishActor, PHASE_SEND_PRESSURE_TABLE);
					Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, const_cast<char*>(LOG_FAULT_TEXT), finishActor, PHASE_SEND_PRESSURE_TABLE);
					stop();
				}
				else
				{
					phaseStartProgram_Start();
					phase = PHASE_START_PROGRAM;
				}
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
			coolingAction();
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
		case PHASE_COOLING_STOP:
			if (phaseCoolingStopProgram_Execution())
			{
				phase = PHASE_COOLING_START;
				phaseCoolingStartProgram_Start();
			}
			break;
		case PHASE_COOLING_START:
			if (phaseCoolingStartProgram_Execution())
			{
				phase = PHASE_WAITING_STOP;
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("phase = PHASE_WAITING_STOP\n"); 
			}
			break;	
		case PHASE_COOLING_END:
			if (phaseCoolingEnd_Execution())
			{
				phaseGateClose_Start();
				phase = PHASE_CLOSE_GATE;
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("phase PHASE_COOLING_END -->\n"); 
			}
			break;	
	}
}

void LogicSearching::finish(FINISH_ACTOR _finishActor)
{
	_asm cli
//	if ((phase == PHASE_WAITING_STOP) && (охлаждение выключено))
	if (phase == PHASE_WAITING_STOP)
	{
		finishTimer = -1;
		if (coolingSignal == -1)
			phase = PHASE_CLOSE_GATE;
		else
			phase = PHASE_COOLING_END;
		finishActor = _finishActor;
		sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, MainFinish::FINISH_MESSAGE_LABEL, MainFinish::FINISH_MESSAGE_PARAM_FINISH, 0));
		_asm sti
		if (coolingSignal == -1)
			phaseGateClose_Start();
		else
			phaseCoolingEnd_Start();
	}
	_asm sti
}

bool LogicSearching::phaseSendPressureTable_Start(unsigned int _fireCount, Fire::FireScanProgram* _programs)
{
	actionCount = _fireCount; 
	actionList = new Action*[actionCount];

	for (unsigned int i = 0; i < actionCount; i++)
		actionList[i] = new ActionSendPressureTable(&_programs[i]);
	
	return true;
}

bool LogicSearching::phaseSendPressureTable_Execution()
{
	for (unsigned int i = 0; i < actionCount; i++)
		if (actionList[i]->getState() == Action::STATE_UNDEFINED)
			return false;

	unsigned int _actionCount = actionCount;
	for (unsigned int i = 0; i < _actionCount; i++)
		switch (actionList[i]->getState())
		{
			case Action::STATE_ERROR:
				deleteActionInList(i);
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseSendPressureTable_Execution_ERROR ", i, actionCount);
				break;
			case Action::STATE_READY:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseSendPressureTable_Execution_READY ", i, actionCount);
				break;
		}	

	return true;
}

bool LogicSearching::phaseStopProgram_Start()
{
	actionCount = listProgramIndex[0]; 
	actionList = new Action*[actionCount];

	ConfigDataStructProgram** sp = Config::getSingleton().getConfigData()->getConfigDataStructPrograms();

	for (unsigned int i = 0; i < actionCount; i++)
	{
		unsigned char addr = Config::getSingleton().getConfigData()->getPRAddressByNumber(sp[listProgramIndex[i]]->prNumber);
		actionList[i] = new ActionStopProgramScan(addr);
	}

	return true;
}

bool LogicSearching::phaseStopProgram_Execution()
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

bool LogicSearching::phaseStartProgram_Start()
{
	sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, LOGIC_MESSAGE_GET_FINISH, 0, 0));

	for (unsigned int i = 0; i < actionCount; i++)
	{
		if ((actionList[i] != nullptr) && (actionList[i]->getState() != Action::STATE_ERROR))
		{
			Fire::FireScanProgram* prg = reinterpret_cast<ActionSendPressureTable*>(actionList[i])->getProgram();
			SAFE_DELETE(actionList[i])
			actionList[i] = new ActionStartProgramScanLine(prg->point1,	prg->point2, prg->nasadokPosition, SCAN_PROGRAM_BALLISTICS_ON, prg->prNumber);
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("phaseStartProgram:\n  "); 
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("prNumber: ", prg->prNumber); 
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("  point1: ", prg->point1.x); 
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("          ", prg->point1.y); 
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("  point2: ", prg->point2.x); 
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("          ", prg->point2.y); 
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("  nasadokPosition: ", prg->nasadokPosition); 
		}
	}

	return true;
}

bool LogicSearching::phaseStartProgram_Execution()
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

bool LogicSearching::phaseGateOpen_Start()
{
	for (unsigned int i = 0; i < actionCount; i++)
	{
		unsigned char deviceAddress;
		if ((actionList[i] != nullptr) && (actionList[i]->getState() != Action::STATE_ERROR))
		{
			deviceAddress = actionList[i]->getDeviceAddress();
			SAFE_DELETE(actionList[i])
		}

		actionList[i] = new ActionGateOpen(deviceAddress);
	}

	return true;
}

bool LogicSearching::phaseGateOpen_Execution()
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
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseGateOpen_ERROR: ", i, _actionCount); 
				deleteActionInList(i);
				break;
			case Action::STATE_READY:
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("phaseGateOpen_READY: ", i, _actionCount); 
				break;
		}	
	}

	return true;
}

bool LogicSearching::phaseWaitingStop_Start()
{
	if (actionCount > 1)
	{
		for (unsigned int i = 0; i < actionCount - 1; i++)
			if (actionList[i]->getState() == Action::STATE_ERROR)
				for (unsigned int j = i + 1; j < actionCount; j++)
				{
					if (actionList[j]->getState() == Action::STATE_READY)
					{	
						Action* tAction = actionList[i];
						actionList[i] = actionList[j];
						actionList[j] = tAction;
					}
				}
	}

	sendMessage(Message(MESSAGE_FROM_OFFSET_LOGIC, MainFinish::FINISH_MESSAGE_LABEL, MainFinish::FINISH_MESSAGE_PARAM_START, 0));
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("LogicSearching::phaseWaitingStop_Start\n");

	SAFE_DELETE_ARRAY(listProgramIndex)
	listProgramIndexCount = 0;
	return true;
}

bool LogicSearching::phaseWaitingStop_Execution()
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

bool LogicSearching::phaseGateClose_Start()
{
	IOSubsystem::getSingleton().disableAllFireAlarmOutputs();
	IOSubsystem::getSingleton().disableAllHardwareOutputs();
	IOSubsystem::getSingleton().disableAllPumpStationOutputs();

//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("phaseGateClose_Start: \n"); 

	for (unsigned int i = 0; i < actionCount; i++)
	{
		if (actionList[i] != nullptr)
		{
			unsigned char deviceAddress = actionList[i]->getDeviceAddress();
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("  gate: ", deviceAddress); 
			SAFE_DELETE(actionList[i])
			actionList[i] = new ActionGateClose(deviceAddress);
		}
	}
	return true;
}

bool LogicSearching::phaseGateClose_Execution()
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

bool LogicSearching::phaseStopProgramEnd_Start()
{
	for (unsigned int i = 0; i < actionCount; i++)
	{
		if (actionList[i] != nullptr)
		{
			unsigned char deviceAddress = actionList[i]->getDeviceAddress();
			SAFE_DELETE(actionList[i])
			actionList[i] = new ActionStopProgramScan(deviceAddress);
		}
	}

	return true;
}

bool LogicSearching::phaseStopProgramEnd_Execution()
{
	for (unsigned int i = 0; i < actionCount; i++)
		if (actionList[i]->getState() == Action::STATE_UNDEFINED)
			return false;

	for (unsigned int i = 0; i < actionCount; i++)
	{
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

bool LogicSearching::calcProgram(unsigned int* channelsCount, PreFire* localFires, Fire::FireObject* fire, Fire::FireScanProgram** programs)
{
	ConfigDataStructPRPosition** prp = Config::getSingleton().getConfigData()->getConfigDataStructPRPositions();
	
SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("LogicSearching::calcProgram: ", *channelsCount);
	if (*channelsCount > 1)
	{
		for (unsigned int i = 0; i < *channelsCount - 1; i++)
			for (unsigned int j = i + 1; j < *channelsCount; j++)
			{
				if (fire->massCenter.distance(prp[Config::getSingleton().getConfigData()->getPRIndexByAddress(localFires[i].channel)]->position) > 
					fire->massCenter.distance(prp[Config::getSingleton().getConfigData()->getPRIndexByAddress(localFires[j].channel)]->position))
				{
					PreFire temp = localFires[i];	
					localFires[i] = localFires[j];
					localFires[j] = temp;
				}
			}
	}

for (unsigned int i = 0; i < *channelsCount; i++)
{
	SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("  PR: ", Config::getSingleton().getConfigData()->getPRIndexByAddress(localFires[i].channel));
	Point3<float> pos = prp[Config::getSingleton().getConfigData()->getPRIndexByAddress(localFires[i].channel)]->position;
	SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING3("  centerMassDistance_PR_POSITION: ", pos.x, pos.y, pos.z);
	Point3<float> posF = fire->massCenter;
	SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING3("  centerMassDistance_FIRE_POSITION: ", posF.x, posF.y, posF.z);
	SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("  centerMassDistance: ", static_cast<unsigned int>(fire->massCenter.distance(pos)));
}
	
	if (Config::getSingleton().getConfigData()->getConfigDataStructConst()->maxPR < *channelsCount)
		*channelsCount = Config::getSingleton().getConfigData()->getConfigDataStructConst()->maxPR;

SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("  LogicSearching::calcProgram:channelsCountFinish: ", *channelsCount);

	Fire::calcProgram(*channelsCount, localFires, fire, programs);

	return true;
}

void LogicSearching::coolingAction()
{
	if (!isCoolingStart)
	{
		coolingSignal = getActiveInitialSignal(LOGIC_FUNCTION_COOLING_LINE);//LOGIC_FUNCTION_COOLING_POINT);
		if (coolingSignal != -1)
		{
//Log::getSingleton().add(LOG_MESSAGE_FROM_LOGIC, LOG_MESSAGE_TYPE_INFO, startLogText, 99, coolingSignal);
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("  coolingSignal != -1", coolingSignal);
			if (createCoolingList())
			{
				phaseCoolingStopProgram_Start();

				phase = PHASE_COOLING_STOP;
				isCoolingStart = true;
			}
			else
			{
				Config::getSingleton().getConfigData()->getConfigDataStructInitSignals()[coolingSignal]->ignorable = true;
			}
		}		
	}
}

bool LogicSearching::createCoolingList()
{
	ConfigDataStructProgram** sp = Config::getSingleton().getConfigData()->getConfigDataStructPrograms();
	SAFE_DELETE_ARRAY(listCoolingStartIndex)

		
	unsigned int validActionCount = 0;
	for (unsigned int aci = 0; aci < actionCount; aci++)
	{
		if ((actionList[aci] != nullptr) && (actionList[aci]->getState() != Action::STATE_ERROR))
			validActionCount++;
	}
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("      actionCount_validActionCount: ", actionCount, validActionCount);
		
	listCoolingStartIndex = new unsigned int[listProgramIndexCount];
	listCoolingStartCount = 0;
	SAFE_DELETE_ARRAY(listOff)
	listOff = new unsigned int[listProgramIndexCount];
	listOffCount = 0;

//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("    if (listCoolingProgramIndex != nullptr): ", reinterpret_cast<unsigned int>(listProgramIndex));
	if (listProgramIndex != nullptr)
	{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("createCoolingList: \n");
		for (unsigned i = 0; i < listProgramIndexCount; i++)
		{
			int index = indexPrInActionList(Config::getSingleton().getConfigData()->getPRAddressByNumber(sp[listProgramIndex[i]]->prNumber));

			if (index != -1)
			{
				if (validActionCount > 1)
				{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("   PR IS IN ACTION LIST\n");
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("     listCoolingStartIndex_ADD: ", listProgramIndex[i]);
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("     deleteActionInList_PR: ", actionList[index]->getDeviceAddress());
					listCoolingStartIndex[listCoolingStartCount++] = listProgramIndex[i];
					deleteActionInList(index);
					validActionCount--;
				}
			}
			else
			{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("   PR IS NOT IN ACTION LIST: ", Config::getSingleton().getConfigData()->getPRAddressByNumber(sp[listProgramIndex[i]]->prNumber));
				if ((validActionCount + listCoolingStartCount) < Config::getSingleton().getConfigData()->getConfigDataStructConst()->maxPR)
				{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("   listCoolingStartIndex_ADD: ", listProgramIndex[i]);
					listCoolingStartIndex[listCoolingStartCount++] = listProgramIndex[i];
				}
				else
				{
					if (validActionCount > 1)
					{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("   listCoolingStartIndex_DELETE: ", actionList[0]->getDeviceAddress());
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("   listCoolingStartIndex_ADD: ", listProgramIndex[i]);
						listOff[listOffCount++] = actionList[0]->getDeviceAddress();
						deleteActionInList(0);
						validActionCount--;
						listCoolingStartIndex[listCoolingStartCount++] = listProgramIndex[i];
					}
					else
					{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("   validActionCount <= 1\n");
					}
				}
			}
		}
	}

	return true;
}

bool LogicSearching::phaseCoolingStopProgram_Start()
{
	actionStopList = new Action*[listOffCount * 2];
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("phaseCoolingStopProgram_Start: \n");
	for (unsigned int i = 0; i < listOffCount; i++)
	{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("   ActionStopProgramScan: ", listOff[i]);
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("   ActionGateClose: ", listOff[i]);
		actionStopList[i * 2] = new ActionStopProgramScan(listOff[i]);
		actionStopList[i * 2 + 1] = new ActionGateClose(listOff[i]);
	}
	return true;
}

bool LogicSearching::phaseCoolingStopProgram_Execution()
{
	for (unsigned int i = 0; i < listOffCount * 2; i++)
		if (actionStopList[i] != nullptr)
			actionStopList[i]->step();

	for (unsigned int i = 0; i < listOffCount * 2; i++)
		if (actionStopList[i]->getState() == Action::STATE_UNDEFINED)
			return false;
	return true;
}

bool LogicSearching::phaseCoolingStartProgram_Start()
{
	if (listOffCount != 0)
	{
		for (unsigned int i = 0; i < listOffCount * 2; i++)
			SAFE_DELETE(actionStopList[i])
	}

	listOffCount = 0;

	SAFE_DELETE_ARRAY(actionStopList)

	actionStartList = new Action*[listCoolingStartCount * 2];

//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("phaseCoolingStartProgram_Start: \n");

	ConfigDataStructProgram** sp = Config::getSingleton().getConfigData()->getConfigDataStructPrograms();
	for (unsigned int i = 0; i < listCoolingStartCount; i++)
	{
		unsigned char addr = Config::getSingleton().getConfigData()->getPRAddressByNumber(sp[listCoolingStartIndex[i]]->prNumber);
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING2("   ActionStartProgramScanPoint: ", addr, sp[listCoolingStartIndex[i]]->nPointProgram);
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("   ActionGateOpen: ", addr);
		//actionStartList[i * 2] = new ActionStartProgramScanPoint(sp[listCoolingStartIndex[i]]->nPointProgram, addr);
actionStartList[i * 2] = new ActionStartProgramScanLine(sp[listCoolingStartIndex[i]]->point1,
	sp[listCoolingStartIndex[i]]->point2, sp[listCoolingStartIndex[i]]->nasadok,
	SCAN_PROGRAM_BALLISTICS_OFF, addr);
		actionStartList[i * 2 + 1] = new ActionGateOpen(addr);
	}

	return true;
}

bool LogicSearching::phaseCoolingStartProgram_Execution()
{
	for (unsigned int i = 0; i < listCoolingStartCount * 2; i++)
		if (actionStartList[i] != nullptr)
			actionStartList[i]->step();

	for (unsigned int i = 0; i < listCoolingStartCount * 2; i++)
		if (actionStartList[i]->getState() == Action::STATE_UNDEFINED)
			return false;

	IOSubsystem::getSingleton().enableAllPumpStationOutputs();
	return true;
}

bool LogicSearching::phaseCoolingEnd_Start()
{
	if (listOffCount != 0)
	{
		for (unsigned int i = 0; i < listOffCount * 2; i++)
			SAFE_DELETE(actionStopList[i])
	}

	listOffCount = 0;

	SAFE_DELETE_ARRAY(actionStopList)

	listOffCount = listCoolingStartCount;
	actionStopList = new Action*[listOffCount * 2];

//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("phaseCoolingEnd_Start: \n");

	for (unsigned int i = 0; i < listOffCount; i++)
	{
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("   CoolingActionStopProgramScan: ", actionStartList[i * 2]->getDeviceAddress());
//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING1("   CoolingActionGateClose: ", actionStartList[i * 2 + 1]->getDeviceAddress());
		actionStopList[i * 2] = new ActionStopProgramScan(actionStartList[i * 2]->getDeviceAddress());
		actionStopList[i * 2 + 1] = new ActionGateClose(actionStartList[i * 2]->getDeviceAddress());
	}

	return true;
}

bool LogicSearching::phaseCoolingEnd_Execution()
{
	for (unsigned int i = 0; i < listOffCount * 2; i++)
		if (actionStopList[i] != nullptr)
			actionStopList[i]->step();

	for (unsigned int i = 0; i < listOffCount * 2; i++)
		if (actionStopList[i]->getState() == Action::STATE_UNDEFINED)
			return false;

	return true;
}
