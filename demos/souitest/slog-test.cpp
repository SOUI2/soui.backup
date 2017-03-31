/*
	²âÊÔlogÄ£¿é
*/
#include <gtest/gtest.h>  
#include <souistd.h>
#include <com-cfg.h>

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

	SApplication *theApp = new SApplication(NULL,0);

	SComMgr comMgr;
	ILog4zManager *pLog = NULL;
	comMgr.CreateLog4z((IObjRef**)&pLog);
	pLog->start();

	theApp->SetLogManager(pLog);

	EXPECT_TRUE(LogStream());

	EXPECT_TRUE(LogFormat());

	pLog->stop();
	pLog->Release();
	delete theApp;
}