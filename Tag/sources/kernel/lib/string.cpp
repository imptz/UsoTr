/*! \file string.cpp
    \brief �������� ������� ������ �� �������� � ������� �������� �� �����.
    
*/
#include "string.h"
#include "memory\memory.h"

/**
* ��������� ����� ������.
* @param str ��������� �� ������.
* @return ����� ������
*/
int strlen(char* str)
{
	for (unsigned int i = 0; i < 256; i++)
		if (str[i] == 0)
			return i;

	return -1;
}

int strlen(const char* str)
{
	return strlen(const_cast<char*>(str));
}

/**
* �������� ������.
* @param dest ��������� �� ������ ��������.
* @param src ��������� �� ������ ��������.
*/
void strcpy(char* dest, char* src)
{
	int length = strlen(src);
	memcpy(dest, src, length);
	dest[length] = 0;
}

void strcpy(char* dest, const char* src)
{
	strcpy(dest, const_cast<char*>(src));
}