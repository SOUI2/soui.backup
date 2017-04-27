#pragma once

namespace SOUI
{
	struct IDpiChangeListener
	{
		virtual void onScaleChanged(int scale) = 0;
	};

	struct IDpiAware
	{
		virtual void registerDpiChangeListener(IDpiChangeListener * listener) = 0;
		virtual void unregisterDipChangeListener(IDpiChangeListener * listener) = 0;
	};
}