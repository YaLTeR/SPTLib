#include "sptlib-stdafx.hpp"

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> string_converter;

void ( *_EngineMsg )( const char *format, ... );
void ( *_EngineDevMsg )( const char *format, ... );
void ( *_EngineWarning )( const char *format, ... );
void ( *_EngineDevWarning )( const char *format, ... );

std::wstring GetFileName( const std::wstring &fileNameWithPath )
{
	size_t slashPos = fileNameWithPath.rfind('/');
	if (slashPos != std::wstring::npos)
	{
		return std::wstring( fileNameWithPath, (slashPos + 1) );
	}
	else
	{
		slashPos = fileNameWithPath.rfind('\\');
		if (slashPos != std::wstring::npos)
		{
			return std::wstring( fileNameWithPath, (slashPos + 1) );
		}
	}

	return fileNameWithPath;
}

std::wstring GetFolderName(const std::wstring &fileNameWithPath)
{
	size_t secondSlashPos = fileNameWithPath.rfind('\\');
	if (secondSlashPos == std::wstring::npos)
		secondSlashPos = fileNameWithPath.rfind('/');

	if (secondSlashPos != std::wstring::npos)
	{
		size_t firstSlashPos = fileNameWithPath.rfind('\\', (secondSlashPos - 1));
		if (firstSlashPos == std::wstring::npos)
			firstSlashPos = fileNameWithPath.rfind('/', (secondSlashPos - 1));

		if (firstSlashPos != std::wstring::npos)
		{
			return std::wstring(fileNameWithPath, (firstSlashPos + 1), (secondSlashPos - firstSlashPos - 1));
		}
		else
		{
			return std::wstring(fileNameWithPath, 0, secondSlashPos);
		}
	}

	return fileNameWithPath;
}
