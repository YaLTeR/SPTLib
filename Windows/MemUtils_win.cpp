#include "../sptlib-stdafx.hpp"

#include "../MemUtils.hpp"
#include "DetoursUtils.hpp"

namespace MemUtils
{
	namespace detail
	{
		void Intercept(const std::wstring& moduleName, size_t n, const std::pair<void**, void*> funcPairs[])
		{
			DetoursUtils::AttachDetours(moduleName, n, funcPairs);
		}

		void RemoveInterception(const std::wstring& moduleName, size_t n, void** const functions[])
		{
			DetoursUtils::DetachDetours(moduleName, n, functions);
		}
	}

	void MarkAsExecutable(void* addr)
	{
		if (!addr)
			return;

		MEMORY_BASIC_INFORMATION mi;
		if (!VirtualQuery(addr, &mi, sizeof(MEMORY_BASIC_INFORMATION)))
			return;

		if (mi.State != MEM_COMMIT)
			return;

		DWORD protect;
		switch (mi.Protect) {
		case PAGE_READONLY:
			protect = PAGE_EXECUTE_READ;
			break;

		case PAGE_READWRITE:
			protect = PAGE_EXECUTE_READWRITE;
			break;

		case PAGE_WRITECOPY:
			protect = PAGE_EXECUTE_WRITECOPY;
			break;

		default:
			return;
		}

		DWORD temp;
		VirtualProtect(addr, 1, protect, &temp);
	}

	void ReplaceBytes(void* addr, size_t length, const uint8_t* newBytes)
	{
		DWORD dwOldProtect;
		auto result = VirtualProtect(addr, length, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		for (size_t i = 0; i < length; ++i)
			*(reinterpret_cast<uint8_t*>(addr) + i) = newBytes[i];

		// The first call might have failed, but the target might have still been accessible.
		if (result)
			VirtualProtect(addr, length, dwOldProtect, &dwOldProtect);
	}

	bool GetModuleInfo(void* moduleHandle, void** moduleBase, size_t* moduleSize)
	{
		if (!moduleHandle)
			return false;

		MODULEINFO Info;
		GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(moduleHandle), &Info, sizeof(Info));

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

	void* GetSymbolAddress(void* moduleHandle, const char* functionName)
	{
		return GetProcAddress(reinterpret_cast<HMODULE>(moduleHandle), functionName);
	}
}
