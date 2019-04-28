#include "stdafx.h"
#include "CppUnitTest.h"
#include "CohenSutherlandCode.h"
#include "Float4MM.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace CppUnitTestExtensionFramework;

namespace UnitTest
{
	TEST_CLASS(TestFloat3)
	{
	public:
		TEST_METHOD(TestGetCohenSutherlandCodeSSE)
		{
			unsigned char intersectCode;
			bool result;

			//all left plane outside
			Float4MM allLeftV[3];
			allLeftV[0] = Float4MM(-5, 1, 1, 4);
			allLeftV[1] = Float4MM(-6, 0, 0, 5);
			allLeftV[2] = Float4MM(-11, 12, 5, 10);

			result = GetCohenSutherlandCodeSSE(allLeftV, &intersectCode);
			Assert::IsFalse(result);
			Assert::AreEqual((unsigned char)0x00, intersectCode);

			//all inside
			Float4MM allInsideV[3];
			allInsideV[0] = Float4MM(-5, 1, 1, 6);
			allInsideV[1] = Float4MM(-6, 0, 0, 7);
			allInsideV[2] = Float4MM(-11, 12, 5, 20);

			result = GetCohenSutherlandCodeSSE(allInsideV, &intersectCode);
			Assert::IsTrue(result);
			Assert::AreEqual((unsigned char)0x00, intersectCode);

			//intersect left, right top
			Float4MM intersectV[3];
			intersectV[0] = Float4MM(-6, 6, 1, 5); //left top
			intersectV[1] = Float4MM(0, 8, 0, 7);	//top 
			intersectV[2] = Float4MM(30, 5, 5, 20);	//right

			result = GetCohenSutherlandCodeSSE(intersectV, &intersectCode);
			Assert::IsTrue(result);
			unsigned char leftRightTopIntersectCode = CS_CODE_LEFT | CS_CODE_RIGHT | CS_CODE_TOP;
			Assert::AreEqual(leftRightTopIntersectCode, intersectCode);

			//intersect bottom, near, far
			intersectV[0] = Float4MM(1, -6, 0, 5); //bottom
			intersectV[1] = Float4MM(0, 5, -6, 7);	//near 
			intersectV[2] = Float4MM(1, 5, 30, 20);	//far

			result = GetCohenSutherlandCodeSSE(intersectV, &intersectCode);
			Assert::IsTrue(result);
			unsigned char bottomNearFarIntersectCode = CS_CODE_BOTTOM | CS_CODE_NEAR | CS_CODE_FAR;
			Assert::AreEqual(bottomNearFarIntersectCode, intersectCode);
		}
	};
}