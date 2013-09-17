#include "MainInfo.h"
#include "../log/Log.h"
#include "../low.h"

#include "Button.h"
#include "../DEBUG/serialDebug.h"

MainInfo::MainInfo(unsigned int _positionX, unsigned int _positionY, MessageReceiver* _messageReceiver)
	:	Control(_positionX, _positionY, WIDTH, HEIGHT, _messageReceiver)
{ 
	draw();
}

MainInfo::~MainInfo()
{

}

void MainInfo::draw()
{
	drawChildControls();
}

void MainInfo::onMessage(Message message)
{
}
