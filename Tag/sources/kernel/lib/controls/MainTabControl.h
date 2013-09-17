#ifndef MAIN_TAB_CONTROL_H 
#define MAIN_TAB_CONTROL_H

#include "TabControl.h"
#include "MainConfirmation.h"
#include "MainInfo.h"
#include "MainFinish.h"
#include "../message/Messages.h"

class MainTabControl : public Control
{
	protected:
		TabControl* tab;
		static const unsigned int TAB_COUNT = 3;

		static const unsigned int INFO_TAB = 0;
		static const unsigned int CONFIRMATION_TAB = 1;
		static const unsigned int FINISH_TAB = 2;

		MainInfo* mainInfo;
		MainConfirmation* mainConfirmation;
		MainFinish* mainFinish;
		
		MainTabControl();

	public:
		MainTabControl(unsigned int _positionX, unsigned int _positionY, unsigned int _width, unsigned int _height, MessageReceiver* _messageReceiver = nullptr);
		virtual ~MainTabControl();
		virtual void draw();
		virtual void onMessage(Message message);

		void activateConfirmationTab();
		void setConfirmationText(char* text);
		void setOwner(MainConfirmation::CONFIRMATION_OWNER _owner);

		void activateFinishTab();

		void activateMainTab();

		enum MAIN_TAB_MESSAGE
		{
			MAIN_TAB_MESSAGE_SET_MAIN_TAB = 14
		};
};

#endif