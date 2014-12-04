#include "sptlib-stdafx.hpp"

#include <Psapi.h>
#include "../MemUtils.hpp"

#pragma comment( lib, "psapi.lib" )

namespace MemUtils
{
	bool GetModuleInfo(void* moduleHandle, void** moduleBase, size_t* moduleSize)
	{
		if (!moduleHandle)
			return false;

		MODULEINFO Info;
		GetModuleInformation(GetCurrentProcess(), static_cast<HMODULE>(moduleHandle), &Info, sizeof(Info));

		if (moduleBase)
			*moduleBase = Info.lpBaseOfDll;

		if (moduleSize)
			*moduleSize = (size_t)Info.SizeOfImage;

		return true;
	}

	bool GetModuleInfo(const std::wstring& moduleName, void** moduleHandle, void** moduleBase, size_t* moduleSize)
	{
		HMODULE Handle = GetModuleHandleW(moduleName.c_str());
		auto ret = GetModuleInfo(Handle, moduleBase, moduleSize);

		if (ret && moduleHandle)
			*moduleHandle = Handle;

		return ret;
	}

	std::wstring GetModulePath(void* moduleHandle)
	{
		WCHAR path[MAX_PATH];
		GetModuleFileNameW(reinterpret_cast<HMODULE>(moduleHandle), path, MAX_PATH);
		return std::wstring(path);
	}
	
	std::vector<void*> GetLoadedModules()
	{
		std::vector<void*> out;

		HMODULE modules[1024];
		DWORD sizeNeeded;

		if (EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &sizeNeeded))
		{
			sizeNeeded = std::min(1024ul, (sizeNeeded / sizeof(HMODULE)));
			for (unsigned long i = 0; i < sizeNeeded; ++i)
				out.push_back(modules[i]);
		}

		return out;
	}

	void ReplaceBytes(void* addr, size_t length, const byte* newBytes)
	{
		DWORD dwOldProtect;
		auto result = VirtualProtect(addr, length, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		for (size_t i = 0; i < length; ++i)
			*(reinterpret_cast<byte*>(addr) + i) = newBytes[i];

		// The first call might have failed, but the target might have still been accessible.
		if (result)
			VirtualProtect(addr, length, dwOldProtect, &dwOldProtect);
	}
}
