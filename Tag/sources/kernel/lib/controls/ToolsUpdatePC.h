#ifndef TOOLS_UPDATE_PC_H 
#define TOOLS_UPDATE_PC_H

#include "Button.h"
#include "../Local.h"
#include "../config/Config.h"

class ToolsUpdatePC : public Control, public ITimer
{
	protected:
        static const unsigned int TIMER_PERIOD = 100;
        virtual void timerHandler();

		static const int WIDTH = 78;
		static const int HEIGHT = 9;
		static const Window::BORDER_STYLE BORDER = Window::BORDER_STYLE_INVISIBLE;

		static const unsigned int BUTTON_WIDTH = 26;
		static const unsigned int BUTTON_HEIGHT = 3;

		Button* exitButton;
		static const char* EXIT_BUTTON_TEXT;

		static const char* LOAD_DATA_TEXT;
		static const char* PROCESS_TEXT;
		static const char* FINISH_TEXT;
		static const char* ERROR_TEXT;

		static char* TOOLS_UPDATE_PC_MESSAGE_FINISH;

		void reboot();

		ToolsUpdatePC();

		int updatePhase;

	public:
		ToolsUpdatePC(unsigned int _positionX, unsigned int _positionY, MessageReceiver* _messageReceiver = nullptr);
		virtual ~ToolsUpdatePC();
		virtual void draw();
		virtual void onMessage(Message message);

		enum TOOLS_UPDATE_PC_MESSAGE
		{
			TOOLS_UPDATE_PC_MESSAGE_EXIT = 21
		};

		void startUpdate();
};

#endif