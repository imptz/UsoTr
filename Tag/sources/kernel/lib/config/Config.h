#pragma once

#include "../Singleton.h"
#include "../timer/Timer.h"
#include "../display/display.h"
#include "../message/Messages.h"
#include "../serialport/serialport.h"
#include "ConfigData.h"
#include "trajectory.h"

class Config : public Singleton<Config>, public ITimer, public MessageSender, public MessageReceiver
{
	private:
		static const unsigned int TIMER_PERIOD = 50;
		virtual void timerHandler();

		ConfigData* pConfigData;
		static const unsigned int MAX_PR_DATA_SIZE = 0;

		virtual void onMessage(Message message);

	public:
		Config();
		~Config();

		ConfigData* getConfigData();

	private:
		int progress;
		int progressFR;
		int progressFRStep;
	public:
		int getUpdateProgress();
		int getUpdateProgressFR();

		void startUpdate();
		void stopUpdate();
		void startUpdateFR();
		void stopUpdateFR();

		enum CONFIG_MESSAGE
		{
			CONFIG_MESSAGE_START_DEVICES_UPDATE = 4,
			CONFIG_MESSAGE_UPDATE_FINISH = 5,
			CONFIG_MESSAGE_UPDATE_ERROR = 6,
			CONFIG_MESSAGE_START_DEVICES_UPDATE_FR = 46,
			CONFIG_MESSAGE_UPDATE_FINISH_FR = 47,
			CONFIG_MESSAGE_UPDATE_ERROR_FR = 48
		};

	private:
		SerialPort* serialPort;
		SERIAL_PORT serialPortName;
		SERIAL_PORT_SPEED serialPortSpeed;

		static const unsigned int CONNECT_CODE = 0xfa12c64b;
		static const unsigned int CONNECT_ANSWER_CODE = 0x45dc69a3;
		static const unsigned int DISCONNECT_CODE = 0x8e94ac13;
		static const unsigned int DOWNLOAD_RESULT_OK = 0x00000000;
		static const unsigned int DOWNLOAD_RESULT_FAULT = 0xffffffff;

	public:
		enum CONFIG_PHASE
		{
			CONFIG_PHASE_STOP = 0,
			CONFIG_PHASE_CONNECTION = 1,
			CONFIG_PHASE_LENGTH = 2,
			CONFIG_PHASE_DATA = 3,
			CONFIG_PHASE_DISCONNECT = 4,
			CONFIG_PHASE_DISCONNECT_AFTER_ERROR = 5
		};
	private:
		CONFIG_PHASE phase;

		unsigned int dataSize;
		int loadDataSize;
public:
		static const unsigned int MAX_DATA_SIZE = 1024 * 1024 * 20;
private:
		unsigned char* buffer;
		static const unsigned int LENGTH_AREA_SIZE = 4;
		static const unsigned int CRC_AREA_SIZE = 4;

		void pcTimerHandler();
		void frTimerHandler();

		unsigned char* prNumbers;
		unsigned int prNumbersCount;
		unsigned int prNumbersIndex;

		unsigned int frameId;
		unsigned int frameId1;
		unsigned int frameIdRes;
		unsigned int frameId1Res;

	public:
		void action();
	private:
		void pcAction();

	public:
		void sendMessageToPort(unsigned char* pData, unsigned int size);

		bool debug;

	private:
		enum CONFIG_PHASE_FR
		{
			CONFIG_PHASE_FR_STOP,
			CONFIG_PHASE_FR_CONNECTION,
			CONFIG_PHASE_FR_CONNECTION_WAIT,
			CONFIG_PHASE_FR_TRAJECTORY_START,
			CONFIG_PHASE_FR_TRAJECTORY_START_WAIT,
			CONFIG_PHASE_FR_TRAJECTORY_SET_POINT,
			CONFIG_PHASE_FR_TRAJECTORY_SET_POINT_WAIT,
			CONFIG_PHASE_FR_TRAJECTORY_STOP,
			CONFIG_PHASE_FR_TRAJECTORY_STOP_WAIT,
			CONFIG_PHASE_FR_PRESSURE_START,
			CONFIG_PHASE_FR_PRESSURE_START_WAIT,
			CONFIG_PHASE_FR_PRESSURE_SET1,
			CONFIG_PHASE_FR_PRESSURE_SET1_WAIT,
			CONFIG_PHASE_FR_PRESSURE_SET2,
			CONFIG_PHASE_FR_PRESSURE_SET2_WAIT,
			CONFIG_PHASE_FR_PRESSURE_STOP,
			CONFIG_PHASE_FR_PRESSURE_STOP_WAIT,
		};

		CONFIG_PHASE_FR phaseFR;

		void frAction();
		void frActionError(CONFIG_PHASE_FR phase, unsigned char prNumber);
		void frActionFinish();

		enum COMMAND
		{
			COMMAND_SET_TRAJECTORY = 130
		};

		enum SUBCOMMAND
		{
			SUBCOMMAND_GENERAL_START = 1,
			SUBCOMMAND_TRAJECTORY_START = 2,
			SUBCOMMAND_SET_POINT = 3,
			SUBCOMMAND_TRAJECTORY_STOP = 4,
			SUBCOMMAND_PRESSURE_TABLE_START = 5,
			SUBCOMMAND_SET_PRESSURE_TABLE = 6,
			SUBCOMMAND_PRESSURE_TABLE_STOP = 7
		};

		unsigned int numberPrWithTrajectories;
		unsigned char* prWithTrajectories;
		
		unsigned int prIndex;
		
		unsigned int numberTrajectories;
		unsigned int trajectoryIndex;
		unsigned int getNextTrajectoryIndex();
		static const unsigned int TRAJECTORY_COUNT = 254;
		Trajectory* pTrajectory;
		unsigned int trajectoryCount;
		unsigned int pointCount;
		PressureTable* pPressureTable;
		unsigned int pressureTableCount;
		unsigned int pressureTableCountc;
		unsigned int pressureTableIndex;
		unsigned int totalPoints;
		unsigned int accumPointsCount;

		unsigned int getNumberPrWithTrajectories(unsigned char** ppPrWithTrajectories);
		unsigned int getTrajectoriesForPr(unsigned char prNumber);
		unsigned int getPressureTableIndexForPr(unsigned int startIndex, unsigned int prNumber);
		unsigned int getPressureTableIndex(unsigned int startIndex, unsigned int arNumber);
		unsigned int getPressureTableCountForPr(unsigned int prNumber);
		unsigned int getPressureTableCrc(unsigned int prNumber);

		void configPhaseFrConnection();
		void configPhaseFrConnectionWait();
		void configPhaseFrTrajectoryStart();
		void configPhaseFrTrajectoryStartWait();
		void configPhaseFrTrajectorySetPoint();
		void configPhaseFrTrajectorySetPointWait();
		void configPhaseFrTrajectoryStop();
		void configPhaseFrTrajectoryStopWait();
		void configPhaseFrPressureStart();
		void configPhaseFrPressureStartWait();
		void configPhaseFrPressureSet1();
		void configPhaseFrPressureSet1Wait();
		void configPhaseFrPressureSet2();
		void configPhaseFrPressureSet2Wait();
		void configPhaseFrPressureStop();
		void configPhaseFrPressureStopWait();

		enum TRAJECTORY_RESULT
		{
			TRAJECTORY_RESULT_OK = 0,
			TRAJECTORY_RESULT_ERROR_SIZE = 1,
			TRAJECTORY_RESULT_ERROR_PARAM = 2,
			TRAJECTORY_RESULT_ERROR_FLASH = 3,
			TRAJECTORY_RESULT_ERROR_CRC = 4,
			TRAJECTORY_RESULT_ERROR_FLASH_MODUL = -1
		};
		bool trajectoryResultErrorAction(unsigned char result);
};
