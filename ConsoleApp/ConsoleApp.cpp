// ConsoleApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "OccluderBuffer.h"
#include "MatrixU.h"
#include "StopWatch.h"
#include <random>

int _tmain(int argc, _TCHAR* argv[])
{
	StopWatch stopWatch;
	COccluderBuffer* pBuffer = new COccluderBuffer(VISCAT_Resolution::VISCAT_W256H128);

	//create occluder vertices
	const int vertexCount=33*33;
	Float3 vertices[vertexCount];
	for(int y=0; y<33; ++y)
	{
		for(int x=0; x<33; ++x)
		{
			//0 ~ 5
			float z = (rand()%10000/2000.0f);
			vertices[x+y*33] = Float3((float)x-15.5f, (float)y-15.5f, z);
		}
	}

	//create occluder indices
	const int triangleCount=32*32*2;
	unsigned short indices[triangleCount*3];
	for(int y=0; y<32; ++y)
	{
		for(int x=0; x<32; ++x)
		{
			ushort index0 = (x+y*33);
			ushort index1 = (x+1+y*33);
			ushort index2 = (x+(y+1)*33);
			ushort index3 = (x+1+(y+1)*33);

			indices[(x+y*32)*6+0] = index0;
			indices[(x+y*32)*6+1] = index2;
			indices[(x+y*32)*6+2] = index1;

			indices[(x+y*32)*6+3] = index2;
			indices[(x+y*32)*6+4] = index3;
			indices[(x+y*32)*6+5] = index1;
		}
	}

	//create occludee
	Float3 occludeeOriginalVertices[8]=
	{
		Float3(-1,-1,-1),Float3(1,-1,-1),Float3(1,-1,1),Float3(-1,-1,1),
		Float3(-1,1,-1),Float3(1,1,-1),Float3(1,1,1),Float3(-1,1,1)
	};

	const int occludeeCount=5000;
	Float3 occludeeVertices[occludeeCount][8];
	for(int occludeeIndex=0; occludeeIndex<occludeeCount; ++occludeeIndex)
	{
		//-100~100
		float moveX = (rand()%10000/500.0f) - 10.0f;
		//-100~100
		float moveY = (rand()%10000/500.0f) - 10.0f;
		//0~50
		float moveZ = (rand()%10000/200.0f);

		//0.5~50.5
		float scale = (rand()%10000/200.0f) + 0.5f;

		for(int p=0; p<8; ++p)
		{
			float occludeeX = occludeeOriginalVertices[p].x*scale + moveX;
			float occludeeY = occludeeOriginalVertices[p].y*scale + moveY;
			float occludeeZ = occludeeOriginalVertices[p].z*scale + moveZ;

			occludeeVertices[occludeeIndex][p] = Float3(occludeeX, occludeeY, occludeeZ);
		}
	}

	Float3 eye(0,0,0);
	Float3 at(0,0,1);
	Float3 up(0,1,0);
	MatrixU viewMatrix = MatrixU::ViewMatrix(&eye, &at, &up);
	float halfPi = 3.141592f*0.5f;
	MatrixU projMatrix = MatrixU::ProjectMatrix(halfPi, 1, 1, 100);
	MatrixU vpMatrix = MatrixU::Mul(&viewMatrix, &projMatrix);

	pBuffer->SetViewProjMatrix(&vpMatrix);

	const int occluderRepeatCount=100;	
	int visibleOccludee=0;
	int invisibleOccludee=0;
	while(true)
	{
		stopWatch.Start();
		pBuffer->FrameStart();
		{
			pBuffer->Clear();

			for(int i=0; i<occluderRepeatCount; ++i)
			{
				//-10~10
				float x = (rand()%10000/500.0f) - 10.0f;
				float y = (rand()%10000/500.0f) - 10.0f;
				//1.5~51.5
				float z = (rand()%10000/200.0f) + 1.5f;
				MatrixU worldMat(	1, 0, 0, 0,
									0, 1, 0, 0,
									0, 0, 1, 0,
									x, y, z, 1);

				pBuffer->DrawIndex(vertices, indices, &worldMat, VISCAT_CullMode::NONE, 0, vertexCount, triangleCount);

			}			
		}
		pBuffer->FrameEnd();
		
		stopWatch.End();
		float elapsedTimeMS = stopWatch.GetElapsedMilliSecond();
		printf("[1]%d polygon draw time %fms\n", triangleCount*occluderRepeatCount, elapsedTimeMS);


		visibleOccludee=0;
		invisibleOccludee=0;
		stopWatch.Start();
		for(int i=0; i<occludeeCount; ++i)
		{
			if(pBuffer->BoxIsVisible(occludeeVertices[i])) ++visibleOccludee;
			else ++invisibleOccludee;
		}
		stopWatch.End();

		elapsedTimeMS = stopWatch.GetElapsedMilliSecond();
		printf("[2]occludee %d's check time %fms, vis/invis %d/%d\n\n", 
				occludeeCount, elapsedTimeMS, visibleOccludee, invisibleOccludee);
	}

	delete pBuffer;

	return 0;
}

