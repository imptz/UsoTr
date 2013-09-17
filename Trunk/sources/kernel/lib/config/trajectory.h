#pragma once

#include "../DEBUG/serialDebug.h"

struct TrajectoryPoint
{
	unsigned short number;
	unsigned short x;
	unsigned short y;
	unsigned short nasadok;
	unsigned short pressureTableNumber;

	TrajectoryPoint(unsigned short _number = 0, unsigned int _x = 0, unsigned int _y = 0, unsigned int _nasadok = 0, unsigned int _pressureTableNumber = 0)
		:	number(_number), x(_x), y(_y), nasadok(_nasadok), pressureTableNumber(_pressureTableNumber)
	{
	}
	
	void clear()
	{
		number = 0;
		x = 0;
		y = 0;
		nasadok = 0;
		pressureTableNumber = 0;
	}
};

struct PressureTable
{
	static const unsigned int SIZE = 10;
	unsigned short pressure[SIZE];
	unsigned short angle[SIZE];

	PressureTable()
	{
		for (unsigned int i = 0; i < SIZE; ++i)
		{
			pressure[i] = 0;
			angle[i] = 0;
		}
	}

	unsigned short getCrc()
	{
		unsigned short result = 0;
		for (unsigned int i = 0; i < SIZE; ++i)
			result += (pressure[i] = angle[i]);

		return result;
	}

	void clear()
	{
		for (unsigned int i = 0; i < SIZE; ++i)
		{
			pressure[i] = 0;
			angle[i] = 0;
		}
	}
};

struct Trajectory
{
	bool valid;
	
	static const unsigned int MAX_POINTS_COUNT = 2300;
	TrajectoryPoint points[MAX_POINTS_COUNT];
	unsigned int pointsCount;

	Trajectory()
		:	valid(false), pointsCount(0)
	{
	}

	void addPoint(unsigned short number, unsigned short x, unsigned short y, unsigned short nasadok, unsigned short pressureTableNumber)
	{
		if (pointsCount < MAX_POINTS_COUNT - 1)
		{
			points[pointsCount].number = number;
			points[pointsCount].x = x;
			points[pointsCount].y = y;
			points[pointsCount].nasadok = nasadok;
			points[pointsCount].pressureTableNumber = pressureTableNumber;

			pointsCount++;
		}
	}

	unsigned short getCrc()
	{
		unsigned short result = 0;
		for (unsigned int i = 0; i < pointsCount; ++i)
		{
			result += (points[i].x + points[i].y + points[i].nasadok + points[i].pressureTableNumber);
//			DEBUG_PUT_METHOD("i = %u, crc = %u, x = %u, y = %u, nas = %u, table = %u\n", i, result, points[i].x, points[i].y, points[i].nasadok, points[i].pressureTableNumber)
		}

//		DEBUG_PUT_METHOD("result crc = %u\n", result)
		return result;
	}

	void clear()
	{
		for (unsigned int i = 0; i < MAX_POINTS_COUNT; ++i)
			points[i].clear();

		valid = false;
		pointsCount = 0;
	}
};

