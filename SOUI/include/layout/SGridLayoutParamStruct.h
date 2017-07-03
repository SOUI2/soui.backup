#pragma once

#include "SLayoutSize.h"

namespace SOUI
{

	enum GridGravity{
		gUndef=-1,
		gLeft=0,
		gTop=0,
		gCenter=1,
		gMiddle=1,
		gRight=2,
		gBottom=2,
	};


	struct SGridLayoutParamStruct
	{
		int iCol;
		int iRow;
		int nColSpan;
		int nRowSpan;
		GridGravity layoutGravityX;
		GridGravity layoutGravityY;
		SLayoutSize  width;
		SLayoutSize  height;
		float fColWeight;
		float fRowWeight;
	};
}