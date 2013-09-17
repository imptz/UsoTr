#include "bk16Device.h"
#include "../ExtensionSystem.h"
#include "../../config/ConfigData.h"
#include "../../config/Config.h"
#include "../../Local.h"
#include "../../DEBUG/serialDebug.h"

bool Bk16Device::registered = false;

Bk16Device::Bk16Device(unsigned char _address, unsigned int _type)
	:	IIODevice(_address, _type)
{
	ConfigData* pConfigData = Config::getSingleton().getConfigData();
	unsigned count = pConfigData->getConfigDataStructIOBk16Count();
	ConfigDataStructIOBk16** bkStruct = pConfigData->getConfigDataStructIOBk16();

	inputsCount = count;
	pInputs = new Input[inputsCount];

	for (unsigned int i = 0; i < count; i++)
		if (bkStruct[i]->outputFunctionGroup != ConfigDataStructIOBk16::OUTPUT_FUNCTION_GROUP_UNDEFINED)
			outputsCount++;

	pOutputs = new Output[outputsCount];

	unsigned int _ouputCount = 0;
	for (unsigned int i = 0; i < count; i++)
	{
		pInputs[i].projectNumber = bkStruct[i]->projectNumber;
		pInputs[i].onDeviceNumber = (bkStruct[i]->bkAddress - 1) * 8 + bkStruct[i]->numberOnDevice - 1;
		pInputs[i].state = INPUT_STATE_UNDEFINED;
		pInputs[i].prevState = INPUT_STATE_UNDEFINED;
		pInputs[i].lock = false;

		if (bkStruct[i]->outputFunctionGroup != ConfigDataStructIOBk16::OUTPUT_FUNCTION_GROUP_UNDEFINED)
		{
			pOutputs[_ouputCount].projectNumber = bkStruct[i]->projectNumber;
			pOutputs[_ouputCount++].onDeviceNumber = (bkStruct[i]->bkAddress - 1) * 8 + bkStruct[i]->numberOnDevice - 1;
		}
	}

	SerialDebug::getSingleton().addReceiver(this);
	ExtensionSystem::getSingleton().addReceiver(this);
	addReceiver(ExtensionSystem::getSingletonPtr());
}

Bk16Device::~Bk16Device()
{
	SAFE_DELETE(pInputs)
	SAFE_DELETE(pOutputs)
}

void Bk16Device::init()
{
	phase = PHASE_CONFIG;
}

bool Bk16Device::isReady()
{
	return (phase == PHASE_START);
}


void Bk16Device::onMessage(Message message)
{
	INPUT_STATE inputStateRes = INPUT_STATE_UNDEFINED;

	if (message.from == MESSAGE_FROM_OFFSET_SERIAL_DEBUG)
		if (message.msg == SerialDebug::SERIAL_DEBUG_MESSAGE_RECV_COMMAND)
			switch (message.par1)
			{
				case SerialDebug::COMMAND_DEBUG_BK16_SET_INPUTS:
					if (getInput(message.par2, &inputStateRes))
						if (inputStateRes == INPUT_STATE_ON)
						{
							setInputAndLock(message.par2, INPUT_STATE_OFF);
							DEBUG_PUT_METHOD("Установлены в состояние ")
							DEBUG_PUT_COLOR(SerialDebug::COLOR_RED, "выключено ")
							DEBUG_PUT("входы: %u\n", message.par2)
						}
						else
						{
							setInputAndLock(message.par2, INPUT_STATE_ON);
							DEBUG_PUT_METHOD("Установлены в состояние ")
							DEBUG_PUT_COLOR(SerialDebug::COLOR_GREEN, "включено ")
							DEBUG_PUT("входы: %u\n", message.par2)
						}
					break;
				case SerialDebug::COMMAND_DEBUG_BK16_RESET_INPUTS:
					resetAllInputs();
					DEBUG_PUT_METHOD("Все входы сброшены\n")
					break;
			}
}

void Bk16Device::timerHandler()
{
	if (actionTimeOut++ == ACTION_TIME_OUT)
	{
		isActionTimeOut = true;
		actionTimeOut = 0;
	}
}

void Bk16Device::action()
{
	if (!disabled)
	{
		switch (phase)
		{
			case PHASE_CONFIG:
				createInitFrame();
				sendMessage(Message(MESSAGE_FROM_OFFSET_EXTENSION_SYSTEM, ExtensionSystem::EXTENSION_SYSTEM_MESSAGE_NEW_SEND_DATA, reinterpret_cast<unsigned int>(fifoFrame), ExtensionSystem::PACKING_OFF));
				phase = PHASE_CONFIG_WAIT;
				break;
			case PHASE_INIT_WAIT:
				if (isActionTimeOut)
				{
					isActionTimeOut = false;
					createGetInitializeFrame();
					sendMessage(Message(MESSAGE_FROM_OFFSET_EXTENSION_SYSTEM, ExtensionSystem::EXTENSION_SYSTEM_MESSAGE_NEW_SEND_DATA, reinterpret_cast<unsigned int>(fifoFrame), ExtensionSystem::PACKING_OFF));
				}
				break;
			case PHASE_START:
			case PHASE_INPUT_WAIT:
				if (isActionTimeOut)
				{
					obrivOffTest(LOCAL_MESSAGE_TEXT_EXT_SVAZ_OFF_BK, address, 0);
					isActionTimeOut = false;
					createGetInputsFrame();
					sendMessage(Message(MESSAGE_FROM_OFFSET_EXTENSION_SYSTEM, ExtensionSystem::EXTENSION_SYSTEM_MESSAGE_NEW_SEND_DATA, reinterpret_cast<unsigned int>(fifoFrame), ExtensionSystem::PACKING_OFF));
					phase = PHASE_INPUT_WAIT;
				}
				break;
		}
	}
}

bool Bk16Device::putFrame(unsigned char* _pArea, bool isNotTransfer)
{
	if (!disabled)
	{
		if (isNotTransfer)
		{
			if (_pArea[0] == address)
			{
				switch (phase)
				{
					case PHASE_CONFIG_WAIT:
					case PHASE_INIT_WAIT:
						phase = PHASE_START;
						break;
					case PHASE_INPUT_WAIT:
						phase = PHASE_START;
						break;
				}
					
				return true;
			}
		}
		else
		{
			if (_pArea[1] == address)
			{
				switch (phase)
				{
					case PHASE_CONFIG_WAIT:
						if (_pArea[2] == COMMAND_INIT)
						{
							phase = PHASE_INIT_WAIT;
						}
						break;
					case PHASE_INIT_WAIT:
						if (_pArea[2] == COMMAND_GET_INITIALIZE_RESULT)
						{
							if (_pArea[5] == INITIALIZE_RESULT_OK)
							{
								if (sdebug)
								{
									DEBUG_IFS_PUT_METHOD("устройство БК16 с адресом = %u инициализировано\n", address)
								}
								phase = PHASE_START;
							}
						}
						break;
					case PHASE_INPUT_WAIT:
						if (_pArea[2] == COMMAND_GET_INPUTS)
						{
							obrivOnTest(LOCAL_MESSAGE_TEXT_EXT_SVAZ_ON_BK, address, 0);
							analizeInputs(_pArea);
						}
						break;
				}

				return true;
			}
		}
	}

	return false;
}


#pragma region createFrames

void Bk16Device::createInitFrame()
{
	ConfigData* pConfigData = Config::getSingleton().getConfigData();
	
	const unsigned int MAX_BOARDS_COUNT = 65;
	bool boards[MAX_BOARDS_COUNT];
	
	for (unsigned int i = 0; i < MAX_BOARDS_COUNT; i++)
		boards[i] = false;

	unsigned count = pConfigData->getConfigDataStructIOBk16Count();
	ConfigDataStructIOBk16** bkStruct = pConfigData->getConfigDataStructIOBk16();

	for (unsigned int i = 0; i < count; i++)
		if (bkStruct[i]->bkAddress < 64)
			boards[bkStruct[i]->bkAddress] = true;

	unsigned char boardsCount = 0;
	for (unsigned int i = 0; i < MAX_BOARDS_COUNT; i++)
		if (boards[i])
			boardsCount++;

	unsigned int dataLength = 1 + boardsCount + boardsCount * 8;
	unsigned char* initData = new unsigned char[dataLength + 7];
	const unsigned int DATA_OFFSET = 5;
	
	initData[0] = address;
	initData[1] = 0;
	initData[2] = COMMAND_INIT;
	initData[3] = dataLength;
	initData[4] = dataLength >> 8;

	initData[DATA_OFFSET + 0] = boardsCount;

	unsigned int c = 1;
	for (unsigned int i = 1; i < MAX_BOARDS_COUNT; i++)
		if (boards[i])
			{
				initData[DATA_OFFSET + c] = i;
				c++;
			}

	for (unsigned int i = 0; i < count; i++)
		initData[DATA_OFFSET + 1 + boardsCount + (bkStruct[i]->bkAddress - 1) * 8 + bkStruct[i]->numberOnDevice - 1] = bkStruct[i]->inputFunctionGroup;

	fifoFrame->put(&initData);
}

void Bk16Device::createGetInitializeFrame()
{
	unsigned char* initData = new unsigned char[7];
	
	initData[0] = address;
	initData[1] = 0;
	initData[2] = COMMAND_GET_INITIALIZE_RESULT;
	initData[3] = 0;
	initData[4] = 0;

	fifoFrame->put(&initData);
}

void Bk16Device::createSetOutputFrame(unsigned int number, OUTPUT_STATE value)
{
	unsigned char* initData = new unsigned char[7 + 5];
	
	initData[0] = address;
	initData[1] = 0;
	initData[2] = COMMAND_SET_OUTPUT;
	initData[3] = 5;
	initData[4] = 0;
	initData[5] = 1;
	initData[6] = 0;
	initData[7] = number;
	initData[8] = number >> 8;
	initData[9] = value;

	fifoFrame->put(&initData);
}

void Bk16Device::createSetOutputsFrame(unsigned int* number, unsigned int count, OUTPUT_STATE value)
{
	unsigned char* initData = new unsigned char[7 + 2 + 3 * count];
	unsigned int dataLength = count * 3 + 2;
	initData[0] = address;
	initData[1] = 0;
	initData[2] = COMMAND_SET_OUTPUT;
	initData[3] = dataLength;
	initData[4] = dataLength >> 8;
	initData[5] = count;
	initData[6] = count >> 8;

	for (unsigned int i = 0; i < count; i++)
	{
		initData[7 + i * 3] = number[i];
		initData[8 + i * 3] = number[i] >> 8;
		initData[9 + i * 3] = value;
	}

	fifoFrame->put(&initData);
}

void Bk16Device::createGetInputsFrame()
{
	unsigned char* initData = new unsigned char[7 + 2 + inputsCount * 2];
	initData[0] = address;
	initData[1] = 0;
	initData[2] = COMMAND_GET_INPUTS;

	unsigned int length = inputsCount * 2 + 2;

	initData[3] = length;
	initData[4] = length >> 8;
	
	initData[5] = inputsCount;
	initData[6] = inputsCount >> 8;

	for (unsigned int i = 0; i < inputsCount; i++)
	{
		initData[7 + i * 2] = pInputs[i].onDeviceNumber;
		initData[8 + i * 2] = pInputs[i].onDeviceNumber >> 8;
	}

	fifoFrame->put(&initData);
}

#pragma endregion

unsigned int Bk16Device::getId()
{
	return ID;
}

bool Bk16Device::setInputAndLock(unsigned int number, INPUT_STATE value)
{
	for (unsigned int i = 0; i < inputsCount; i++)
	{
		if (pInputs[i].projectNumber == number)
		{
			pInputs[i].state = value;
			pInputs[i].lock = true;
			return true;
		}
	}

	return false;
}

void Bk16Device::resetAllInputs()
{
	for (unsigned int i = 0; i < inputsCount; i++)
	{
		pInputs[i].lock = false;
		pInputs[i].state = INPUT_STATE_UNDEFINED;
		pInputs[i].prevState = INPUT_STATE_UNDEFINED;
	}
}

bool Bk16Device::getInput(unsigned int number, INPUT_STATE* value)
{
	for (unsigned int i = 0; i < inputsCount; i++)
	{
		if (pInputs[i].projectNumber == number)
		{
			*value = pInputs[i].state;
			return true;
		}
	}

	return false;
}

bool Bk16Device::isOutputExists(unsigned int number)
{
	for (unsigned int i = 0; i < outputsCount; i++)
		if (pOutputs[i].onDeviceNumber == number)
			return true;

	return false;
}

bool Bk16Device::setOutput(unsigned int number, OUTPUT_STATE value)
{
	if (!disabled)
	{
		for (unsigned int i = 0; i < outputsCount; i++)
		{
			if (isOutputExists(number))
			{
				createSetOutputFrame(number, value);
				sendMessage(Message(MESSAGE_FROM_OFFSET_EXTENSION_SYSTEM, ExtensionSystem::EXTENSION_SYSTEM_MESSAGE_NEW_SEND_DATA, reinterpret_cast<unsigned int>(fifoFrame), ExtensionSystem::PACKING_OFF));
				return true;
			}
		}
	}

	return false;
}

void Bk16Device::setOutputs(unsigned int* numbers, unsigned int count, OUTPUT_STATE value)
{
	if (!disabled)
	{
		unsigned int* _outputs = new unsigned int[count];
		unsigned int _count = 0;

		for (unsigned int i = 0; i < count; i++)
			if (isOutputExists(numbers[i]))
				_outputs[_count++] = numbers[i];


		createSetOutputsFrame(_outputs, _count, value);
		sendMessage(Message(MESSAGE_FROM_OFFSET_EXTENSION_SYSTEM, ExtensionSystem::EXTENSION_SYSTEM_MESSAGE_NEW_SEND_DATA, reinterpret_cast<unsigned int>(fifoFrame), ExtensionSystem::PACKING_OFF));
	}
}

void Bk16Device::analizeInputs(unsigned char* resizableArea)
{
	unsigned char* _pData = resizableArea;
	unsigned int _inCount = _pData[6];
	_inCount = _inCount * 256 + _pData[5];

	unsigned int _messageOffset = _inCount * 3 + 5;

	if (inputsCount < _inCount)
		_inCount = inputsCount;

	bool inputsNew = false;

	for (unsigned int i = 0; i < _inCount; i++)
	{
		unsigned int _onDeviceNumber = _pData[7 + i * 3 + 1]; 
		_onDeviceNumber = _onDeviceNumber * 256 + _pData[7 + i * 3];
		Input* inp = getInputByOnDeviceNumber(_onDeviceNumber);
		if ((inp != nullptr) && (!inp->lock))
		{
			if ((_pData[7 + i * 3 + 2] == 0) || (_pData[7 + i * 3 + 2] > 2))
				inp->state = INPUT_STATE_UNDEFINED;
			else
				inp->state = static_cast<IIODevice::INPUT_STATE>(_pData[7 + i * 3 + 2]);

			if (inp->state != inp->prevState)
				inputsNew = true;
		}
	}

	if (inputsNew)
	{
		DEBUG_IFS_PUT_METHOD("Изменилось состояние входов:\n")

		for (unsigned int i = 0; i < inputsCount; i++)
		{
			if (pInputs[i].state != pInputs[i].prevState)
			{
				pInputs[i].prevState = pInputs[i].state;
				DEBUG_IFS_PUT_METHOD("   projectNumber - %u  onDeviceNumber - %u  state - %u\n", pInputs[i].projectNumber, pInputs[i].onDeviceNumber, pInputs[i].state)
			}
		}
	}

	phase = PHASE_START;
}
