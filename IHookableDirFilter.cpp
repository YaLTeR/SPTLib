#include "sptlib-stdafx.hpp"

#include "sptlib.hpp"
#include "memutils.hpp"
#include "IHookableDirFilter.hpp"

using std::uintptr_t;
using std::size_t;

bool IHookableDirFilter::CanHook(const std::wstring& moduleFullName)
{
	return (dirNames.find( GetFolderName(moduleFullName) ) != dirNames.end());
}

void IHookableDirFilter::Clear()
{
	IHookableModule::Clear();
}

void IHookableDirFilter::TryHookAll()
{
	for (auto module : MemUtils::GetLoadedModules())
	{
		WCHAR moduleName[MAX_PATH];
		GetModuleFileNameW(module, moduleName, MAX_PATH * sizeof(WCHAR));

		if (dirNames.find( GetFolderName(std::wstring(moduleName)) ) != dirNames.end())
		{
			uintptr_t start;
			size_t size;

			if (MemUtils::GetModuleInfo(moduleName, &hModule, &start, &size))
			{
				EngineDevMsg("Hooking %s (start: %p; size: %x)...\n", string_converter.to_bytes(moduleName).c_str(), start, size);
				Hook(moduleName, hModule, start, size);
				break;
			}
		}
	}
}
