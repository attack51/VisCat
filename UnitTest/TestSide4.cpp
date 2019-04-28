#include "stdafx.h"
#include "CppUnitTest.h"
#include "Side4.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace CppUnitTestExtensionFramework;

namespace UnitTest
{
	TEST_CLASS(TestSide4)
	{
	public:

		TEST_METHOD(IsInside)
		{
			Side4 side(_mm_set_ps(2, -1, 0, 1));
			Assert::IsFalse(side.IsInside(0));
			Assert::IsTrue(side.IsInside(1));
			Assert::IsTrue(side.IsInside(2));
			Assert::IsFalse(side.IsInside(3));
		}

		TEST_METHOD(AllSide)
		{
			//3개만 검사
			Side4 mixed(_mm_set_ps(2, -1, 0, 1));
			Side4 allInside(_mm_set_ps(4, -2, 0, -1));
			Side4 allOutside(_mm_set_ps(-4, 2, 1, 5));

			Assert::IsTrue(AllSide::Mixed==mixed.GetAllSide(3));
			Assert::IsTrue(AllSide::AllInside==allInside.GetAllSide(3));
			Assert::IsTrue(AllSide::AllOutside==allOutside.GetAllSide(3));
		}

		TEST_METHOD(CombinationAllSide)
		{
			//총 10개를 테스트 할거임
			//all inside
			Side4 sides[3];
			sides[0] = Side4(_mm_set_ps(0, -1, -2, -3));
			sides[1] = Side4(_mm_set_ps(-4, -5, -6, -7));
			sides[2] = Side4(_mm_set_ps(100, 100, -8, -9));

			Assert::IsTrue(AllSide::AllInside==Side4::CombinationAllSide(sides, 10));

			//all outside
			sides[0] = Side4(_mm_set_ps(1, 2, 3, 4));
			sides[1] = Side4(_mm_set_ps(5, 6, 7, 8));
			sides[2] = Side4(_mm_set_ps(-100, -100, 9, 10));

			Assert::IsTrue(AllSide::AllOutside==Side4::CombinationAllSide(sides, 10));

			//mixed
			sides[0] = Side4(_mm_set_ps(0, -1, -2, -3));
			sides[1] = Side4(_mm_set_ps(4, 5, 6, 7));
			sides[2] = Side4(_mm_set_ps(100, 100, -8, -9));

			Assert::IsTrue(AllSide::Mixed==Side4::CombinationAllSide(sides, 10));
		}
	};
}