#pragma once

#include <codecvt>
#include <locale>
#include <string>

#define SPTLIB_VERSION "1.0-beta"

extern std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> string_converter;

extern void ( *_EngineMsg )( const char *format, ... );
extern void ( *_EngineDevMsg )( const char *format, ... );
extern void ( *_EngineWarning )( const char *format, ... );
extern void ( *_EngineDevWarning )( const char *format, ... );

#ifndef SPT_MESSAGE_PREFIX
#define SPT_MESSAGE_PREFIX "SPTLib: "
#endif // SPT_MESSAGE_PREFIX

#define EngineMsg(...)        _EngineMsg(SPT_MESSAGE_PREFIX __VA_ARGS__)
#define EngineDevMsg(...)     _EngineDevMsg(SPT_MESSAGE_PREFIX __VA_ARGS__)
#define EngineWarning(...)    _EngineWarning(SPT_MESSAGE_PREFIX __VA_ARGS__)
#define EngineDevWarning(...) _EngineDevWarning(SPT_MESSAGE_PREFIX __VA_ARGS__)

std::wstring GetFileName( const std::wstring &fileNameWithPath );
std::wstring GetFolderName( const std::wstring &fileNameWithPath );
