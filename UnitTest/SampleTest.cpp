#include "stdafx.h"
#include "CppUnitTest.h"
#include "TestClass.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(ImportDllVar)
		{
			Assert::AreEqual(nTestVar, 0);
			nTestVar=50;
			Assert::AreEqual(nTestVar, 50);
		}

		TEST_METHOD(ImportDllFunc)
		{
			Assert::AreEqual(fnTestAddFunc(1, 2), 3);
		}

		TEST_METHOD(ImportDllClass)
		{
			CTestClass testClass;
			Assert::AreEqual(testClass.Add(1, 2), 3);
		}

		TEST_METHOD(TestMethod1)
		{
			int a=10;
			int b=20;
			Assert::AreNotEqual(a, b);

			a=20;
			Assert::AreEqual(a, b);
		}
	};
}
