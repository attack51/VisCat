#pragma once

#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CppUnitTestExtensionFramework
{
	class AssertEx
	{
	public:
		inline static void AreEqualWithIn(const float& expected, const float& actual, float within)
		{
			float sub = expected - actual;
			Assert::IsTrue(sub > -within && sub < within);
		}
	};
}