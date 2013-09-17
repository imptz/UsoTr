#pragma once

#pragma region example
/* ������ �������������:

#include <Thread.h>
#include <iostream>

class A : public Fr::Thread<A>
{
public:
	A()
		:	Thread(&A::f1)
	{}

private:
	Fr::CPointer<A> f1()
	{
		static bool g = false;
	
		if (g)
		{
			g = false;
			std::cout << "void f1() - true" << std::endl;
			return &A::f2;
		}
		else
		{
			g = true;
			std::cout << "void f1() - false" << std::endl;
			return &A::f1;
		}
	}


	Fr::CPointer<A> f2()
	{
		static bool g = false;
	
		if (g)
		{
			g = false;
			std::cout << "void f2() - true" << std::endl;
			return &A::f1;
		}
		else
		{
			g = true;
			std::cout << "void f2() - false" << std::endl;
			return &A::f2;
		}
	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	A a;

	a.threadAction();
	a.threadAction();
	a.threadAction();
	a.threadAction();
	a.threadAction();
	
	return 0;
}
*/
#pragma endregion

namespace Fr
{
	template<class T>
	struct CPointer;
	
	//! ��������� ������ ��� ��������� �� ����� ������.
	/*!
		���������� �������� ��������� typedef, ������� ���������� ���� ��������� ��������� � ���������. 
	*/
	template<class T>
	struct TFPointer
	{
		typedef CPointer<T> (T::*FPointer)();
	};

	//! ��������� - ��������� �� ����� ������, ����� ���������� ��������� ������ �� ����.
	/*!
		���������� �������� ��� ��������� ����: typedef FPointer (T::*FPointer)()
		������� ������������ typedef CPointer<T> (T::*FPointer)();
	*/
	template <class T>
	struct CPointer
	{
		typedef typename TFPointer<T>::FPointer TFP; ///< ��� ���������� ������

		CPointer(TFP _p)
			:	p(_p)
		{}

		operator TFP()
		{
			return p;
		}

		typename TFP p;
	};

	class IThread
	{
		public:
			virtual void threadAction() = 0;
	};

	//!  Thread class
	/*!
		������� ����� ����� ��� ������������������� ���������� � ������������� �����. 
		������ ������������ ����� ������ ����������� �� Thread.

		� ������� ������ ����������� ������ ����������� �������, ��� ����������� ������ � ������� ������. ����������� ������������ ������ ��������� ��������� ����� ������.
		����������� �������� ����� action, ������� ��������� ����� �� ���������. ��� ���������� ������ �� ����������� ����� �������� ������� ������, �� ������ ����������� ������ 
		����������� � ������� ����������. ���� ����� ���������� ���������, �� �� ���������� ��������� �� ��������� �� ��������� ����� ������, ���� ��������� ��� ���� ����� ������
		� ��������� ������ �������, �� �� ���������� ��������� �� ����.
	*/
	template<class T>
	class Thread : public IThread
	{
		CPointer<T> ptr; ///< ��������� �� ������� ����������� ����� ������

	public:
	    //! A constructor.
		/*!
			���������������� ���������� �� ����� ������ � �������� ���������� ���������� ������.
		*/
		Thread(typename TFPointer<T>::FPointer _ptr = nullptr)
			:	ptr(_ptr)
		{
		}

		virtual ~Thread(void)
		{
		}

	    //! ����� ���������� ������������� ��� ��������� ������
		/*!
			��������� ����� �� ��������� � ��������� ��������� ���������, ������� ������ ����������� �����.
		*/
		virtual void threadAction()
		{
			if (ptr != nullptr)
				ptr = (reinterpret_cast<T*>(this)->*ptr)();
		}
	};
}
