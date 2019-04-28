#include "stdafx.h"
#include "CppUnitTest.h"
#include "Float4MM.h"
#include "Float3.h"
#include "MatrixU.h"
#include "MatrixMM.h"
#include "Side4.h"
#include "VisCatMath.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace CppUnitTestExtensionFramework;

namespace UnitTest
{
	TEST_CLASS(TestFloat4MM)
	{
	public:

		TEST_METHOD(Init)
		{
			Float4MM vec(1,2,3,0);

			Assert::AreEqual(1.0f, vec.X());
			Assert::AreEqual(2.0f, vec.Y());
			Assert::AreEqual(3.0f, vec.Z());
			Assert::AreEqual(0.0f, vec.W());
		}

		TEST_METHOD(InitFromM128)
		{
			__m128 original = _mm_set_ps(0,3,2,1);
			Float4MM vec(original);

			Assert::AreEqual(1.0f, vec.X());
			Assert::AreEqual(2.0f, vec.Y());
			Assert::AreEqual(3.0f, vec.Z());
			Assert::AreEqual(0.0f, vec.W());
		}

		TEST_METHOD(InitFromFloat4MM)
		{
			Float4MM original(1,2,3,0);
			Float4MM vec(original);

			Assert::AreEqual(1.0f, vec.X());
			Assert::AreEqual(2.0f, vec.Y());
			Assert::AreEqual(3.0f, vec.Z());
			Assert::AreEqual(0.0f, vec.W());
		}

		TEST_METHOD(InitFromFloat3)
		{
			Float3 original(1,2,3);
			Float4MM vec(original, 0);

			Assert::AreEqual(1.0f, vec.X());
			Assert::AreEqual(2.0f, vec.Y());
			Assert::AreEqual(3.0f, vec.Z());
			Assert::AreEqual(0.0f, vec.W());

			vec = Float4MM(original, 1);

			Assert::AreEqual(1.0f, vec.X());
			Assert::AreEqual(2.0f, vec.Y());
			Assert::AreEqual(3.0f, vec.Z());
			Assert::AreEqual(1.0f, vec.W());
		}

		TEST_METHOD(Add)
		{
			Float4MM lhs(1,2,3,1);
			Float4MM rhs(4,5,6,1);

			Float4MM addResult = Float4MM::Add(&lhs, &rhs);
			Assert::AreEqual(5.0f, addResult.X());
			Assert::AreEqual(7.0f, addResult.Y());
			Assert::AreEqual(9.0f, addResult.Z());
			Assert::AreEqual(2.0f, addResult.W());
		}

		TEST_METHOD(MulWithFloat4)
		{
			Float4MM lhs(1,2,3,1);
			Float4MM rhs(4,5,6,1);

			Float4MM mulResult = Float4MM::Mul(&lhs, &rhs);
			Assert::AreEqual(4.0f, mulResult.X());
			Assert::AreEqual(10.0f, mulResult.Y());
			Assert::AreEqual(18.0f, mulResult.Z());
			Assert::AreEqual(1.0f, mulResult.W());
		}

		TEST_METHOD(MulWithScalar)
		{
			Float4MM lhs(1,2,3,1);

			Float4MM mulResult = Float4MM::Mul(&lhs, 3.0f);
			Assert::AreEqual(3.0f, mulResult.X());
			Assert::AreEqual(6.0f, mulResult.Y());
			Assert::AreEqual(9.0f, mulResult.Z());
			Assert::AreEqual(3.0f, mulResult.W());
		}

		TEST_METHOD(Dot3)
		{
			Float4MM lhs(1,2,3,1);
			Float4MM rhs(4,5,6,1);

			float dotResult = Float4MM::Dot3(&lhs, &rhs);
			float expectedResult=1*4 + 2*5 + 3*6;
			Assert::AreEqual(expectedResult, dotResult);
		}

		TEST_METHOD(Cross3)
		{
			Float4MM lhs(1,0,0,0);
			Float4MM rhs(0,0,-1,0);

			Float4MM crossResult = Float4MM::Cross3(&lhs, &rhs);
			Float4MM expectedResult = Float4MM(0,1,0,0);

			Assert::AreEqual(expectedResult.X(), crossResult.X());
			Assert::AreEqual(expectedResult.Y(), crossResult.Y());
			Assert::AreEqual(expectedResult.Z(), crossResult.Z());
		}

		TEST_METHOD(Length3)
		{
			Float4MM vec(5, 0, 0, 1);
			float length = Float4MM::Length3(&vec);

			Assert::AreEqual(5.0f, length);
		}

		TEST_METHOD(Normalize3)
		{
			Float4MM vec(5, 0, 0, 1);
			Float4MM normalized = Float4MM::Normalize3(&vec);

			AssertEx::AreEqualWithIn(1.0f, normalized.X(), 0.001f);
			AssertEx::AreEqualWithIn(0.0f, normalized.Y(), 0.001f);
			AssertEx::AreEqualWithIn(0.0f, normalized.Z(), 0.001f);
			AssertEx::AreEqualWithIn(1.0f, normalized.W(), 0.001f);
		}

		TEST_METHOD(XbaseSlope)
		{
			const float Infinite=10000;

			Float4MM v0(0, 0, 0, 0);
			Float4MM v1(5, 10, 20, -2.5f);

			Float4MM slope = Float4MM::XbaseSlope(&v0, &v1);
			AssertEx::AreEqualWithIn(1.0f, slope.X(), 0.001f);
			AssertEx::AreEqualWithIn(2.0f, slope.Y(), 0.001f);
			AssertEx::AreEqualWithIn(4.0f, slope.Z(), 0.001f);
			AssertEx::AreEqualWithIn(-0.5f, slope.W(), 0.001f);

			v1 = Float4MM(0, 10, 20, -2.5f);
			slope = Float4MM::XbaseSlope(&v0, &v1);
			AssertEx::AreEqualWithIn(0.0f, slope.X(), 0.001f);
			Assert::IsTrue(slope.Y() > Infinite);
			Assert::IsTrue(slope.Z() > Infinite);
			Assert::IsTrue(slope.W() < -Infinite);
		}

		TEST_METHOD(YbaseSlope)
		{
			const float Infinite=10000;

			Float4MM v0(0, 0, 0, 0);
			Float4MM v1(5, 10, 20, -2.5f);

			Float4MM slope = Float4MM::YbaseSlope(&v0, &v1);
			AssertEx::AreEqualWithIn(0.5f, slope.X(), 0.001f);
			AssertEx::AreEqualWithIn(1.0f, slope.Y(), 0.001f);
			AssertEx::AreEqualWithIn(2.0f, slope.Z(), 0.001f);
			AssertEx::AreEqualWithIn(-0.25f, slope.W(), 0.001f);

			v1 = Float4MM(5, 0, 20, -2.5f);
			slope = Float4MM::YbaseSlope(&v0, &v1);
			Assert::IsTrue(slope.X() > Infinite);
			AssertEx::AreEqualWithIn(0.0f, slope.Y(), 0.001f);
			Assert::IsTrue(slope.Z() > Infinite);
			Assert::IsTrue(slope.W() < -Infinite);
		}
		
		TEST_METHOD(IsInsideOfLeftPlane)
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,2);
			Float3 up(0,1,0);
			MatrixMM viewMatrix(MatrixU::ViewMatrix(&eye, &at, &up));
			float halfPi = 3.141592f*0.5f;
			MatrixMM projMatrix(MatrixU::ProjectMatrix(halfPi, 1, 1, 100));
			MatrixMM viewProjMatrix = MatrixMM::Mul(&viewMatrix, &projMatrix);

			Float4MM v[4];
			v[0] = Float4MM(-5, 0, 4, 1);
			v[1] = Float4MM(-3, 0, 4, 1);
			v[2] = Float4MM(-3.99f, 0, 4, 1);

			MatrixMM::TransformArray(&viewProjMatrix, v, v, 3);

			Float4MM frac[1];
			Side4 side[1];
			AllSide allSide = Float4MM::GetPlaneSides(v, frac, side, 3, Float4MM::GetLeftPlaneSide4);

			Assert::IsTrue(AllSide::Mixed==allSide);

			Assert::IsFalse(side[0].IsInside(0));
			Assert::IsTrue(side[0].IsInside(1));
			Assert::IsTrue(side[0].IsInside(2));

			AssertEx::AreEqualWithIn(1, frac[0].X(), 0.01f);
			AssertEx::AreEqualWithIn(-1, frac[0].Y(), 0.01f);
			AssertEx::AreEqualWithIn(0, frac[0].Z(), 0.01f);
		}

		TEST_METHOD(IsInsideOfRightPlane)
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,2);
			Float3 up(0,1,0);
			MatrixMM viewMatrix(MatrixU::ViewMatrix(&eye, &at, &up));
			float halfPi = 3.141592f*0.5f;
			MatrixMM projMatrix(MatrixU::ProjectMatrix(halfPi, 1, 1, 100));
			MatrixMM viewProjMatrix = MatrixMM::Mul(&viewMatrix, &projMatrix);

			Float4MM v[4];
			v[0] = Float4MM(5, 0, 4, 1);
			v[1] = Float4MM(3, 0, 4, 1);
			v[2] = Float4MM(3.99f, 0, 4, 1);

			MatrixMM::TransformArray(&viewProjMatrix, v, v, 3);

			Float4MM frac[1];
			Side4 side[1];
			AllSide allSide = Float4MM::GetPlaneSides(v, frac, side, 3, Float4MM::GetRightPlaneSide4);

			Assert::IsTrue(AllSide::Mixed==allSide);

			Assert::IsFalse(side[0].IsInside(0));
			Assert::IsTrue(side[0].IsInside(1));
			Assert::IsTrue(side[0].IsInside(2));

			AssertEx::AreEqualWithIn(1, frac[0].X(), 0.01f);
			AssertEx::AreEqualWithIn(-1, frac[0].Y(), 0.01f);
			AssertEx::AreEqualWithIn(0, frac[0].Z(), 0.01f);
		}

		TEST_METHOD(IsInsideOfTopPlane)
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,2);
			Float3 up(0,1,0);
			MatrixMM viewMatrix(MatrixU::ViewMatrix(&eye, &at, &up));
			float halfPi = 3.141592f*0.5f;
			MatrixMM projMatrix(MatrixU::ProjectMatrix(halfPi, 1, 1, 100));
			MatrixMM viewProjMatrix = MatrixMM::Mul(&viewMatrix, &projMatrix);

			Float4MM v[4];
			v[0] = Float4MM(0, 5, 4, 1);
			v[1] = Float4MM(0, 3, 4, 1);
			v[2] = Float4MM(0, 3.99f, 4, 1);

			MatrixMM::TransformArray(&viewProjMatrix, v, v, 3);

			Float4MM frac[1];
			Side4 side[1];
			AllSide allSide = Float4MM::GetPlaneSides(v, frac, side, 3, Float4MM::GetTopPlaneSide4);

			Assert::IsTrue(AllSide::Mixed==allSide);

			Assert::IsFalse(side[0].IsInside(0));
			Assert::IsTrue(side[0].IsInside(1));
			Assert::IsTrue(side[0].IsInside(2));

			AssertEx::AreEqualWithIn(1, frac[0].X(), 0.01f);
			AssertEx::AreEqualWithIn(-1, frac[0].Y(), 0.01f);
			AssertEx::AreEqualWithIn(0, frac[0].Z(), 0.01f);
		}

		TEST_METHOD(IsInsideOfBottomPlane)
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,2);
			Float3 up(0,1,0);
			MatrixMM viewMatrix(MatrixU::ViewMatrix(&eye, &at, &up));
			float halfPi = 3.141592f*0.5f;
			MatrixMM projMatrix(MatrixU::ProjectMatrix(halfPi, 1, 1, 100));
			MatrixMM viewProjMatrix = MatrixMM::Mul(&viewMatrix, &projMatrix);

			Float4MM v[4];
			v[0] = Float4MM(0, -5, 4, 1);
			v[1] = Float4MM(0, -3, 4, 1);
			v[2] = Float4MM(0, -3.99f, 4, 1);

			MatrixMM::TransformArray(&viewProjMatrix, v, v, 3);

			Float4MM frac[1];
			Side4 side[1];
			AllSide allSide = Float4MM::GetPlaneSides(v, frac, side, 3, Float4MM::GetBottomPlaneSide4);

			Assert::IsTrue(AllSide::Mixed==allSide);

			Assert::IsFalse(side[0].IsInside(0));
			Assert::IsTrue(side[0].IsInside(1));
			Assert::IsTrue(side[0].IsInside(2));

			AssertEx::AreEqualWithIn(1, frac[0].X(), 0.01f);
			AssertEx::AreEqualWithIn(-1, frac[0].Y(), 0.01f);
			AssertEx::AreEqualWithIn(0, frac[0].Z(), 0.01f);
		}

		TEST_METHOD(IsInsideOfNearPlane)
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,2);
			Float3 up(0,1,0);
			MatrixMM viewMatrix(MatrixU::ViewMatrix(&eye, &at, &up));
			float halfPi = 3.141592f*0.5f;
			MatrixMM projMatrix(MatrixU::ProjectMatrix(halfPi, 1, 1, 100));
			MatrixMM viewProjMatrix = MatrixMM::Mul(&viewMatrix, &projMatrix);

			Float4MM v[4];
			v[0] = Float4MM(0, 0, 0, 1);
			v[1] = Float4MM(0, 0, 2, 1);
			v[2] = Float4MM(0, 0, 1.001f, 1);

			MatrixMM::TransformArray(&viewProjMatrix, v, v, 3);

			Float4MM frac[1];
			Side4 side[1];
			AllSide allSide = Float4MM::GetPlaneSides(v, frac, side, 3, Float4MM::GetNearPlaneSide4);

			Assert::IsTrue(AllSide::Mixed==allSide);

			Assert::IsFalse(side[0].IsInside(0));
			Assert::IsTrue(side[0].IsInside(1));
			Assert::IsTrue(side[0].IsInside(2));

			AssertEx::AreEqualWithIn(frac[0].X()+frac[0].Y(), 0, 0.001f);
			AssertEx::AreEqualWithIn(0, frac[0].Z(), 0.001f);
		}

		TEST_METHOD(IsInsideOfFarPlane)
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,2);
			Float3 up(0,1,0);
			MatrixMM viewMatrix(MatrixU::ViewMatrix(&eye, &at, &up));
			float halfPi = 3.141592f*0.5f;
			MatrixMM projMatrix(MatrixU::ProjectMatrix(halfPi, 1, 1, 100));
			MatrixMM viewProjMatrix = MatrixMM::Mul(&viewMatrix, &projMatrix);

			Float4MM v[4];
			v[0] = Float4MM(0, 0, 101, 1);
			v[1] = Float4MM(0, 0, 99, 1);
			v[2] = Float4MM(0, 0, 99.999f, 1);

			MatrixMM::TransformArray(&viewProjMatrix, v, v, 3);

			Float4MM frac[1];
			Side4 side[1];
			AllSide allSide = Float4MM::GetPlaneSides(v, frac, side, 3, Float4MM::GetFarPlaneSide4);

			Assert::IsTrue(AllSide::Mixed==allSide);

			Assert::IsFalse(side[0].IsInside(0));
			Assert::IsTrue(side[0].IsInside(1));
			Assert::IsTrue(side[0].IsInside(2));

			AssertEx::AreEqualWithIn(frac[0].X()+frac[0].Y(), 0, 0.001f);
			AssertEx::AreEqualWithIn(0, frac[0].Z(), 0.001f);
		}

		TEST_METHOD(Lerp)
		{
			Float4MM v0(10, 5, 10, 1);
			Float4MM v1(20, 10, 5, 2);

			Float4MM result = Float4MM::Lerp(&v0, &v1, 0.5f);
			AssertEx::AreEqualWithIn(15.0f, result.X(), 0.001f);
			AssertEx::AreEqualWithIn(7.5f, result.Y(), 0.001f);
			AssertEx::AreEqualWithIn(7.5f, result.Z(), 0.001f);
			AssertEx::AreEqualWithIn(1.5f, result.W(), 0.001f);

			result = Float4MM::Lerp(&v0, &v1, 0.0f);
			AssertEx::AreEqualWithIn(v0.X(), result.X(), 0.001f);
			AssertEx::AreEqualWithIn(v0.Y(), result.Y(), 0.001f);
			AssertEx::AreEqualWithIn(v0.Z(), result.Z(), 0.001f);
			AssertEx::AreEqualWithIn(v0.W(), result.W(), 0.001f);

			result = Float4MM::Lerp(&v0, &v1, 1.0f);
			AssertEx::AreEqualWithIn(v1.X(), result.X(), 0.001f);
			AssertEx::AreEqualWithIn(v1.Y(), result.Y(), 0.001f);
			AssertEx::AreEqualWithIn(v1.Z(), result.Z(), 0.001f);
			AssertEx::AreEqualWithIn(v1.W(), result.W(), 0.001f);
		}

		TEST_METHOD(ClipNearAndLerp)
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,1);
			Float3 up(0,1,0);
			MatrixMM viewMatrix(MatrixU::ViewMatrix(&eye, &at, &up));
			float halfPi = 3.141592f*0.5f;
			MatrixMM projMatrix(MatrixU::ProjectMatrix(halfPi, 1, 1, 100));
			MatrixMM viewProjMatrix = MatrixMM::Mul(&viewMatrix, &projMatrix);

			Float4MM v[4];
			v[0] = Float4MM(1, 0, 2, 1);
			v[1] = Float4MM(-1, 0, 0, 1);
			v[2] = Float4MM(-6, 0, -5, 1);

			MatrixMM::TransformArray(&viewProjMatrix, v, v, 3);

			Float4MM frac[1];
			Side4 side[1];
			AllSide allSide = Float4MM::GetPlaneSides(v, frac, side, 3, Float4MM::GetNearPlaneSide4);

			float v0ToV1Factor = GetLerpFactor(frac[0].X(), frac[0].Y());
			float v0ToV2Factor = GetLerpFactor(frac[0].X(), frac[0].Z());
			Float4MM v0ToV1Pos = Float4MM::Lerp(&v[0], &v[1], v0ToV1Factor);
			Float4MM v0ToV2Pos = Float4MM::Lerp(&v[0], &v[2], v0ToV2Factor);

			//잘리는 값인 z는 평면에 거의 붙어있어야 한다
			AssertEx::AreEqualWithIn(0.0f, v0ToV1Pos.X(), 0.001f);
			AssertEx::AreEqualWithIn(0.0f, v0ToV1Pos.Y(), 0.001f);
			AssertEx::AreEqualWithIn(0.0f, v0ToV1Pos.Z(), 0.000001f);
			AssertEx::AreEqualWithIn(1.0f, v0ToV1Pos.W(), 0.000001f);

			AssertEx::AreEqualWithIn(0.0f, v0ToV2Pos.X(), 0.001f);
			AssertEx::AreEqualWithIn(0.0f, v0ToV2Pos.Y(), 0.001f);
			AssertEx::AreEqualWithIn(0.0f, v0ToV2Pos.Z(), 0.000001f);
			AssertEx::AreEqualWithIn(1.0f, v0ToV2Pos.W(), 0.000001f);
		}

		TEST_METHOD(GetScreenScaleAndShift)
		{
			Float4MM screenScale, screenShift;
			Float4MM::GetScreenScaleAndShift(200, 100, &screenScale, &screenShift);

			Assert::AreEqual(100.0f, screenScale.X());
			Assert::AreEqual(-50.0f, screenScale.Y());
			Assert::AreEqual(1.0f, screenScale.Z());
			Assert::AreEqual(1.0f, screenScale.W());

			Assert::AreEqual(99.5f, screenShift.X());
			Assert::AreEqual(49.5f, screenShift.Y());
			Assert::AreEqual(0.0f, screenShift.Z());
			Assert::AreEqual(0.0f, screenShift.W());
		}

		TEST_METHOD(GetScreenScaleAndShift2)
		{
			Float4MM screenScale2, screenShift2;
			Float4MM::GetScreenScaleAndShift2(200, 100, &screenScale2, &screenShift2);

			Assert::AreEqual(100.0f, screenScale2.X());
			Assert::AreEqual(-50.0f, screenScale2.Y());
			Assert::AreEqual(100.0f, screenScale2.Z());
			Assert::AreEqual(-50.0f, screenScale2.W());

			Assert::AreEqual(100.0f, screenShift2.X());
			Assert::AreEqual(50.0f, screenShift2.Y());
			Assert::AreEqual(100.0f, screenShift2.Z());
			Assert::AreEqual(50.0f, screenShift2.W());
		}

		TEST_METHOD(ClipToNdcArray)
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,1);
			Float3 up(0,1,0);
			MatrixMM viewMatrix(MatrixU::ViewMatrix(&eye, &at, &up));
			float halfPi = 3.141592f*0.5f;
			MatrixMM projMatrix(MatrixU::ProjectMatrix(halfPi, 1, 1, 100));
			MatrixMM viewProjMatrix = MatrixMM::Mul(&viewMatrix, &projMatrix);

			Float4MM v[4];
			v[0] = Float4MM(0, 0, 2, 1);
			v[1] = Float4MM(0, 2, 2, 1);
			v[2] = Float4MM(2, 0, 2, 1);
			v[3] = Float4MM(2, 2, 2, 1);

			MatrixMM::TransformArray(&viewProjMatrix, v, v, 4);

			Float4MM screenScale, screenShift;
			Float4MM::GetScreenScaleAndShift(200, 100, &screenScale, &screenShift);

			Float4MM screenV[4];
			Float4MM::ClipToNdcArray(v, screenV, 4);

			AssertEx::AreEqualWithIn(0.0f, screenV[0].X(), 0.1f);
			AssertEx::AreEqualWithIn(0.0f, screenV[0].Y(), 0.1f);
			AssertEx::AreEqualWithIn(v[0].Z()/v[0].W(), screenV[0].Z(), 0.0001f);
			AssertEx::AreEqualWithIn(1.0f, screenV[0].W(), 0.0001f);

			AssertEx::AreEqualWithIn(0.0f, screenV[1].X(), 0.1f);
			AssertEx::AreEqualWithIn(1.0f, screenV[1].Y(), 0.1f);
			AssertEx::AreEqualWithIn(v[1].Z()/v[1].W(), screenV[1].Z(), 0.0001f);
			AssertEx::AreEqualWithIn(1.0f, screenV[1].W(), 0.0001f);

			AssertEx::AreEqualWithIn(1.0f, screenV[2].X(), 0.1f);
			AssertEx::AreEqualWithIn(0.0f, screenV[2].Y(), 0.1f);
			AssertEx::AreEqualWithIn(v[2].Z()/v[2].W(), screenV[2].Z(), 0.0001f);
			AssertEx::AreEqualWithIn(1.0f, screenV[2].W(), 0.0001f);

			AssertEx::AreEqualWithIn(1.0f, screenV[3].X(), 0.1f);
			AssertEx::AreEqualWithIn(1.0f, screenV[3].Y(), 0.1f);
			AssertEx::AreEqualWithIn(v[3].Z()/v[3].W(), screenV[3].Z(), 0.0001f);
			AssertEx::AreEqualWithIn(1.0f, screenV[3].W(), 0.0001f);
		}

		TEST_METHOD(ClipToScreenArray)
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,1);
			Float3 up(0,1,0);
			MatrixMM viewMatrix(MatrixU::ViewMatrix(&eye, &at, &up));
			float halfPi = 3.141592f*0.5f;
			MatrixMM projMatrix(MatrixU::ProjectMatrix(halfPi, 1, 1, 100));
			MatrixMM viewProjMatrix = MatrixMM::Mul(&viewMatrix, &projMatrix);

			Float4MM v[4];
			v[0] = Float4MM(0, 0, 2, 1);
			v[1] = Float4MM(0, 2, 2, 1);
			v[2] = Float4MM(2, 0, 2, 1);
			v[3] = Float4MM(2, 2, 2, 1);

			MatrixMM::TransformArray(&viewProjMatrix, v, v, 4);

			Float4MM screenScale, screenShift;
			Float4MM::GetScreenScaleAndShift(200, 100, &screenScale, &screenShift);

			Float4MM screenV[4];
			Float4MM::ClipToScreenArray(screenScale, screenShift, v, screenV, 4);

			AssertEx::AreEqualWithIn(99.5f, screenV[0].X(), 0.1f);
			AssertEx::AreEqualWithIn(49.5f, screenV[0].Y(), 0.1f);
			AssertEx::AreEqualWithIn(v[0].Z()/v[0].W(), screenV[0].Z(), 0.0001f);
			AssertEx::AreEqualWithIn(1.0f, screenV[0].W(), 0.0001f);

			AssertEx::AreEqualWithIn(99.5f, screenV[1].X(), 0.1f);
			AssertEx::AreEqualWithIn(-0.5f, screenV[1].Y(), 0.1f);
			AssertEx::AreEqualWithIn(v[1].Z()/v[1].W(), screenV[1].Z(), 0.0001f);
			AssertEx::AreEqualWithIn(1.0f, screenV[1].W(), 0.0001f);

			AssertEx::AreEqualWithIn(199.5f, screenV[2].X(), 0.1f);
			AssertEx::AreEqualWithIn(49.5f, screenV[2].Y(), 0.1f);
			AssertEx::AreEqualWithIn(v[2].Z()/v[2].W(), screenV[2].Z(), 0.0001f);
			AssertEx::AreEqualWithIn(1.0f, screenV[2].W(), 0.0001f);

			AssertEx::AreEqualWithIn(199.5f, screenV[3].X(), 0.1f);
			AssertEx::AreEqualWithIn(-0.5f, screenV[3].Y(), 0.1f);
			AssertEx::AreEqualWithIn(v[3].Z()/v[3].W(), screenV[3].Z(), 0.0001f);
			AssertEx::AreEqualWithIn(1.0f, screenV[3].W(), 0.0001f);
		}

		TEST_METHOD(NdcMinMaxToScreenXY)
		{
			Float4MM ndcMin(-0.25f, -0.5f, 0, 1);
			Float4MM ndcMax(0.75f, 1.0f, 0, 1);

			Float4MM screenScale, screenShift;
			Float4MM::GetScreenScaleAndShift2(200, 200, &screenScale, &screenShift);

			Float4MM screenXY2 = Float4MM::NdcMinMaxToScreenXY(screenScale, screenShift, &ndcMin, &ndcMax);

			Assert::AreEqual(75.0f, screenXY2.X());
			Assert::AreEqual(150.0f, screenXY2.Y());
			Assert::AreEqual(175.0f, screenXY2.Z());
			Assert::AreEqual(0.0f, screenXY2.W());
		}

		TEST_METHOD(ExtractXYZW3)
		{
			Float4MM v[3];
			v[0] = Float4MM(1, 2, 3, 4);
			v[1] = Float4MM(5, 6, 7, 8);
			v[2] = Float4MM(9, 10, 11, 12);

			__m128 x3, y3, z3, w3;

			Float4MM::ExtractXYZW3(v, &x3, &y3, &z3, &w3);

			Assert::AreEqual(x3.m128_f32[0],v[0].X());
			Assert::AreEqual(x3.m128_f32[1],v[1].X());
			Assert::AreEqual(x3.m128_f32[2],v[2].X());
			Assert::AreEqual(x3.m128_f32[3],v[2].X());

			Assert::AreEqual(y3.m128_f32[0],v[0].Y());
			Assert::AreEqual(y3.m128_f32[1],v[1].Y());
			Assert::AreEqual(y3.m128_f32[2],v[2].Y());
			Assert::AreEqual(y3.m128_f32[3],v[2].Y());

			Assert::AreEqual(z3.m128_f32[0],v[0].Z());
			Assert::AreEqual(z3.m128_f32[1],v[1].Z());
			Assert::AreEqual(z3.m128_f32[2],v[2].Z());
			Assert::AreEqual(z3.m128_f32[3],v[2].Z());

			Assert::AreEqual(w3.m128_f32[0],v[0].W());
			Assert::AreEqual(w3.m128_f32[1],v[1].W());
			Assert::AreEqual(w3.m128_f32[2],v[2].W());
			Assert::AreEqual(w3.m128_f32[3],v[2].W());

		}
	};
}