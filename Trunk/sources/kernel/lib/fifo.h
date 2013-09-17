/*! \file fifo.h
    \brief 
    
*/

#pragma once

#include "memory\memory.h"

//! ������� ������������ ���������� ������ � ������ ������� �� ����������� ������ ������� ������ (��� ���������� ������).
enum FIFO_ORDER_FLAG
{
	FIFO_ORDER_R_BEFORE_W = 1, /*!< ��������� ������ ����� ���������� ������ */
	FIFO_ORDER_W_BEFORE_R = 2  /*!< ��������� ������ ����� ���������� ������ */
};

//! ��������� ����� ��������� FIFO
/*!
	����� ���������� �������� ������ � ���� ������ ��� ���������� ����������� �������� ������ � �������� ������.
	��������� �������� T ���������� ��� ������������ ��������.
*/
template<class T> class Fifo
{
private:
	T *data; /*!< ��������� �� ������ ��������� */
	unsigned int size; /*!< ������������ ����� ��������� � ��������� */
	unsigned int rPosition; /*!< ������� (�� 0 �� size - 1) �������� ��� ������  */
	unsigned int wPosition; /*!< ������� (�� 0 �� size - 1) �������� ��� ������  */
public:
	FIFO_ORDER_FLAG order;  /*!< ������� ������������ ���������� ������ � ������  */

	static const int DEFAULT_SIZE = 1000; /*!< ������ ��������� �� ��������� */

	Fifo()
	{}

	/**
	* �������� ����� ��������� �����.
	* @return ������� ��� ��������� ����� �������� � ���������.
	*/
	unsigned int getAvailableWriteSize()
	{
		if (order == FIFO_ORDER_W_BEFORE_R)
			return rPosition - wPosition;
		else
			return rPosition + size - wPosition;
	}

public:

	//! �����������.
	/*!
		������������ ������ ��� �������� ����� ���������

		@param _size ������������ ����� ��������� � ���������
	*/
	Fifo(unsigned int _size)
		:	size(_size), rPosition(0), wPosition(0), order(FIFO_ORDER_R_BEFORE_W)
	{
		data = new T[size];
	}

	//! ����������.
	/*!
		����������� ������ ���������
	*/
	~Fifo()
	{
		if (data != nullptr)
			delete[] data;
	}

	//! ��������� ������� � ���������
	/*!
		��������� ���� �� � ��������� ��������� �����. ���� ����, �� ��������� ��������. ���� ���, �� �� ���������.

		@param value ����������� �������

		@return true - ������� ������� � ���������, false - ������� �� ������� � ���������
	*/
	bool put(T value)
	{
		if (order == FIFO_ORDER_W_BEFORE_R)
			if (wPosition == rPosition)
				return false;

		data[wPosition] = value;
		wPosition++;
		if (wPosition == size)
		{
			wPosition = 0;
			order = FIFO_ORDER_W_BEFORE_R;
		}
		return true;
	}

	//! ��������� ��������� ��������� � ���������
	/*!
		��������� ���� �� � ��������� ��������� ����� (��� ���� ���������). ���� ����, �� ��������� ��� ��������. ���� ���, �� ��� �� ���������.

		@param pData - ��������� �� ��������
		@param length - ����� ���������

		@return true - �������� �������� � ���������, false - �������� �� �������� � ���������
	*/
	bool put(T *pData, unsigned int length = 1)
	{
		if (length > getAvailableWriteSize())
			return false;

		for (unsigned int i = 0; i < length; i++)
			put(pData[i]);
		return true;
	}

	//! �������� ��������� ��������� �� ���������
	/*!
		��������� ������� ��������� ��������� � ��������� � ���������� �� ����� �������������� ����� ���������.

		@param pData - ��������� �� ������� ������ ���� ���������� ���������� ��������
		@param count - ����� ��������� ������� ��������� ��������
		@param remove - true - ������� ���������� �������� �� ���������, false - �� �������

		@return ����� ���������� ���������:
			0, ���� � ��������� ��� ���������; 
			������ count, ���� � ��������� ������ count ���������; 
			count, ���� � ��������� �� ������ count ���������
	*/
	unsigned int get(T* pData, unsigned int count = 1, bool remove = true)
	{
		if (order == FIFO_ORDER_R_BEFORE_W)
		{
			if (wPosition == rPosition)
				return 0;
			else
			{
				unsigned int rSize = wPosition - rPosition;
				
				if (count > rSize)
					count = rSize;

				memcpy(reinterpret_cast<unsigned char*>(pData), data + rPosition, count * sizeof(T));

				if (remove)
					rPosition += count;

				return count;
			}
		}

		unsigned int rSize = size - rPosition;
		
		if (count <= rSize)
		{
			memcpy(reinterpret_cast<unsigned char*>(pData), data + rPosition, count * sizeof(T));

			if (remove)
			{
				rPosition += count;
				if (rPosition == size)
				{
					rPosition = 0;
					order = FIFO_ORDER_R_BEFORE_W;
				}
			}

			return count;
		}

		memcpy(reinterpret_cast<unsigned char*>(pData), data + rPosition, rSize * sizeof(T));

		unsigned int dopCount = 0;
		if (count - rSize > wPosition)
			dopCount = wPosition;
		else
			dopCount = count - rSize;

		memcpy(reinterpret_cast<unsigned char*>(pData) + rSize, data, dopCount * sizeof(T));

		if (remove)
		{
			rPosition = dopCount;
			order = FIFO_ORDER_R_BEFORE_W;
		}

		return rSize + dopCount;
	}

	//! �������� ��������� �� �������.
	/*!
		@return true - � ��������� ��� ���������, false - � ��������� ���� ���� �� ���� �������.
	*/
	bool isEmpty()
	{
		if (order == FIFO_ORDER_R_BEFORE_W)
			return (rPosition == wPosition);
		else
			return false;
	}

	//! �������� ��������� �� ���������.
	/*!
		@param count - ���������� ��������� ���������
	*/	
	void remove(unsigned int count = 1)
	{
		if (order == FIFO_ORDER_R_BEFORE_W)
		{
			unsigned int rSize = wPosition - rPosition;
			if (count > rSize)
				count = rSize;

			rPosition += count;
			return;
		}

		unsigned int rSize = size - rPosition;
		if (count <= rSize)
		{
			rPosition += count;
			if (rPosition == size)
			{
				rPosition = 0;
				order = FIFO_ORDER_R_BEFORE_W;
			}
			return;
		}

		unsigned int dopCount = 0;
		if (count - rSize > wPosition)
			dopCount = wPosition;
		else
			dopCount = count - rSize;

		rPosition = dopCount;
		order = FIFO_ORDER_R_BEFORE_W;
	}

	//! ������� ���������.
	/*!
		�������� ���� ��������� �� ���������.
	*/	
	void clear()
	{
		rPosition = 0;
		wPosition = 0;
		order = FIFO_ORDER_R_BEFORE_W;
	}

	//! �������� ����� ��������� � ���������.
	/*!
		@return ����� ��������� � ���������.
	*/	
	unsigned int getDataSize()
	{
		if (order == FIFO_ORDER_R_BEFORE_W)
			return (wPosition - rPosition);
		else
			return size - rPosition + wPosition;
	}
};
