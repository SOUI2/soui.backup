// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include <winsock2.h>

#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/souictrls.h>
#include <event/notifycenter.h>

using namespace SOUI;

// TODO: 在此处引用程序需要的其他头文件

#include <ShlObj.h>
#include <assert.h>
#define ASSERT			assert
#define VERIFY			ASSERT
#define TRACE			ATLTRACE

#include "res/resource.h"