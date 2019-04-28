// TestClass.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "TestClass.h"

// This is an example of an exported variable
VISCAT_LIB_API int nTestVar=0;

// This is an example of an exported function.
VISCAT_LIB_API int fnTestAddFunc(int a, int b)
{
	return a+b;
}

// This is the constructor of a class that has been exported.
// see TestClass.h for the class definition
CTestClass::CTestClass()
{
	return;
}

int CTestClass::Add(int a, int b)
{
	return a+b;
}
