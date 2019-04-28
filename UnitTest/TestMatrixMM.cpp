#include "stdafx.h"
#include "CppUnitTest.h"
#include "MatrixMM.h"
#include "MatrixU.h"
#include "Float3.h"
#include "Float4MM.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{		
	TEST_CLASS(TestMatrixMM)
	{
	public:
		
		TEST_METHOD(Init)
		{
			MatrixMM matrix(	1,2,3,4,
								5,6,7,8,
								9,10,11,12,
								13,14,15,16);

			Assert::AreEqual(1.0f, matrix.M11());
			Assert::AreEqual(2.0f, matrix.M12());
			Assert::AreEqual(3.0f, matrix.M13());
			Assert::AreEqual(4.0f, matrix.M14());

			Assert::AreEqual(5.0f, matrix.M21());
			Assert::AreEqual(6.0f, matrix.M22());
			Assert::AreEqual(7.0f, matrix.M23());
			Assert::AreEqual(8.0f, matrix.M24());
			
			Assert::AreEqual(9.0f, matrix.M31());
			Assert::AreEqual(10.0f, matrix.M32());
			Assert::AreEqual(11.0f, matrix.M33());
			Assert::AreEqual(12.0f, matrix.M34());
			
			Assert::AreEqual(13.0f, matrix.M41());
			Assert::AreEqual(14.0f, matrix.M42());
			Assert::AreEqual(15.0f, matrix.M43());
			Assert::AreEqual(16.0f, matrix.M44());
		}

		TEST_METHOD(InitFromMatrix)
		{
			MatrixU originalMat(1,2,3,4,
								5,6,7,8,
								9,10,11,12,
								13,14,15,16);

			MatrixMM matrix(originalMat);

			Assert::AreEqual(1.0f, matrix.M11());
			Assert::AreEqual(2.0f, matrix.M12());
			Assert::AreEqual(3.0f, matrix.M13());
			Assert::AreEqual(4.0f, matrix.M14());

			Assert::AreEqual(5.0f, matrix.M21());
			Assert::AreEqual(6.0f, matrix.M22());
			Assert::AreEqual(7.0f, matrix.M23());
			Assert::AreEqual(8.0f, matrix.M24());
			
			Assert::AreEqual(9.0f, matrix.M31());
			Assert::AreEqual(10.0f, matrix.M32());
			Assert::AreEqual(11.0f, matrix.M33());
			Assert::AreEqual(12.0f, matrix.M34());
			
			Assert::AreEqual(13.0f, matrix.M41());
			Assert::AreEqual(14.0f, matrix.M42());
			Assert::AreEqual(15.0f, matrix.M43());
			Assert::AreEqual(16.0f, matrix.M44());

			//서로 reference를 공유하고 있지 않음을 확인
			originalMat.m23 = 100.0f;

			Assert::AreNotEqual(originalMat.m23, matrix.M23());
		}

		TEST_METHOD(InitFromMatrixMM)
		{
			MatrixMM originalMat(	1,2,3,4,
									5,6,7,8,
									9,10,11,12,
									13,14,15,16);

			MatrixMM matrix(originalMat);

			Assert::AreEqual(1.0f, matrix.M11());
			Assert::AreEqual(2.0f, matrix.M12());
			Assert::AreEqual(3.0f, matrix.M13());
			Assert::AreEqual(4.0f, matrix.M14());

			Assert::AreEqual(5.0f, matrix.M21());
			Assert::AreEqual(6.0f, matrix.M22());
			Assert::AreEqual(7.0f, matrix.M23());
			Assert::AreEqual(8.0f, matrix.M24());
			
			Assert::AreEqual(9.0f, matrix.M31());
			Assert::AreEqual(10.0f, matrix.M32());
			Assert::AreEqual(11.0f, matrix.M33());
			Assert::AreEqual(12.0f, matrix.M34());
			
			Assert::AreEqual(13.0f, matrix.M41());
			Assert::AreEqual(14.0f, matrix.M42());
			Assert::AreEqual(15.0f, matrix.M43());
			Assert::AreEqual(16.0f, matrix.M44());

			//서로 reference를 공유하고 있지 않음을 확인
			originalMat.SetM23(100.0f);

			Assert::AreNotEqual(originalMat.M23(), matrix.M23());
		}

		TEST_METHOD(InitFromM128)
		{
			__m128 row0 = _mm_set_ps(4,3,2,1);
			__m128 row1 = _mm_set_ps(8,7,6,5);
			__m128 row2 = _mm_set_ps(12,11,10,9);
			__m128 row3 = _mm_set_ps(16,15,14,13);
			
			MatrixMM matrix(row0, row1, row2, row3);

			Assert::AreEqual(1.0f, matrix.M11());
			Assert::AreEqual(2.0f, matrix.M12());
			Assert::AreEqual(3.0f, matrix.M13());
			Assert::AreEqual(4.0f, matrix.M14());

			Assert::AreEqual(5.0f, matrix.M21());
			Assert::AreEqual(6.0f, matrix.M22());
			Assert::AreEqual(7.0f, matrix.M23());
			Assert::AreEqual(8.0f, matrix.M24());
			
			Assert::AreEqual(9.0f, matrix.M31());
			Assert::AreEqual(10.0f, matrix.M32());
			Assert::AreEqual(11.0f, matrix.M33());
			Assert::AreEqual(12.0f, matrix.M34());
			
			Assert::AreEqual(13.0f, matrix.M41());
			Assert::AreEqual(14.0f, matrix.M42());
			Assert::AreEqual(15.0f, matrix.M43());
			Assert::AreEqual(16.0f, matrix.M44());
		}

		TEST_METHOD(MatrixMul)
		{
			MatrixMM lhs(	3,0,0,0,
							0,4,0,0,
							0,0,5,0,
							0,0,0,1);

			MatrixMM rhs(	1,0,0,0,
							0,1,0,0,
							0,0,1,0,
							1,2,3,1);

			MatrixMM result = MatrixMM::Mul(&lhs, &rhs);

			Assert::AreEqual(3.0f, result.M11());
			Assert::AreEqual(0.0f, result.M12());
			Assert::AreEqual(0.0f, result.M13());
			Assert::AreEqual(0.0f, result.M14());

			Assert::AreEqual(0.0f, result.M21());
			Assert::AreEqual(4.0f, result.M22());
			Assert::AreEqual(0.0f, result.M23());
			Assert::AreEqual(0.0f, result.M24());
			
			Assert::AreEqual(0.0f, result.M31());
			Assert::AreEqual(0.0f, result.M32());
			Assert::AreEqual(5.0f, result.M33());
			Assert::AreEqual(0.0f, result.M34());
			
			Assert::AreEqual(1.0f, result.M41());
			Assert::AreEqual(2.0f, result.M42());
			Assert::AreEqual(3.0f, result.M43());
			Assert::AreEqual(1.0f, result.M44());
		}

		TEST_METHOD(TransformPoint)
		{
			MatrixMM mat(	2,0,0,0,
							0,1,0,0,
							0,0,1,0,
							10,-20,30,1);

			Float3 vec(1,2,3);

			Float4MM result = MatrixMM::TransformPoint(&mat, &vec);

			Assert::AreEqual(12.0f, result.X());
			Assert::AreEqual(-18.0f, result.Y());
			Assert::AreEqual(33.0f, result.Z());
			Assert::AreEqual(1.0f, result.W());
		}

		TEST_METHOD(TransformDirection)
		{
			MatrixMM mat(	2,0,0,0,
							0,1,0,0,
							0,0,1,0,
							10,-20,30,1);

			Float3 vec(1,2,3);

			Float4MM result = MatrixMM::TransformDirection(&mat, &vec);

			Assert::AreEqual(2.0f, result.X());
			Assert::AreEqual(2.0f, result.Y());
			Assert::AreEqual(3.0f, result.Z());
			Assert::AreEqual(0.0f, result.W());
		}

		TEST_METHOD(Transform)
		{
			MatrixMM mat(	2,0,0,0,
							0,1,0,0,
							0,0,1,0,
							10,-20,30,1);

			Float4MM vec(1,2,3,1);

			Float4MM result = MatrixMM::Transform(&mat, &vec);

			Assert::AreEqual(12.0f, result.X());
			Assert::AreEqual(-18.0f, result.Y());
			Assert::AreEqual(33.0f, result.Z());
			Assert::AreEqual(1.0f, result.W());
		}

		//
		TEST_METHOD(TransformPointArray)
		{
			MatrixMM mat(	2,0,0,0,
							0,1,0,0,
							0,0,1,0,
							5,6,7,1);

			Float3 vecArray[10];
			for(int i=0; i<10; ++i)
			{
				float component=(float)i;
				vecArray[i] = Float3(component,component,component);
			}

			Float4MM vecResultArray[10];

			MatrixMM::TransformPointArray(&mat, vecArray, vecResultArray, 10);

			for(int i=0; i<10; ++i)
			{
				Assert::AreEqual((float)(i*2+5), vecResultArray[i].X());
				Assert::AreEqual((float)(i+6), vecResultArray[i].Y());
				Assert::AreEqual((float)(i+7), vecResultArray[i].Z());
				Assert::AreEqual(1.0f, vecResultArray[i].W());
			}
		}
		
		TEST_METHOD(TransformDirectionArray)
		{
			MatrixMM mat(	2,0,0,0,
							0,1,0,0,
							0,0,1,0,
							5,6,7,1);

			Float3 vecArray[10];
			for(int i=0; i<10; ++i)
			{
				float component=(float)i;
				vecArray[i] = Float3(component,component,component);
			}

			Float4MM vecResultArray[10];

			MatrixMM::TransformDirectionArray(&mat, vecArray, vecResultArray, 10);

			for(int i=0; i<10; ++i)
			{
				Assert::AreEqual((float)(i*2), vecResultArray[i].X());
				Assert::AreEqual((float)(i), vecResultArray[i].Y());
				Assert::AreEqual((float)(i), vecResultArray[i].Z());
				Assert::AreEqual(0.0f, vecResultArray[i].W());
			}
		}

		TEST_METHOD(TransformArray)
		{
			MatrixMM mat(	2,0,0,0,
							0,1,0,0,
							0,0,1,0,
							5,6,7,1);

			Float4MM vecArray[10];
			for(int i=0; i<10; ++i)
			{
				float component=(float)i;
				vecArray[i] = Float4MM(component,component,component, 1);
			}

			Float4MM vecResultArray[10];

			MatrixMM::TransformArray(&mat, vecArray, vecResultArray, 10);

			for(int i=0; i<10; ++i)
			{
				Assert::AreEqual((float)(i*2+5), vecResultArray[i].X());
				Assert::AreEqual((float)(i+6), vecResultArray[i].Y());
				Assert::AreEqual((float)(i+7), vecResultArray[i].Z());
				Assert::AreEqual(1.0f, vecResultArray[i].W());
			}
		}
	};
}
