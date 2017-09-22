/*
	测试log模块
*/
#include <gtest/gtest.h>
#include <tchar.h>
#include <unknown/obj-ref-i.h>
#include <com-cfg.h>

#include <core-def.h>
#ifdef DLL_CORE
#   define SOUI_EXP __declspec(dllimport)
#else
#   define SOUI_EXP
#endif

#include <interface/slog-i.h>
SOUI::ILog4zManager *g_LogMgr=NULL;
#define GETLOGMGR() g_LogMgr
#include <helper/slog.h>

using namespace SOUI;


bool LogStream()
{
	LOGW("logStream","abc="<<45<<" float:"<<1.5f);
	return true;
}

bool LogFormat()
{
	LOGFMTI("logFormat", "abc=%d float=%.2f", 54, 5.1f);
	return true;
}

TEST(Log, stream) {

	SComMgr comMgr;
	comMgr.CreateLog4z((IObjRef**)&g_LogMgr);
	g_LogMgr->start();

	EXPECT_TRUE(LogStream());

	EXPECT_TRUE(LogFormat());

	g_LogMgr->stop();
	g_LogMgr->Release();
}