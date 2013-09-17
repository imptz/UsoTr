#ifndef FIRE_H
#define FIRE_H

#include "../math/math.h"

class Fire
{
	private:
		Fire();
		virtual ~Fire() = 0
		{
		}

	public:
		struct FireObject
		{
			Point3<float> pivotPoint;
			Point3<float> axisX;
			Point3<float> axisY;
			Point3<float> axisZ;
			Point3<float> massCenter;
		};

		struct FireScanProgram
		{
			unsigned int pressureTable[10];
			unsigned int prNumber;
			Point2<unsigned int> point1;
			Point2<unsigned int> point2;
			unsigned int step;
			unsigned int nasadokPosition;
		};
};

#endif


//
//#include "../math/math.h"
//
//class Fire
//{
//	private:
//		Fire();
//		virtual ~Fire() = 0
//		{
//		}
//
//	public:
//		struct FireObject
//		{
//			Point3<float> pivotPoint;
//			Point3<float> axisX;
//			Point3<float> axisY;
//			Point3<float> axisZ;
//			Point3<float> massCenter;
//		};
//
//		struct FireScanProgram
//		{
//			unsigned int pressureTable[10];
//			unsigned int prNumber;
//			Point2<unsigned int> point1;
//			Point2<unsigned int> point2;
//			unsigned int step;
//			unsigned int nasadokPosition;
//		};
//
//	private:
//		static void calcPressureTable(unsigned char prNumber, Fire::FireObject* fire, FireScanProgram* programs, unsigned int i);
//
//	public:
//		static void calcFire(PreFire* preFires, FireObject* pFire, unsigned int preFiresCount);
//		static void calcFireNEW(PreFire* preFires, FireObject* pFire, unsigned int preFiresCount);
//		static void calcProgram(unsigned int fireCount, PreFire* localFires, Fire::FireObject* fire, Fire::FireScanProgram** programs);
//};
//
