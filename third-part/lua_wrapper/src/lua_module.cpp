#include <souistd.h>
#include <sol.hpp>

#include "iface_lua_module.h"
#include "lua_module.h"

int Utf8ToW(lua_State* L) {
    size_t n = 0;
    char* str = (char*) luaL_checklstring(L, -1, &n);
    if (!str)   return 0;
    SOUI::SStringW strW = SOUI::S_CA2W(str, CP_UTF8);
    SOUI::SStringA strA = SOUI::S_CW2A(strW);
    lua_pushfstring(L, "%s", strA);
    //lua_pushlstring(L, (const char*) (LPCWSTR) strW, 2 * strW.GetLength() + 2);
    return 1;
}

int Utf8ToT(lua_State *L) {
    size_t n = 0;
    char* str = (char*) luaL_checklstring(L, -1, &n);
    if (!str)   return 0;
    SOUI::SStringT strT = SOUI::S_CA2T(str, CP_UTF8);
    SOUI::SStringA strA = SOUI::S_CT2A(strT);
    lua_pushfstring(L, "%s", strA);
    //lua_pushlstring(L, (const char*) (LPCTSTR) strT, (strT.GetLength() + 1) * sizeof(TCHAR));
    return 1;
}

SOUI::SLuaModule::SLuaModule() {
}

SOUI::SLuaModule::~SLuaModule() {
}

void * SOUI::SLuaModule::LuaGetEngine() {
    return (void*)lua_.lua_state();
}

bool SOUI::SLuaModule::LuaInitialize() {
    lua_.open_libraries();

    ExportBasic();
    ExportString();
    ExportStrCpCvt();
    ExportPugixml();
    ExportApp();
    ExportMessageBox();
    ExportScriptModule();
    ExportResProvider();
    ExportSObject();
    ExportWindow();
    ExportHostWnd();
    ExportEventArgs();
    ExportCtrls();

    lua_register(lua_, "A2W", Utf8ToW);
    lua_.set_function("cast_a2w", [](char *str) {
        return (wchar_t *) str;
    });
    lua_.safe_script("function L(str)\n"
                     "return cast_a2w(A2W(str));\n"
                     "end");//注册一个全局的"L"函数，用来将utf8编码的字符串转换为WCHAR

    lua_register(lua_, "A2T", Utf8ToT);
    lua_.set_function("cast_a2t", [](char *str) {
        return (TCHAR *) str;
    });
    lua_.safe_script("function T (str)\n"
                     "return cast_a2t(A2T(str));\n"
                     "end");
    return true;
}

void SOUI::SLuaModule::LuaShutdown() {
}

const char * SOUI::SLuaModule::LuaIdentifier() {
    return "SOUI.Script.Lua5.3.4 & Sol2";
}

void SOUI::SLuaModule::LuaExecuteFile(const char * lpszFile) {
    lua_.safe_script_file(lpszFile);
}

void SOUI::SLuaModule::LuaExecuteBuffer(const char * lpBuffer, size_t sz) {
    lua_.safe_script(std::string(lpBuffer, sz));
}

void SOUI::SLuaModule::LuaExecuteString(const char * str) {
    lua_.safe_script(str);
}

bool SOUI::SLuaModule::LuaCall(const char * fun, SOUI::EventArgs * pArg) {
    bool result = false;
    sol::function f = lua_[fun];
    if (f)
        result = f(pArg);
    return result;
}

void SOUI::SLuaModule::ExportBasic() {
    lua_.set_function("RGB", [](int r, int g, int b) {
        return RGBA(r, g, b, 255);
    });
    lua_.set_function("RGBA", [](int r, int g, int b, int a) {
        return RGBA(r, g, b, a);
    });

    //POINT
    lua_.new_usertype<POINT>("POINT", "x", &POINT::x, "y", &POINT::y);

    //RECT
    lua_.new_usertype<RECT>("RECT", "left", &RECT::left, "top", &RECT::top, "right", &RECT::right, "bottom", &RECT::bottom);

    //SIZE
    lua_.new_usertype<SIZE>("SIZE", "cx", &SIZE::cx, "cy", &SIZE::cy);

    //CPoint
    lua_.new_usertype<CPoint>("CPoint",
                              sol::constructors<CPoint(), CPoint(LONG, LONG)>(),
                              sol::call_constructor,sol::constructors<sol::types<>, sol::types<LONG, LONG>>(),
                              sol::base_classes, sol::bases<POINT>());

    //CRect
    lua_.new_usertype<CRect>("CRect",
                             sol::constructors<CRect(), CRect(LONG, LONG, LONG, LONG)>(),
                             sol::call_constructor, sol::constructors<sol::types<>, sol::types<LONG, LONG, LONG, LONG>>(),
                             "Width", &CRect::Width,
                             "Height", &CRect::Height,
                             "Size", &CRect::Size,
                             "IsRectEmpty", &CRect::IsRectEmpty,
                             "IsRectNull", &CRect::IsRectNull,
                             "PtInRect", &CRect::PtInRect,
                             "SetRectEmpty", &CRect::SetRectEmpty,
                             "OffsetRect", (void (CRect::*)(int, int))&CRect::OffsetRect,
                             sol::base_classes, sol::bases<RECT>());

    //CSize
    lua_.new_usertype<CSize>("CSize",
                             sol::constructors<CSize(), CSize(LONG, LONG)>(),
                             sol::call_constructor, sol::constructors<sol::types<>, sol::types<LONG, LONG>>(),
                             sol::base_classes, sol::bases<SIZE>());
}

void SOUI::SLuaModule::ExportString() {
    lua_.new_usertype<SStringA>("SStringA"
                                , sol::constructors<SStringA(), SStringA(const SStringA &), SStringA(const char*)>()
                                , sol::call_constructor, sol::constructors<sol::types<>, sol::types<const SStringA&>, sol::types<const char*>>()
                                , "GetLength", &SStringA::GetLength
                                , "IsEmpty", &SStringA::IsEmpty
                                , "Empty", &SStringA::Empty
                                , "GetAt", &SStringA::GetAt
                                , "SetAt", &SStringA::SetAt
                                , "Mid", (SStringA(SStringA::*)(int) const)&SStringA::Mid
                                , "Mid2", (SStringA(SStringA::*)(int, int) const)&SStringA::Mid
                                , "Left", &SStringA::Left
                                , "Right", &SStringA::Left
                                , "MakeUpper", &SStringA::MakeUpper
                                , "MakeLower", &SStringA::MakeLower
                                , "TrimRight", &SStringA::TrimRight
                                , "TrimLeft", &SStringA::TrimLeft
                                , "Trim", &SStringA::Trim
                                , "InsertChar", (int (SStringA::*)(int, char))&SStringA::Insert
                                , "InsertStr", (int (SStringA::*)(int, const char*))&SStringA::Insert
                                , "Delete", &SStringA::Delete
                                , "ReplaceChar", (int (SStringA::*)(char, char))&SStringA::Replace
                                , "ReplaceStr", (int (SStringA::*)(const char*, const char*))&SStringA::Replace
                                , "Remove", &SStringA::Remove
                                , "FindChar", (int (SStringA::*)(char, int) const)&SStringA::Find
                                , "FindStr", (int (SStringA::*)(const char *, int) const)&SStringA::Find
                                , "ReverseFind", &SStringA::ReverseFind
                                , "GetBuffer", &SStringA::GetBuffer
                                , "ReleaseBuffer", &SStringA::ReleaseBuffer
                                , "GetBufferSetLength", &SStringA::GetBufferSetLength
                                , "SetLength", &SStringA::SetLength
                                , "LockBuffer", &SStringA::LockBuffer
                                , "UnlockBuffer", &SStringA::UnlockBuffer
                                , "AppendChar", (const SStringA & (SStringA::*)(char))&SStringA::Append
                                , "AppendPsz", (const SStringA & (SStringA::*)(const char *))&SStringA::Append
                                , "AppendStr", (const SStringA & (SStringA::*)(const SStringA &))&SStringA::Append);

    lua_.new_usertype<SStringW>("SStringW"
                                , sol::constructors<SStringW(), SStringW(const SStringW&), SStringW(const wchar_t *)>()
                                , sol::call_constructor, sol::constructors<sol::types<>, sol::types<const SStringW &>, sol::types<const wchar_t *>>()
                                , "GetLength", &SStringW::GetLength
                                , "IsEmpty", &SStringW::IsEmpty
                                , "Empty", &SStringW::Empty
                                , "GetAt", &SStringW::GetAt
                                , "SetAt", &SStringW::SetAt
                                , "Mid", (SStringW(SStringW::*)(int) const)&SStringW::Mid
                                , "Mid2", (SStringW(SStringW::*)(int, int) const)&SStringW::Mid
                                , "Left", &SStringW::Left
                                , "Right", &SStringW::Left
                                , "MakeUpper", &SStringW::MakeUpper
                                , "MakeLower", &SStringW::MakeLower
                                , "TrimRight", &SStringW::TrimRight
                                , "TrimLeft", &SStringW::TrimLeft
                                , "Trim", &SStringW::Trim
                                , "InsertChar", (int (SStringW::*)(int, wchar_t))&SStringW::Insert
                                , "InsertStr", (int (SStringW::*)(int, const wchar_t*))&SStringW::Insert
                                , "Delete", &SStringW::Delete
                                , "ReplaceChar", (int (SStringW::*)(wchar_t, wchar_t))&SStringW::Replace
                                , "ReplaceStr", (int (SStringW::*)(const wchar_t*, const wchar_t*))&SStringW::Replace
                                , "Remove", &SStringW::Remove
                                , "FindChar", (int (SStringW::*)(wchar_t, int) const)&SStringW::Find
                                , "FindStr", (int (SStringW::*)(const wchar_t *, int) const)&SStringW::Find
                                , "ReverseFind", &SStringW::ReverseFind
                                , "GetBuffer", &SStringW::GetBuffer
                                , "ReleaseBuffer", &SStringW::ReleaseBuffer
                                , "GetBufferSetLength", &SStringW::GetBufferSetLength
                                , "SetLength", &SStringW::SetLength
                                , "LockBuffer", &SStringW::LockBuffer
                                , "UnlockBuffer", &SStringW::UnlockBuffer
                                , "AppendChar", (const SStringW & (SStringW::*)(wchar_t))&SStringW::Append
                                , "AppendPsz", (const SStringW & (SStringW::*)(const wchar_t *))&SStringW::Append
                                , "AppendStr", (const SStringW & (SStringW::*)(const SStringW &))&SStringW::Append);
}

void SOUI::SLuaModule::ExportStrCpCvt() {
    lua_.set_function("S_W2A", &SStrCpCvt::CvtW2A);
    lua_.set_function("S_A2W", &SStrCpCvt::CvtA2W);
    lua_.set_function("S_A2A", &SStrCpCvt::CvtA2A);
    lua_.set_function("S_W2W", &SStrCpCvt::CvtW2W);
}

void SOUI::SLuaModule::ExportPugixml() {

    //xml_parse_result
    lua_.new_usertype<pugi::xml_parse_result>("xml_parse_result", "isOK", &pugi::xml_parse_result::isOK);

    //xml_text
    lua_.new_usertype<pugi::xml_text>("xml_text"
                                      , sol::constructors<pugi::xml_text()>()
                                      , sol::call_constructor, sol::constructors<sol::types<>>()
                                      , "get", &pugi::xml_text::get
                                      , "as_int", &pugi::xml_text::as_int
                                      , "as_uint", &pugi::xml_text::as_uint
                                      , "as_double", &pugi::xml_text::as_double
                                      , "as_float", &pugi::xml_text::as_float
                                      , "as_bool", &pugi::xml_text::as_bool);

    //xml_attribute
    lua_.new_usertype<pugi::xml_attribute>("xml_attribute"
                                           , "name", &pugi::xml_attribute::name
                                           , "value", &pugi::xml_attribute::value
                                           , "empty", &pugi::xml_attribute::empty
                                           , "as_int", &pugi::xml_attribute::as_int
                                           , "as_uint", &pugi::xml_attribute::as_uint
                                           , "as_double", &pugi::xml_attribute::as_double
                                           , "as_float", &pugi::xml_attribute::as_float
                                           , "as_bool", &pugi::xml_attribute::as_bool
                                           , "next_attribute", &pugi::xml_attribute::next_attribute
                                           , "previous_attribute", &pugi::xml_attribute::previous_attribute);

    //xml_node
    lua_.new_usertype<pugi::xml_node>("xml_node"
                                      , sol::constructors<pugi::xml_node()>()
                                      , sol::call_constructor, sol::constructors<sol::types<>>()
                                      , "name", &pugi::xml_node::name
                                      , "set_name", &pugi::xml_node::set_name
                                      , "value", &pugi::xml_node::value
                                      , "set_value", &pugi::xml_node::set_value
                                      , "first_attribute", &pugi::xml_node::first_attribute
                                      , "last_attribute", &pugi::xml_node::last_attribute
                                      , "first_child", &pugi::xml_node::first_child
                                      , "last_child", &pugi::xml_node::last_child
                                      , "next_sibling", (pugi::xml_node(pugi::xml_node::*)()const)&pugi::xml_node::next_sibling
                                      , "previous_sibling", (pugi::xml_node(pugi::xml_node::*)()const)&pugi::xml_node::previous_sibling
                                      , "next_siblingByName", (pugi::xml_node(pugi::xml_node::*)(const wchar_t *, bool)const)&pugi::xml_node::next_sibling
                                      , "previous_siblingByName", (pugi::xml_node(pugi::xml_node::*)(const wchar_t *, bool)const)&pugi::xml_node::previous_sibling
                                      , "parent", &pugi::xml_node::parent
                                      , "root", &pugi::xml_node::root
                                      , "text", &pugi::xml_node::text
                                      , "child", &pugi::xml_node::child
                                      , "attribute", &pugi::xml_node::attribute);
    //xml_document
    lua_.new_usertype<pugi::xml_document>("xml_document"
                                          , sol::constructors<pugi::xml_document()>()
                                          , sol::call_constructor, sol::constructors<sol::types<>>()
                                          , "load", &pugi::xml_document::load
                                          //, "load_buffer", &pugi::xml_document::load_buffer
                                          , "load_fileA", (pugi::xml_parse_result(pugi::xml_document::*)(const char *, unsigned int, pugi::xml_encoding))&pugi::xml_document::load_file
                                          , "load_fileW", (pugi::xml_parse_result(pugi::xml_document::*)(const wchar_t *, unsigned int, pugi::xml_encoding))&pugi::xml_document::load_file
                                          , "reset", (void (pugi::xml_document::*)())&pugi::xml_document::reset
                                          , "reset2", (void (pugi::xml_document::*)(const pugi::xml_document&))&pugi::xml_document::reset
                                          , "save_fileA", (bool (pugi::xml_document::*)(const char*, const wchar_t*, unsigned int, pugi::xml_encoding)const)&pugi::xml_document::save_file
                                          , "save_fileW", (bool (pugi::xml_document::*)(const wchar_t*, const wchar_t*, unsigned int, pugi::xml_encoding)const)&pugi::xml_document::save_file
                                          , sol::base_classes, sol::bases<pugi::xml_node>());
}

void SOUI::SLuaModule::ExportApp() {
    lua_.new_usertype<SApplication>("SApplication"
                                          , "AddResProvider", &SApplication::AddResProvider
                                          , "RemoveResProvider", &SApplication::RemoveResProvider
                                          , "GetInstance", &SApplication::GetInstance
                                          , "CreateScriptModule", &SApplication::CreateScriptModule
                                          , "SetScriptModule", &SApplication::SetScriptFactory
                                          , "GetTranslator", &SApplication::GetTranslator
                                          , "SetTranslator", &SApplication::SetTranslator);

    lua_.set_function("theApp", &SApplication::getSingletonPtr);
}

void SOUI::SLuaModule::ExportMessageBox() {
    lua_.set_function("SMessageBox", SMessageBox);
}

void SOUI::SLuaModule::ExportScriptModule() {
    lua_.new_usertype<IScriptModule>("IScriptModule"
                                     , "GetScriptEngine", &IScriptModule::GetScriptEngine
                                     , "executeScriptFile", &IScriptModule::executeScriptFile
                                     , "executeString", &IScriptModule::executeString
                                     , "executeScriptedEventHandler", &IScriptModule::executeScriptedEventHandler
                                     , "getIdentifierString", &IScriptModule::getIdentifierString
                                     , "subscribeEvent", &IScriptModule::subscribeEvent);
}

void SOUI::SLuaModule::ExportResProvider() {
    lua_.new_usertype<IResProvider>("IResProvider"
                                    , "Init", &IResProvider::Init
                                    , "HasResource", &IResProvider::HasResource
                                    , "LoadIcon", &IResProvider::LoadIcon
                                    , "LoadBitmap", &IResProvider::LoadBitmap
                                    , "LoadImage", &IResProvider::LoadImage
                                    , "GetRawBufferSize", &IResProvider::GetRawBufferSize
                                    , "GetRawBuffer", &IResProvider::GetRawBuffer);

    lua_.set_function("CreateResProvider", &CreateResProvider);
}

void SOUI::SLuaModule::ExportSObject() {

    lua_.new_usertype<IObject>("IObject"
                               , "IsClass", &IObject::IsClass
                               , "GetObjectClass", &IObject::GetObjectClass
                               , "InitFromXml", &IObject::InitFromXml
                               , "SetAttribute", (HRESULT(IObject::*)(const char *, const char *, BOOL))&IObject::SetAttribute
                               , "SetAttributeA", (HRESULT(IObject::*)(const SStringA &, const SStringA &, BOOL))&IObject::SetAttribute
                               , "SetAttributeW", (HRESULT(IObject::*)(const SStringW &, const SStringW &, BOOL))&IObject::SetAttribute
                               , "GetID", &IObject::GetID
                               , "GetName", &IObject::GetName);

    lua_.set_function("SetObjAttr", [](IObject *pObj, LPCSTR pszAttr, LPCSTR pszValue) {
        pObj->SetAttribute(pszAttr, pszValue, FALSE);
    });

    lua_.new_usertype<SObject>("SObject", sol::base_classes, sol::bases<IObject>());
}

void SOUI::SLuaModule::ExportWindow() {

    lua_.new_usertype<SWindow>("SWindow"
                               , sol::constructors<SWindow()>()
                               , sol::call_constructor, sol::constructors<sol::types<>>()
                               , "GetContainer", (ISwndContainer *(SWindow::*)(void))&SWindow::GetContainer
                               , "GetRoot", &SWindow::GetRoot
                               , "GetTopLevelParent", &SWindow::GetTopLevelParent
                               , "GetParent", &SWindow::GetParent
                               , "DestroyChild", &SWindow::DestroyChild
                               , "GetChildrenCount", &SWindow::GetChildrenCount
                               , "FindChildByID", &SWindow::FindChildByID
                               , "FindChildByNameA", (SWindow* (SWindow::*)(LPCSTR, int))&SWindow::FindChildByName
                               , "FindChildByNameW", (SWindow* (SWindow::*)(LPCWSTR, int))&SWindow::FindChildByName
                               , "CreateChildrenFromString", (SWindow* (SWindow::*)(LPCWSTR))&SWindow::CreateChildren
                               , "GetTextAlign", &SWindow::GetTextAlign
                               , "GetWindowRect", (void (SWindow::*)(LPRECT))&SWindow::GetWindowRect
                               , "GetWindowRect2", (CRect(SWindow::*)())&SWindow::GetWindowRect
                               , "GetClientRect", (void (SWindow::*)(LPRECT)const)&SWindow::GetClientRect
                               , "GetClientRect2", (CRect(SWindow::*)()const)&SWindow::GetClientRect
                               , "GetWindowText", &SWindow::GetWindowText
                               , "SetWindowText", &SWindow::SetWindowText
                               , "SendSwndMessage", &SWindow::SSendMessage
                               , "GetID", &SWindow::GetID
                               , "SetID", &SWindow::SetID
                               , "GetUserData", &SWindow::GetUserData
                               , "SetUserData", &SWindow::SetUserData
                               , "GetName", &SWindow::GetName
                               , "GetSwnd", &SWindow::GetSwnd
                               , "InsertChild", &SWindow::InsertChild
                               , "RemoveChild", &SWindow::RemoveChild
                               , "IsChecked", &SWindow::IsChecked
                               , "IsDisabled", &SWindow::IsDisabled
                               , "IsVisible", &SWindow::IsVisible
                               , "SetVisible", &SWindow::SetVisible
                               , "EnableWindow", &SWindow::EnableWindow
                               , "SetCheck", &SWindow::SetCheck
                               , "SetOwner", &SWindow::SetOwner
                               , "GetOwner", &SWindow::GetOwner
                               , "Invalidate", &SWindow::Invalidate
                               , "InvalidateRect", (void (SWindow::*)(LPCRECT))&SWindow::InvalidateRect
                               , "AnimateWindow", &SWindow::AnimateWindow
                               , "GetScriptModule", &SWindow::GetScriptModule
                               , "Move2", (void (SWindow::*)(int, int, int, int))&SWindow::Move
                               , "Move", (void (SWindow::*)(LPCRECT))&SWindow::Move
                               , "FireCommand", &SWindow::FireCommand
                               , "GetDesiredSize", (CSize(SWindow::*)(int, int))&SWindow::GetDesiredSize
                               , "GetDesiredSize2",(CSize(SWindow::*)(LPCRECT))&SWindow::GetDesiredSize
                               , "GetWindow", &SWindow::GetWindow
                               , "SetWindowRgn", &SWindow::SetWindowRgn
                               , "GetWindowRgn", &SWindow::GetWindowRgn
                               , sol::base_classes, sol::bases<SObject>());

    lua_.set_function("toSWindow", [](IObject *pObj) {
        return sobj_cast<SWindow>(pObj);
    });
}

void SOUI::SLuaModule::ExportHostWnd() {
    lua_.new_usertype<SHostWnd>("SHostWnd"
                                , "AnimateHostWindow", &SHostWnd::AnimateHostWindow
                                , "setTimeout", &SHostWnd::setTimeout
                                , "setInterval", &SHostWnd::setInterval
                                , "clearTimer", &SHostWnd::clearTimer
                                , "GetRoot", &SHostWnd::GetRoot
                                , sol::base_classes, sol::bases<SObject>());

    lua_.set_function("toHostWnd", [](IObject *pObj) {
        return sobj_cast<SHostWnd>(pObj);
    });
}

void SOUI::SLuaModule::ExportEventArgs() {
    lua_.new_usertype<EventArgs>("EventArgs"
                                 , "sender", &EventArgs::sender
                                 , "idFrom", &EventArgs::idFrom
                                 , "nameFrom", &EventArgs::nameFrom
                                 , sol::base_classes, sol::bases<IObject>());

    lua_.new_usertype<EventTimer>("EventTimer"
                                  , "uID", &EventTimer::uID
                                  , sol::base_classes, sol::bases<EventArgs>());

    lua_.set_function("toEventTimer", [](EventArgs *pEvt) {
        return sobj_cast<EventTimer>(pEvt);
    });//类型转换

    lua_.new_usertype<EventSwndSize>("EventSwndSize"
                                     , "szWnd", &EventSwndSize::szWnd
                                     , sol::base_classes, sol::bases<EventArgs>());

    lua_.set_function("toEventSize", [](EventArgs *pEvt) {
        return sobj_cast<EventSwndSize>(pEvt);
    });//类型转换
}

void SOUI::SLuaModule::ExportCtrls() {
    lua_.new_usertype<SComboBase>("SComboBoxBase"
                                  , "GetCurSel", &SComboBase::GetCurSel
                                  , "GetCount", &SComboBase::GetCount
                                  , "SetCurSel", &SComboBase::SetCurSel
                                  , "GetLBText", &SComboBase::GetLBText
                                  , "FindString", &SComboBase::FindString
                                  , "DropDown", &SComboBase::DropDown
                                  , "CloseUp", &SComboBase::CloseUp
                                  , sol::base_classes, sol::bases<SWindow>());

    lua_.set_function("toComboboxBase", [](IObject *pObj) {
        return sobj_cast<SComboBase>(pObj);
    });

    lua_.new_usertype<SComboBox>("SComboBox"
                                 , "InsertItem", &SComboBox::InsertItem
                                 , "DeleteString", &SComboBox::DeleteString
                                 , "ResetContent", &SComboBox::ResetContent
                                 , "GetLBText", &SComboBox::GetLBText
                                 , "GetListBox", &SComboBox::GetListBox
                                 , "GetItemData", &SComboBox::GetItemData
                                 , "SetItemData", &SComboBox::SetItemData
                                 , sol::base_classes, sol::bases<SComboBase>());

    lua_.set_function("toCombobox", [](IObject *pObj) {
        return sobj_cast<SComboBox>(pObj);
    });
}

static SOUI::SLuaModule lua_module;

extern "C"
__declspec(dllexport) ILuaModule *CreateInterface() {
    return &lua_module;
}