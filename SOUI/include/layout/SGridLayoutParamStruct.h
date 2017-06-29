#pragma once

#include "SLayoutSize.h"

namespace SOUI
{

	enum GridGravityX{
		gLeft=0,
		gCenter,
		gRight,
	};

	enum GridGravityY{
		gTop=0,
		gMiddle,
		gBottom,
	};

	struct SGridLayoutParamStruct
	{
		int iCol;
		int iRow;
		int nColSpan;
		int nRowSpan;
		GridGravityX xGravity;
		GridGravityY yGravity;
		SLayoutSize  width;
		SLayoutSize  height;
	};
}