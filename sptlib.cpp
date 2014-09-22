#include "sptlib-stdafx.hpp"

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> string_converter;

void ( *_EngineMsg )( const char *format, ... );
void ( *_EngineDevMsg )( const char *format, ... );
void ( *_EngineWarning )( const char *format, ... );
void ( *_EngineDevWarning )( const char *format, ... );

std::wstring::size_type GetRightmostSlash( const std::wstring &str, std::wstring::size_type pos = std::wstring::npos )
{
	std::wstring::size_type slashPos = str.rfind('/', pos),
	                    backSlashPos = str.rfind('\\', pos);

	if (slashPos == std::wstring::npos)
		return backSlashPos;
	else if (backSlashPos == std::wstring::npos)
		return slashPos;
	else
		return std::max(slashPos, backSlashPos);
}

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
	size_t secondSlashPos = GetRightmostSlash(fileNameWithPath);

	if (secondSlashPos != std::wstring::npos)
	{
		size_t firstSlashPos = GetRightmostSlash(fileNameWithPath, (secondSlashPos - 1));

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
