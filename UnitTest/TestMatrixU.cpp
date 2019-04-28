#include "stdafx.h"
#include "CppUnitTest.h"
#include "MatrixU.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{		
	TEST_CLASS(TestMatrixU)
	{
	public:
		
		TEST_METHOD(Init)
		{
			MatrixU matrix(	1,2,3,4,
							5,6,7,8,
							9,10,11,12,
							13,14,15,16);

			Assert::AreEqual(1.0f, matrix.m11);
			Assert::AreEqual(2.0f, matrix.m12);
			Assert::AreEqual(3.0f, matrix.m13);
			Assert::AreEqual(4.0f, matrix.m14);

			Assert::AreEqual(5.0f, matrix.m21);
			Assert::AreEqual(6.0f, matrix.m22);
			Assert::AreEqual(7.0f, matrix.m23);
			Assert::AreEqual(8.0f, matrix.m24);
			
			Assert::AreEqual(9.0f, matrix.m31);
			Assert::AreEqual(10.0f, matrix.m32);
			Assert::AreEqual(11.0f, matrix.m33);
			Assert::AreEqual(12.0f, matrix.m34);
			
			Assert::AreEqual(13.0f, matrix.m41);
			Assert::AreEqual(14.0f, matrix.m42);
			Assert::AreEqual(15.0f, matrix.m43);
			Assert::AreEqual(16.0f, matrix.m44);
		}

		TEST_METHOD(MatrixMul)
		{
			MatrixU lhs(3,0,0,0,
						0,4,0,0,
						0,0,5,0,
						0,0,0,1);

			MatrixU rhs(1,0,0,0,
						0,1,0,0,
						0,0,1,0,
						1,2,3,1);

			MatrixU result = MatrixU::Mul(&lhs, &rhs);

			Assert::AreEqual(3.0f, result.m11);
			Assert::AreEqual(0.0f, result.m12);
			Assert::AreEqual(0.0f, result.m13);
			Assert::AreEqual(0.0f, result.m14);
										  
			Assert::AreEqual(0.0f, result.m21);
			Assert::AreEqual(4.0f, result.m22);
			Assert::AreEqual(0.0f, result.m23);
			Assert::AreEqual(0.0f, result.m24);
										  
			Assert::AreEqual(0.0f, result.m31);
			Assert::AreEqual(0.0f, result.m32);
			Assert::AreEqual(5.0f, result.m33);
			Assert::AreEqual(0.0f, result.m34);
										  
			Assert::AreEqual(1.0f, result.m41);
			Assert::AreEqual(2.0f, result.m42);
			Assert::AreEqual(3.0f, result.m43);
			Assert::AreEqual(1.0f, result.m44);
		}
	};
}
