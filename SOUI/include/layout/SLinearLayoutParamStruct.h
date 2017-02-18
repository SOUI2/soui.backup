#pragma once

namespace SOUI
{
	enum Gravity{
		G_Undefined=-1,
		G_Left=0,G_Top=0,
		G_Center=1,
		G_Right=2,G_Bottom=2,
	};

	struct SLinearLayoutParamStruct
	{
		int width,height;
		float weight;
		Gravity gravity;
		CRect rcExtend;//相当于android的margin属性
	};
}