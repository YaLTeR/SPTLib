#pragma once

#include <string>

#define SPTLIB_VERSION "1.2"

extern void ( *_EngineMsg )( const char *format, ... );
extern void ( *_EngineDevMsg )( const char *format, ... );
extern void ( *_EngineWarning )( const char *format, ... );
extern void ( *_EngineDevWarning )( const char *format, ... );

#ifndef SPT_MESSAGE_PREFIX
#define SPT_MESSAGE_PREFIX "SPTLib: "
#endif

#define EngineMsg(...)        _EngineMsg(SPT_MESSAGE_PREFIX __VA_ARGS__)
#define EngineDevMsg(...)     _EngineDevMsg(SPT_MESSAGE_PREFIX __VA_ARGS__)
#define EngineWarning(...)    _EngineWarning(SPT_MESSAGE_PREFIX __VA_ARGS__)
#define EngineDevWarning(...) _EngineDevWarning(SPT_MESSAGE_PREFIX __VA_ARGS__)

std::wstring::size_type GetRightmostSlash( const std::wstring& str, std::wstring::size_type pos = std::wstring::npos );
std::wstring GetFileName( const std::wstring& fileNameWithPath );
std::wstring GetFolderName( const std::wstring& fileNameWithPath );
std::wstring NormalizePath(const std::wstring& path);
std::wstring Convert(const std::string& from);
std::string Convert(const std::wstring& from);
