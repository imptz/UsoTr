#include "detectionSubsystem.h"
#include "../../../config/Config.h"
#include "../../../Local.h"
//#include "../../devices/fv300Device.h"
//#include "../../devices/TvDevice.h"
//#include "../../devices/scannerDevice.h"

const char* DetectionSubsystem::MISSING_DEVICE_LOG_TEXT = LOCAL_DETECTIONSYSTEM_MISSING_DEVICE_TEXT;

bool DetectionSubsystem::addDevice(unsigned char id, unsigned char address)
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

void DetectionSubsystem::testMissingDevices()
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
				Log::getSingleton().add(LOG_MESSAGE_FROM_EXTENSION_SYSTEM, LOG_MESSAGE_TYPE_ERROR, LOCAL_DETECTIONSYSTEM_MISSING_DEVICE_TEXT, SUBSYSTEM_TYPE, devices[i].id);
			}	
		}
	}
}

DetectionSubsystem::DetectionSubsystem()
	: Fr::Thread<DetectionSubsystem>(&DetectionSubsystem::qqq)
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		pDevices[i] = nullptr;

	ExtensionSystem::getSingleton().regSubsystem(SUBSYSTEM_TYPE, this);

	pTimer->start();
	SerialDebug::getSingleton().addReceiver(this);
	DEBUG_IF_PUT_COLOR(SerialDebug::COLOR_YELLOW, "%s\n", __FUNCTION__)
}

DetectionSubsystem::~DetectionSubsystem()
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			SAFE_DELETE(pDevices[i])
}

void DetectionSubsystem::onMessage(Message message)
{
	if (message.from == MESSAGE_FROM_OFFSET_SERIAL_DEBUG)
	{
		if (message.msg == SerialDebug::SERIAL_DEBUG_MESSAGE_RECV_COMMAND)
			if (message.par1 == SerialDebug::COMMAND_DEBUG_DETECTIONSUBSYSTEM)
			{
				if (!debug)
				{
					debug = true;
					enableDevicesDebug();
					DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "DetectionSubSystem DEBUG ON\n")
				}
				else
				{
					debug = false;
					disableDevicesDebug();
					DEBUG_PUT_COLOR_ATTR(SerialDebug::COLOR_GREEN, SerialDebug::TEXT_ATTR_BOLD, "DetectionSubSystem DEBUG OFF\n")
				}
			}
	}
}

void DetectionSubsystem::timerHandler()
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			pDevices[i]->timerHandler();
}

bool DetectionSubsystem::putFrame(unsigned char* _pArea, bool isNotTransfer)
{
	for (unsigned int i = 0; i < MAX_DEVICE_COUNT; i++)
		if (pDevices[i] != nullptr)
			if (pDevices[i]->putFrame(_pArea, isNotTransfer))
				return true;

	return false;
}

void DetectionSubsystem::init()
{
	DEBUG_PUT_METHOD("")
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

bool DetectionSubsystem::isReady()
{
	return true;
}

void DetectionSubsystem::action()
{
	threadAction();
}

//#pragma region detectionSpecific

void DetectionSubsystem::searchFire(unsigned int* listProgramIndex, unsigned int count)
{
}

void DetectionSubsystem::searchFireJustirovka(unsigned char address)
{
}

bool DetectionSubsystem::getFireJustirovka()
{
	return true;
}

void DetectionSubsystem::resetDevices()
{
}

//#pragma endregion
void DetectionSubsystem::enableDevicesDebug()
{
	for (unsigned int j = 0; j < MAX_DEVICE_COUNT; j++)
		if (pDevices[j] != nullptr)
			pDevices[j]->sdebug = true;
}

void DetectionSubsystem::disableDevicesDebug()
{
	for (unsigned int j = 0; j < MAX_DEVICE_COUNT; j++)
		if (pDevices[j] != nullptr)
			pDevices[j]->sdebug = false;
}

Fr::CPointer<DetectionSubsystem> DetectionSubsystem::qqq()
{
	return &DetectionSubsystem::qqq;
}

