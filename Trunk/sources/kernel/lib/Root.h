/*! \file Root.h
    \brief ������ Root.    
*/
#pragma once

#include "Singleton.h"
#include "interrupt\interrupt.h"
#include "display\display.h"
#include "serialport\serialport.h"
#include "timer\Timer.h"
#include "hdd\hddManager.h"
#include "touchpad\touchpad.h"
#include "Application.h"
#include "message\Messages.h"
#include "log\Log.h"
#include "clock\clock.h"
#include "config\Config.h"
#include "controls\UI.h"

#include "extension\ExtensionSystem.h"
#include "extension\devices\bk16Device.h"
//#include "extension\devices\fv300Device.h"
#include "extension\devices\scannerDevice.h"
//#include "extension\devices\tvDevice.h"
#include "extension\devices\monitoringDevice.h"
#include "extension\devices\rpkDevice.h"

#include "logic\povorotniy.h"

//! ����� �������� ��� ��������� �������.
/*!
	��������� ����� Root �������� ��������� �� ��� ��������� �������, ������� � ������� ��.
*/
class Root : public Singleton<Root>
{
private:
	MemoryAllocator memoryAllocator;
	Display* pDisplay;
	TimerManager* pTimerManager;
	SerialPortManager* pSerialPortManager;
	HddManager* pHddManager;
	Touchpad* pTouchpad;

	SerialDebug* pSerialDebug;

	Log* pLog;
	Clock* pClock;
	Config* pConfig;
	
	Povorotniy* pPovorotniy;

	UI* pUI;

	Application* pApplication;
		
public:
	//! �����������.
	/*!
		�������������� ���������� ����������, ������ �����������, ���������� ���.
		������� ��� ��������� �������.
		������������ ���������� ����������.
		���������� ������ �� ������������ ��� ����� ���������� ����������� ��������� ����� ���������� ����� �������������� � ������������ ���.
	*/
	Root()
	{
		Interrupts::initInterrupts();	
		MessageReceiver::init();
		
		if (Display::getSingletonPtr() == nullptr)
			pDisplay = new Display();

		if (TimerManager::getSingletonPtr() == nullptr)
			pTimerManager = new TimerManager();

		if (SerialPortManager::getSingletonPtr() == nullptr)
			pSerialPortManager = new SerialPortManager();

		if (HddManager::getSingletonPtr() == nullptr)
			pHddManager = new HddManager();

		pSerialDebug = new SerialDebug();
		
		if (Touchpad::getSingletonPtr() == nullptr)
			pTouchpad = new Touchpad();

		_asm sti

		if (Log::getSingletonPtr() == nullptr)
			pLog = new Log();

		if (Clock::getSingletonPtr() == nullptr)
			pClock = new Clock();

		if (Config::getSingletonPtr() == nullptr)
			pConfig = new Config();

		while (!Config::getSingleton().getConfigData()->isReady()){}

		MonitoringDevice::registerDevice();
		Bk16Device::registerDevice();

		bool scannerDev = false;

		//if (Config::getSingleton().getConfigData()->isTvExistsInConfig())
		//	TvDevice::registerDevice();
		//else
		//{
		//	if (Config::getSingleton().getConfigData()->isFv300ExistsInConfig())
		//		Fv300Device::registerDevice();
		//	else
		//	{
//				scannerDev = true;
//				ScannerDevice::registerDevice();
		//	}		
		//}

		RpkDevice::registerDevice();
		
		ExtensionSystem::getSingleton().init();

//		if (scannerDev)
//			ExtensionSystem::getSingleton().forceAddDevice(2, ScannerDevice::ID, 30);

		if (Povorotniy::getSingletonPtr() == nullptr)
			pPovorotniy = new Povorotniy();

		if (UI::getSingletonPtr() == nullptr)
			pUI = new UI();

		if (Application::getSingletonPtr() == nullptr)
			pApplication = new Application();
	}

	//! ����������.
	/*!
		������� ��� ��������� �������.
	*/
	~Root()
	{
		SAFE_DELETE(pApplication)
		SAFE_DELETE(pUI)
		SAFE_DELETE(pPovorotniy)
		if (ExtensionSystem::getSingletonPtr() != nullptr)
			delete ExtensionSystem::getSingletonPtr();
		SAFE_DELETE(pConfig)
		SAFE_DELETE(pClock)

	delete pSerialDebug;

		SAFE_DELETE(pLog)
		SAFE_DELETE(pTouchpad)
		SAFE_DELETE(pHddManager)
		SAFE_DELETE(pSerialPortManager)
		SAFE_DELETE(pTimerManager)
		SAFE_DELETE(pDisplay)
	}
};

