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

		void setWrapContent();
		bool isWrapContent() const;

		void setMatchParent();
		bool isMatchParent() const;

		bool isSpecifiedSize() const;
		bool isValid() const;

		int  toPixelSize() const;

		SStringW toString() const;


		void parseString(const SStringW & strSize);
	protected:
		virtual bool isVert() const = 0;

	};


	class SOUI_EXP SLayoutWidth : public SLayoutSize
	{
	public:
		static SLayoutWidth fromString(const SStringW & strSize);
		SLayoutSize & operator = (const SLayoutSize& src);

	protected:
		virtual bool isVert() const;
	};

	class SOUI_EXP SLayoutHeight : public SLayoutSize
	{
	public:
		static SLayoutHeight fromString(const SStringW & strSize);

		SLayoutSize & operator = (const SLayoutSize& src);
	protected:
		virtual bool isVert() const;
	};
}