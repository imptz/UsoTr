/*! \file main.cpp
    \brief �������� ������� main.
    
     � main ��������� ������ Root � ������������ ����� ���������� (Application::start).
*/
#include "../lib/Root.h"

extern "C" void _main()
{
	_asm mov eax,16
	_asm mov fs, ax
	_asm mov gs, ax

	Root root;
	Application::getSingleton().start();
}
