#include "stdafx.h"
#include "CppUnitTest.h"
#include "Float3.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace CppUnitTestExtensionFramework;

namespace UnitTest
{
	TEST_CLASS(TestFloat3)
	{
	public:

		TEST_METHOD(Init)
		{
			Float3 vec(1,2,3);

			Assert::AreEqual(1.0f, vec.x);
			Assert::AreEqual(2.0f, vec.y);
			Assert::AreEqual(3.0f, vec.z);
		}

		TEST_METHOD(Dot)
		{
			Float3 lhs(1,2,3);
			Float3 rhs(4,5,6);

			float dotResult = Float3::Dot(&lhs, &rhs);
			float expectedResult=1*4 + 2*5 + 3*6;
			Assert::AreEqual(expectedResult, dotResult);
		}

		TEST_METHOD(Cross)
		{
			Float3 lhs(1,0,0);
			Float3 rhs(0,0,-1);

			Float3 crossResult = Float3::Cross(&lhs, &rhs);
			Float3 expectedResult = Float3(0,1,0);

			Assert::AreEqual(expectedResult.x, crossResult.x);
			Assert::AreEqual(expectedResult.y, crossResult.y);
			Assert::AreEqual(expectedResult.z, crossResult.z);
		}

		TEST_METHOD(Length)
		{
			Float3 vec(5, 0, 0);
			float length = Float3::Length(&vec);

			Assert::AreEqual(5.0f, length);
		}

		TEST_METHOD(Normalize)
		{
			Float3 vec(5, 0, 0);
			Float3 normalized = Float3::Normalize(&vec);

			AssertEx::AreEqualWithIn(1.0f, normalized.x, 0.001f);
			AssertEx::AreEqualWithIn(0.0f, normalized.y, 0.001f);
			AssertEx::AreEqualWithIn(0.0f, normalized.z, 0.001f);
		}
	};
}