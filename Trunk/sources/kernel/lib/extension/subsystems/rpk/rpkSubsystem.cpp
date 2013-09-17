#include "rpkSubsystem.h"
#include "../../../config/Config.h"
#include "../../../Local.h"

const char* RpkSubsystem::MISSING_DEVICE_LOG_TEXT = LOCAL_IOSYSTEM_MISSING_DEVICE_TEXT;

bool RpkSubsystem::addDevice(unsigned char id, unsigned char address)
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
	{
		if (devices[i].id == id)
		{
			for (unsigned int j = 0; j < MAX_DEVICE_COUNT; j++)
				if (pDevices[j] == nullptr)
				{
					pDevices[j] = devices[i].pCreateDevice(address, SUBSYSTEM_TYPE);
					DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s: ", __FUNCTION__)
					DEBUG_IF_PUT("add device: id = %u, index = %u, address = %u\n", id, j, address)
					return true;
				}

			return false;
		}
	}

	return false;
}

void RpkSubsystem::testMissingDevices()
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
	{
		if (devices[i].pCreateDevice != nullptr)
		{
			bool res = false;
			for (unsigned int j = 0; j < MAX_DEVICE_COUNT; j++)
				if (pDevices[j] != nullptr)
					if (pDevices[j]->getId() == devices[i].id)
					{
						res = true;
						break;
					}
			
			if (!res)
				Log::getSingleton().add(LOG_MESSAGE_FROM_EXTENSION_SYSTEM, LOG_MESSAGE_TYPE_ERROR, LOCAL_IOSYSTEM_MISSING_DEVICE_TEXT, SUBSYSTEM_TYPE, devices[i].id);
		}
	}
}

RpkSubsystem::RpkSubsystem()
	: ISubsystem()
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		pDevices[i] = nullptr;

	ExtensionSystem::getSingleton().regSubsystem(SUBSYSTEM_TYPE, this);

	pTimer->start();
	SerialDebug::getSingleton().addReceiver(this);
	DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s\n", __FUNCTION__)
}

RpkSubsystem::~RpkSubsystem()
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			SAFE_DELETE(pDevices[i])
}

void RpkSubsystem::onMessage(Message message)
{
	if (message.from == MESSAGE_FROM_OFFSET_SERIAL_DEBUG)
	{
		if (message.msg == SerialDebug::SERIAL_DEBUG_MESSAGE_RECV_COMMAND)
			if (message.par1 == SerialDebug::COMMAND_DEBUG_RPKSUBSYSTEM)
			{
				if (!debug)
				{
					debug = true;
					enableDevicesDebug();
					DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "RpkSubSystem DEBUG ON\n")
				}
				else
				{
					debug = false;
					disableDevicesDebug();
					DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "RpkSubSystem DEBUG OFF\n")
				}
			}
	}
}

void RpkSubsystem::timerHandler()
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			pDevices[i]->timerHandler();
}

void RpkSubsystem::action()
{
	bool initResult = true;

	if (pDevices[deviceActionIndex] != nullptr)
		pDevices[deviceActionIndex++]->action();

	if ((deviceActionIndex == MAX_DEVICE_COUNT) || (pDevices[deviceActionIndex] == nullptr))
		deviceActionIndex = 0;

	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
		{
			if (phase == PHASE_INIT)
				if (!pDevices[i]->isReady())
					initResult = false;
		}
		
	if (phase == PHASE_INIT)
		if (initResult)
			phase = PHASE_START;
}

bool RpkSubsystem::putFrame(unsigned char* _pArea, bool isNotTransfer)
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			if (pDevices[i]->putFrame(_pArea, isNotTransfer))
				return true;

	return false;
}

void RpkSubsystem::init()
{
	bool isInit = false;
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
		{
			isInit = true;
			phase = PHASE_INIT;
			pDevices[i]->init();
		}

	if (!isInit)
		phase = PHASE_START;

//SERIAL_DEBUG_ADD_DEBUG_MESSAGE_STRING("RpkSubsystem::init\n");	
}

bool RpkSubsystem::isReady()
{
	return (phase == PHASE_START);
}

unsigned int RpkSubsystem::write(unsigned char* pFrame)
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			return pDevices[i]->write(pFrame);

	return IRpkDevice::BAD_FRAME_ID;
}

IRpkDevice::FRAME_RESULT RpkSubsystem::read(unsigned int id, unsigned char** pFrame)
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			return pDevices[i]->read(id, pFrame);

	return IRpkDevice::FRAME_RESULT_ERROR;
}

void RpkSubsystem::enableDevicesDebug()
{
	for (unsigned int j = 0; j < MAX_DEVICE_COUNT; j++)
		if (pDevices[j] != nullptr)
			pDevices[j]->sdebug = true;
}

void RpkSubsystem::disableDevicesDebug()
{
	for (unsigned int j = 0; j < MAX_DEVICE_COUNT; j++)
		if (pDevices[j] != nullptr)
			pDevices[j]->sdebug = false;
}

