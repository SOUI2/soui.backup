#ifndef __LUA_MODULE_H__
#define __LUA_MODULE_H__
#pragma once

namespace SOUI {

class SLuaModule :public ILuaModule {
public:
    explicit SLuaModule();
    ~SLuaModule();

    //ILuaModule methods
    virtual void *          LuaGetEngine() override;
    virtual bool            LuaInitialize() override;
    virtual void            LuaShutdown() override;
    virtual const char *    LuaIdentifier() override;
    virtual void            LuaExecuteFile(const char *lpszFile) override;
    virtual void            LuaExecuteBuffer(const char *lpBuffer, size_t sz) override;
    virtual void            LuaExecuteString(const char *str) override;
    virtual bool            LuaCall(const char *fun, EventArgs *pArg) override;

protected:
    void ExportBasic();
    void ExportString();
    void ExportStrCpCvt();
    void ExportPugixml();
    void ExportApp();
    void ExportMessageBox();
    void ExportScriptModule();
    void ExportResProvider();
    void ExportSObject();
    void ExportWindow();
    void ExportHostWnd();
    void ExportEventArgs();
    void ExportCtrls();
protected:
    sol::state lua_;
};

} // namespace SOUI

#endif // !__LUA_MODULE_H__