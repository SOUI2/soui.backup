#pragma once

namespace SOUI
{

	class SOUI_EXP SLayoutSize
	{
	public:
		enum Unit{
			px=0,dp
		};

		SLayoutSize();

		float fSize;
		Unit  unit;

		ORIENTATION orientation;

		void setWrapContent();
		bool isWrapContent() const;

		void setMatchParent();
		bool isMatchParent() const;

		bool isSpecifiedSize() const;

		void setInvalid();
		bool isValid() const;

		int  toPixelSize() const;

		SStringW toString() const;

		void setOrientation(ORIENTATION orientation);
		bool isVert() const ;
		


		void parseString(const SStringW & strSize);

		SLayoutSize & operator = (const SLayoutSize & src);

	};


	class SOUI_EXP SLayoutWidth : public SLayoutSize
	{
	public:
		SLayoutWidth(){orientation = Horz;}

		SLayoutSize & operator = (const SLayoutSize & src);
		static SLayoutWidth fromString(const SStringW & strSize);
	};

	class SOUI_EXP SLayoutHeight : public SLayoutSize
	{
	public:
		SLayoutHeight(){orientation = Vert;}

		SLayoutSize & operator = (const SLayoutSize & src);

		static SLayoutHeight fromString(const SStringW & strSize);
	};
}