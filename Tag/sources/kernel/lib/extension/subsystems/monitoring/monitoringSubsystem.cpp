#include "monitoringSubsystem.h"
#include "../../../Local.h"
#include "../../../log/Log.h"

const char* MonitoringSubsystem::MISSING_DEVICE_LOG_TEXT = LOCAL_MONITORINGSYSTEM_MISSING_DEVICE_TEXT;

bool MonitoringSubsystem::addDevice(unsigned char id, unsigned char address)
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (devices[i].id == id)
		{
			for (unsigned int j = 0; j < MAX_DEVICE_COUNT; j++)
				if (pDevices[j] == nullptr)
				{
					pDevices[j] = devices[i].pCreateDevice(address, SUBSYSTEM_TYPE);
					DEBUG_IF_PUT_METHOD("add device: id = %u, index = %u, address = %u\n", id, j, address)
					return true;
				}

			return false;
		}

	return false;
}

void MonitoringSubsystem::testMissingDevices()
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
			{
				Log::getSingleton().add(LOG_MESSAGE_FROM_EXTENSION_SYSTEM, LOG_MESSAGE_TYPE_ERROR, LOCAL_MONITORINGSYSTEM_MISSING_DEVICE_TEXT, SUBSYSTEM_TYPE, devices[i].id);
			}	
		}
	}
}

MonitoringSubsystem::MonitoringSubsystem()
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		pDevices[i] = nullptr;

	ExtensionSystem::getSingleton().regSubsystem(SUBSYSTEM_TYPE, this);

	pTimer->start();	
	SerialDebug::getSingleton().addReceiver(this);
	DEBUG_IF_PUT_METHOD("\n")
}

MonitoringSubsystem::~MonitoringSubsystem()
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			SAFE_DELETE(pDevices[i])
}

void MonitoringSubsystem::onMessage(Message message)
{
	if (message.from == MESSAGE_FROM_OFFSET_SERIAL_DEBUG)
	{
		if (message.msg == SerialDebug::SERIAL_DEBUG_MESSAGE_RECV_COMMAND)
			if (message.par1 == SerialDebug::COMMAND_DEBUG_MONITORINGSUBSYSTEM)
			{
				if (!debug)
				{
					debug = true;
					enableDevicesDebug();
					DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "MonitoringSubSystem DEBUG ON\n")
				}
				else
				{
					debug = false;
					disableDevicesDebug();
					DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "MonitoringSubSystem DEBUG OFF\n")
				}
			}
	}
}

void MonitoringSubsystem::timerHandler()
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			pDevices[i]->timerHandler();
}

void MonitoringSubsystem::action()
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

bool MonitoringSubsystem::putFrame(unsigned char* _pArea, bool isNotTransfer)
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			if (pDevices[i]->putFrame(_pArea, isNotTransfer))
				return true;

	return false;
}

void MonitoringSubsystem::init()
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
}

bool MonitoringSubsystem::isReady()
{
	return (phase == PHASE_START);
}

#pragma region monitoringSpecific
void MonitoringSubsystem::createAndSendMessage(IMonitoringDevice::MESSAGE_NUMBER messageNumber, unsigned char parameter1, unsigned char parameter2, unsigned char parameter3, unsigned char parameter4)
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			pDevices[i]->createAndSendMessage(messageNumber, parameter1, parameter2, parameter3, parameter4);
}
#pragma endregion

void MonitoringSubsystem::enableDevicesDebug()
{
	for (unsigned int j = 0; j < MAX_DEVICE_COUNT; j++)
		if (pDevices[j] != nullptr)
			pDevices[j]->sdebug = true;
}

void MonitoringSubsystem::disableDevicesDebug()
{
	for (unsigned int j = 0; j < MAX_DEVICE_COUNT; j++)
		if (pDevices[j] != nullptr)
			pDevices[j]->sdebug = false;
}

