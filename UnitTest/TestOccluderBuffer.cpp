#include "stdafx.h"
#include "CppUnitTest.h"
#include "OccluderBuffer.h"
#include "MinMax.h"
#include "Float3.h"
#include "MatrixU.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace CppUnitTestExtensionFramework;

namespace UnitTest
{		
	class UtilForTest
	{
	public:
		static MatrixU GetViewProjMatrix()
		{
			Float3 eye(0,0,0);
			Float3 at(0,0,1);
			Float3 up(0,1,0);
			MatrixU viewMatrix = MatrixU::ViewMatrix(&eye, &at, &up);
			float halfPi = 3.141592f*0.5f;
			MatrixU projMatrix = MatrixU::ProjectMatrix(halfPi, 1, 1, 100);

			return MatrixU::Mul(&viewMatrix, &projMatrix);
		}
	};

	TEST_CLASS(TestOccluderBuffer)
	{
	public:
		
		TEST_METHOD(Init)
		{
			COccluderBuffer* pOccluderBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W32H16);

			Assert::AreEqual(pOccluderBuffer->GetDepthLevelCount(), 4);
			
			Assert::AreEqual(pOccluderBuffer->GetWidth(0), 32);
			Assert::AreEqual(pOccluderBuffer->GetWidth(1), 16);
			Assert::AreEqual(pOccluderBuffer->GetWidth(2), 8);
			Assert::AreEqual(pOccluderBuffer->GetWidth(3), 4);

			Assert::AreEqual(pOccluderBuffer->GetHeight(0), 16);
			Assert::AreEqual(pOccluderBuffer->GetHeight(1), 8);
			Assert::AreEqual(pOccluderBuffer->GetHeight(2), 4);
			Assert::AreEqual(pOccluderBuffer->GetHeight(3), 2);

			delete pOccluderBuffer;
		}
		
		TEST_METHOD(GetPixel)
		{
			COccluderBuffer* pOccluderBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W32H16);

			pOccluderBuffer->FrameStart();
			pOccluderBuffer->Clear();
			pOccluderBuffer->FrameEnd();

			int width=pOccluderBuffer->GetWidth(0);
			int height=pOccluderBuffer->GetHeight(0);
			for(int y=0; y<height; ++y)
			{
				for(int x=0; x<width; ++x)
				{
					Assert::AreEqual(1.0f, pOccluderBuffer->GetPixel(Point(x, y)));
				}
			}

			delete pOccluderBuffer;
		}

		TEST_METHOD(GetPixelMinMax)
		{
			COccluderBuffer* pOccluderBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W32H16);
			MinMax clearValue(1.0f, 1.0f);

			pOccluderBuffer->FrameStart();
			pOccluderBuffer->Clear();
			pOccluderBuffer->FrameEnd();

			for(int level=0; level<pOccluderBuffer->GetDepthLevelCount(); ++level)
			{
				int width=pOccluderBuffer->GetWidth(level);
				int height=pOccluderBuffer->GetHeight(level);
				for(int y=0; y<height; ++y)
				{
					for(int x=0; x<width; ++x)
					{
						MinMax pixelValue = pOccluderBuffer->GetPixelMinMax(Point(x, y), level);
						Assert::AreEqual(clearValue.min, pixelValue.min);
						Assert::AreEqual(clearValue.max, pixelValue.max);
					}
				}
			}

			delete pOccluderBuffer;
		}

		TEST_METHOD(DrawIndex)
		{
			COccluderBuffer* pOccluderBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W32H16);
			pOccluderBuffer->FrameStart();
			pOccluderBuffer->Clear();

			Float3 vertices[4];
			vertices[0] = Float3(0, 0, 2);
			vertices[1] = Float3(0, 2, 2);
			vertices[2] = Float3(2, 0, 2);
			vertices[3] = Float3(2, 2, 2);

			ushort indices[6] = {0,1,2,2,1,3};
			
			MatrixU viewProjMatrix = UtilForTest::GetViewProjMatrix();
			pOccluderBuffer->SetViewProjMatrix(&viewProjMatrix);

			MatrixU worldMatrix = MatrixU::Identity();
			pOccluderBuffer->DrawIndex(vertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);

			pOccluderBuffer->FrameEnd();

			//32*16
			MinMax minmax = pOccluderBuffer->GetPixelMinMax(Point(15, 8), 0);
			Assert::AreEqual(1.0f, minmax.min);
			Assert::AreEqual(1.0f, minmax.max);

			minmax = pOccluderBuffer->GetPixelMinMax(Point(16, 7), 0);
			Assert::IsTrue(minmax.min < 1.0f);
			Assert::IsTrue(minmax.max < 1.0f);

			//16*8
			minmax = pOccluderBuffer->GetPixelMinMax(Point(7, 4), 1);
			Assert::AreEqual(1.0f, minmax.min);
			Assert::AreEqual(1.0f, minmax.max);

			minmax = pOccluderBuffer->GetPixelMinMax(Point(8, 3), 1);
			Assert::IsTrue(minmax.min < 1.0f);
			Assert::IsTrue(minmax.max < 1.0f);

			//8*4
			minmax = pOccluderBuffer->GetPixelMinMax(Point(3, 2), 2);
			Assert::AreEqual(1.0f, minmax.min);
			Assert::AreEqual(1.0f, minmax.max);

			minmax = pOccluderBuffer->GetPixelMinMax(Point(4, 1), 2);
			Assert::IsTrue(minmax.min < 1.0f);
			Assert::IsTrue(minmax.max < 1.0f);

			//4*2
			minmax = pOccluderBuffer->GetPixelMinMax(Point(1, 1), 3);
			Assert::AreEqual(1.0f, minmax.min);
			Assert::AreEqual(1.0f, minmax.max);

			minmax = pOccluderBuffer->GetPixelMinMax(Point(2, 0), 3);
			Assert::IsTrue(minmax.min < 1.0f);
			Assert::IsTrue(minmax.min < 1.0f);
			
			delete pOccluderBuffer;
		}

		TEST_METHOD(DrawIndexWithCullMode)
		{
			COccluderBuffer* pOccluderBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W32H16);
			pOccluderBuffer->FrameStart();
			pOccluderBuffer->Clear();

			Float3 vertices[4];
			vertices[0] = Float3(0, 0, 2);
			vertices[1] = Float3(0, 2, 2);
			vertices[2] = Float3(2, 0, 2);
			vertices[3] = Float3(2, 2, 2);

			ushort indices[6] = {0,1,2,2,1,3};
			MatrixU viewProjMatrix = UtilForTest::GetViewProjMatrix();
			pOccluderBuffer->SetViewProjMatrix(&viewProjMatrix);

			MatrixU worldMatrix = MatrixU::Identity();
			pOccluderBuffer->DrawIndex(vertices, indices, &worldMatrix, VISCAT_CullMode::CW, 0, 4, 2);

			pOccluderBuffer->FrameEnd();

			//32*16
			MinMax minmax = pOccluderBuffer->GetPixelMinMax(Point(15, 8), 0);
			Assert::AreEqual(1.0f, minmax.min);
			Assert::AreEqual(1.0f, minmax.max);

			minmax = pOccluderBuffer->GetPixelMinMax(Point(16, 7), 0);
			Assert::AreEqual(1.0f, minmax.min);
			Assert::AreEqual(1.0f, minmax.max);

			delete pOccluderBuffer;
		}

		TEST_METHOD(ZCompare)
		{
			COccluderBuffer* pOccluderBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W32H16);
			
			Float3 firstVertices[4];
			firstVertices[0] = Float3(-2, -2, 2);
			firstVertices[1] = Float3(-2, 2, 2);
			firstVertices[2] = Float3(2, -2, 2);
			firstVertices[3] = Float3(2, 2, 2);

			//first 보다 뒤에있음
			Float3 secondVertices[4];
			secondVertices[0] = Float3(-2, -2, 3);
			secondVertices[1] = Float3(-2, 2, 3);
			secondVertices[2] = Float3(2, -2, 3);
			secondVertices[3] = Float3(2, 2, 3);

			//first 보다 앞에있음
			Float3 thirdVertices[4];
			thirdVertices[0] = Float3(-2, -2, 1.5f);
			thirdVertices[1] = Float3(-2, 2, 1.5f);
			thirdVertices[2] = Float3(2, -2, 1.5f);
			thirdVertices[3] = Float3(2, 2, 1.5f);

			ushort indices[6] = {0,1,2,2,1,3};
			
			MatrixU viewProjMatrix = UtilForTest::GetViewProjMatrix();
			pOccluderBuffer->SetViewProjMatrix(&viewProjMatrix);

			MatrixU worldMatrix = MatrixU::Identity();

			pOccluderBuffer->FrameStart();
			{
				pOccluderBuffer->Clear();
				pOccluderBuffer->DrawIndex(firstVertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);
			}
			pOccluderBuffer->FrameEnd();

			//32*16
			float firstDepth = pOccluderBuffer->GetPixel(Point(15, 8));
			Assert::IsTrue(firstDepth < 1.0f);

			
			pOccluderBuffer->FrameStart();
			{
				pOccluderBuffer->Clear();
				pOccluderBuffer->DrawIndex(firstVertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);
				pOccluderBuffer->DrawIndex(secondVertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);
			}
			pOccluderBuffer->FrameEnd();

			float secondDepth = pOccluderBuffer->GetPixel(Point(15, 8));
			Assert::AreEqual(firstDepth, secondDepth);
						
			pOccluderBuffer->FrameStart();
			{
				pOccluderBuffer->Clear();
				pOccluderBuffer->DrawIndex(firstVertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);
				pOccluderBuffer->DrawIndex(secondVertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);
				pOccluderBuffer->DrawIndex(thirdVertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);
			}
			pOccluderBuffer->FrameEnd();
			
			float thirdDepth = pOccluderBuffer->GetPixel(Point(15, 8));
			Assert::IsTrue(firstDepth > thirdDepth);

			delete pOccluderBuffer;
		}

		TEST_METHOD(FillScreen)
		{
			COccluderBuffer* pOccluderBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W32H16);
			pOccluderBuffer->FrameStart();
			pOccluderBuffer->Clear();

			MatrixU viewProjMatrix = UtilForTest::GetViewProjMatrix();

			//화면중간이 view좌표계 기준 10으로 잡음
			Float3 expectedCenter = Float3(0, 0, 10);
			MatrixMM viewProjMatrixMM = MatrixMM(viewProjMatrix);
			Float4MM clipCoordExpectedCenter=MatrixMM::TransformPoint(&viewProjMatrixMM, &expectedCenter);
			float ndcExpectedCenterDepth = clipCoordExpectedCenter.Z()/clipCoordExpectedCenter.W();

			Float3 vertices[4];		

			vertices[0] = Float3(-100, -100, 0);
			vertices[1] = Float3(-100, 100, 0);
			vertices[2] = Float3(100, -100, 10);
			vertices[3] = Float3(100, 100, 10);			

			ushort indices[6] = {0,1,2,2,1,3};
			
			pOccluderBuffer->SetViewProjMatrix(&viewProjMatrix);

			MatrixU worldMatrix = MatrixU::Identity();
			pOccluderBuffer->DrawIndex(vertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);
			pOccluderBuffer->FrameEnd();
			
			for(int screenY=0; screenY<16; ++screenY)
			{
				for(int screenX=0; screenX<32; ++screenX)
				{
					float depth = pOccluderBuffer->GetPixel(Point(screenX, screenY));
					Assert::IsTrue(depth<1.0f);
				}
			}
			
			delete pOccluderBuffer;
		}

		TEST_METHOD(Z_Interpolation)
		{
			COccluderBuffer* pOccluderBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W32H16);
			pOccluderBuffer->FrameStart();
			pOccluderBuffer->Clear();

			MatrixU viewProjMatrix = UtilForTest::GetViewProjMatrix();

			//화면중간이 view좌표계 기준 10으로 잡음
			Float3 vertices[4];			
			vertices[0] = Float3(-5, -10, 15);
			vertices[1] = Float3(-5, 10, 15);
			vertices[2] = Float3(10, -10, 0);
			vertices[3] = Float3(10, 10, 0);
			
			ushort indices[6] = {0,1,2,2,1,3};
			
			pOccluderBuffer->SetViewProjMatrix(&viewProjMatrix);

			MatrixU worldMatrix = MatrixU::Identity();
			pOccluderBuffer->DrawIndex(vertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);
			pOccluderBuffer->FrameEnd();

			//32*16
			float firstDepth = pOccluderBuffer->GetPixel(Point(15, 8));

			vertices[0] = Float3(-2, -2, 12);
			vertices[1] = Float3(-2, 2, 12);
			vertices[2] = Float3(2, -2, 8);
			vertices[3] = Float3(2, 2, 8);

			pOccluderBuffer->FrameStart();
			pOccluderBuffer->Clear();
			pOccluderBuffer->DrawIndex(vertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);
			pOccluderBuffer->FrameEnd();

			float secondDepth = pOccluderBuffer->GetPixel(Point(15, 8));
			AssertEx::AreEqualWithIn(firstDepth, secondDepth, 0.0001f);

			delete pOccluderBuffer;
		}

		TEST_METHOD(BoxIsVisible)
		{
			COccluderBuffer* pOccluderBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W32H16);
			pOccluderBuffer->FrameStart();
			pOccluderBuffer->Clear();

			Float3 vertices[4];
			vertices[0] = Float3(0, 0, 2);
			vertices[1] = Float3(0, 2, 2);
			vertices[2] = Float3(2, 0, 2);
			vertices[3] = Float3(2, 2, 2);

			ushort indices[6] = {0,1,2,2,1,3};
			
			MatrixU viewProjMatrix = UtilForTest::GetViewProjMatrix();
			pOccluderBuffer->SetViewProjMatrix(&viewProjMatrix);

			MatrixU worldMatrix = MatrixU::Identity();
			pOccluderBuffer->DrawIndex(vertices, indices, &worldMatrix, VISCAT_CullMode::CCW, 0, 4, 2);

			pOccluderBuffer->FrameEnd();

			Float3 visibleBoxPoints[8];			
			//윗쪽
			visibleBoxPoints[0] = Float3(-1, 1, 3);
			visibleBoxPoints[1] = Float3(1, 1, 3);
			visibleBoxPoints[2] = Float3(1, 1, 4);
			visibleBoxPoints[3] = Float3(-1, 1, 4);
			//아랫쪽
			visibleBoxPoints[4] = Float3(-1, -1, 3);
			visibleBoxPoints[5] = Float3(1, -1, 3);
			visibleBoxPoints[6] = Float3(1, -1, 4);
			visibleBoxPoints[7] = Float3(-1, -1, 4);
			Assert::IsTrue(pOccluderBuffer->BoxIsVisible(visibleBoxPoints));

			Float3 occludeedBoxPoints[8];
			//윗쪽
			occludeedBoxPoints[0] = Float3(0.5f, 1.5f, 3);
			occludeedBoxPoints[1] = Float3(1.5f, 1.5f, 3);
			occludeedBoxPoints[2] = Float3(1.5f, 1.5f, 4);
			occludeedBoxPoints[3] = Float3(0.5f, 1.5f, 4);
			//아랫쪽
			occludeedBoxPoints[4] = Float3(0.5f, 0.5f, 3);
			occludeedBoxPoints[5] = Float3(1.5f, 0.5f, 3);
			occludeedBoxPoints[6] = Float3(1.5f, 0.5f, 4);
			occludeedBoxPoints[7] = Float3(0.5f, 0.5f, 4);
			Assert::IsFalse(pOccluderBuffer->BoxIsVisible(occludeedBoxPoints));

			Float3 outsideOfRightPlaneBoxPoints[8];
			//윗쪽
			occludeedBoxPoints[0] = Float3(5.0f, 1.5f, 3);
			occludeedBoxPoints[1] = Float3(6.0f, 1.5f, 3);
			occludeedBoxPoints[2] = Float3(6.0f, 1.5f, 4);
			occludeedBoxPoints[3] = Float3(5.0f, 1.5f, 4);
			//아랫쪽
			occludeedBoxPoints[4] = Float3(5.0f, 0.5f, 3);
			occludeedBoxPoints[5] = Float3(6.0f, 0.5f, 3);
			occludeedBoxPoints[6] = Float3(6.0f, 0.5f, 4);
			occludeedBoxPoints[7] = Float3(5.0f, 0.5f, 4);
			Assert::IsFalse(pOccluderBuffer->BoxIsVisible(occludeedBoxPoints));

			Float3 clipNearButVisibleBoxPoints[8];
			//윗쪽
			clipNearButVisibleBoxPoints[0] = Float3(-1, 1, 0);
			clipNearButVisibleBoxPoints[1] = Float3(1, 1, 0);
			clipNearButVisibleBoxPoints[2] = Float3(1, 1, 4);
			clipNearButVisibleBoxPoints[3] = Float3(-1, 1, 4);
			//아랫쪽
			clipNearButVisibleBoxPoints[4] = Float3(-1, -1, 0);
			clipNearButVisibleBoxPoints[5] = Float3(1, -1, 0);
			clipNearButVisibleBoxPoints[6] = Float3(1, -1, 4);
			clipNearButVisibleBoxPoints[7] = Float3(-1, -1, 4);
			Assert::IsTrue(pOccluderBuffer->BoxIsVisible(clipNearButVisibleBoxPoints));

			Float3 outSideOfNearBoxPoints[8];
			//윗쪽
			outSideOfNearBoxPoints[0] = Float3(-1, 1, -2);
			outSideOfNearBoxPoints[1] = Float3(1, 1, -2);
			outSideOfNearBoxPoints[2] = Float3(1, 1, 0);
			outSideOfNearBoxPoints[3] = Float3(-1, 1, 0);
			//아랫쪽
			outSideOfNearBoxPoints[4] = Float3(-1, -1, -2);
			outSideOfNearBoxPoints[5] = Float3(1, -1, -2);
			outSideOfNearBoxPoints[6] = Float3(1, -1, 0);
			outSideOfNearBoxPoints[7] = Float3(-1, -1, 0);
			Assert::IsFalse(pOccluderBuffer->BoxIsVisible(outSideOfNearBoxPoints));

			delete pOccluderBuffer;
		}
	};
}
