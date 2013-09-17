#pragma once

#include "action.h"

class ActionDummy : public Action
{
	private:
		virtual void timerHandler();
		virtual void step();
		virtual void onMessage(Message message);

	public:
		ActionDummy();
		virtual ~ActionDummy();
};
