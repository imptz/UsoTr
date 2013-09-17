#ifndef INFO_PANEL_H 
#define INFO_PANEL_H

#include "Panel.h"
#include "UsoModeControl.h"
#include "ClockControl.h"
#include "Label.h"

class InfoPanel : public Control
{
	protected:
		Panel *panel;
		UsoModeControl *usoModeControl;
		ClockControl *clockControl;
		Label* version;

public:
protected:
		static const int PANEL_WIDTH = 80;
		static const int PANEL_HEIGHT = 3;
		static const Window::BORDER_STYLE PANEL_BORDER_STYLE = Window::BORDER_STYLE_DOUBLE;

		static const int DATE_TIME_CONTROL_POSITION_X = 59;
		static const int DATE_TIME_CONTROL_POSITION_Y = 1;

		static const int VERSION_POSITION_X = 43;

		InfoPanel();

	public:
		InfoPanel(unsigned int _positionX, unsigned int _positionY, MessageReceiver* _messageReceiver = nullptr);
		virtual ~InfoPanel();
		virtual void draw();
		virtual void onMessage(Message message);

		enum INFO_PANEL_MESSAGE
		{
			INFO_PANEL_MESSAGE_NEW_USO_MODE = 11
		};

		UsoModeControl* getUsoModeControl();
};

#endif