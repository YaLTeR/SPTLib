#include "sptlib-stdafx.hpp"

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
	size_t slashPos = GetRightmostSlash(fileNameWithPath);
	if (slashPos != std::wstring::npos)
		return std::wstring( fileNameWithPath, (slashPos + 1) );

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

std::wstring NormalizePath(const std::wstring& path)
{
	std::wstring ret(path);
	for (auto slash = GetRightmostSlash(ret); slash != std::wstring::npos && slash > 1; slash = GetRightmostSlash(ret, slash - 1))
	{
		if (ret[slash - 1] == '.' && GetRightmostSlash(ret, slash - 2) == (slash - 2))
		{
			ret.erase(slash - 2, 2);
			slash -= 2;
		}
	}
	
	return ret;
}

std::wstring Convert(const std::string& from)
{
	std::string previousLocale(std::setlocale(LC_ALL, NULL));
	std::setlocale(LC_ALL, "");

	std::wstring result(from.size(), L' ');
	auto fromCStr = from.c_str();
	auto state = std::mbstate_t();
	auto length = std::mbsrtowcs(&result[0], &fromCStr, from.size(), &state);
	if (length != static_cast<std::size_t>(-1))
		result.resize(length);

	std::setlocale(LC_ALL, previousLocale.c_str());
	return result;
}

std::string Convert(const std::wstring& from)
{
	std::string previousLocale(std::setlocale(LC_ALL, NULL));
	std::setlocale(LC_ALL, "");

	auto fromCStr = from.c_str();
	auto state = std::mbstate_t();
	auto length = std::wcsrtombs(NULL, &fromCStr, from.size(), &state);
	std::string result;
	if (length != static_cast<std::size_t>(-1))
	{
		result.resize(length);
		std::wcsrtombs(&result[0], &fromCStr, from.size(), &state);
	}

	std::setlocale(LC_ALL, previousLocale.c_str());
	return result;
}
