// 
// Результат - найденные очаги: координаты относительно центра зоны поиска + размер очага по горизотали и вертикали.
//
#pragma once

#include "../../devices/IDevice.h"
#include "../../../DEBUG/serialDebug.h"
#include "../../../math/math.h"

class IDetectionDevice : public IDevice
{
protected:
		enum COMMAND
		{
			COMMAND_GET_ID = 1, 
			COMMAND_INIT = 2, 
			COMMAND_GET_INITIALIZE_RESULT = 3,
			COMMAND_SET_UPDATE_LIST = 4,
			COMMAND_GET_FIRE = 5,
			COMMAND_GET_STATE = 6,
			COMMAND_RESET_DEVICES = 7
		};

	public:
		IDetectionDevice(unsigned char _address, unsigned int _type)
			:	IDevice(_address, _type)
		{}

		void init() = 0;
		bool isReady() = 0;

	public:
		struct ChannelInfo
		{
			unsigned int structureItemsCount;
			bool valid;

			unsigned char address;
			Point2<unsigned int> point1;
			Point2<unsigned int> point2;
			unsigned int mode;
		};

		virtual void startUpdateFire(ChannelInfo* pChannelInfo) = 0;
		virtual bool isUpdateComplete() = 0;
		virtual ChannelInfo* getFire() = 0;
		virtual void reset() = 0;
};
