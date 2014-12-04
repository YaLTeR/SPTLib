#include "sptlib-stdafx.hpp"

#include "sptlib.hpp"
#include "MemUtils.hpp"
#include "IHookableDirFilter.hpp"

bool IHookableDirFilter::CanHook(const std::wstring& moduleFullName)
{
	return (m_DirNames.find( GetFolderName(moduleFullName) ) != m_DirNames.end());
}

void IHookableDirFilter::TryHookAll(bool needToIntercept)
{
	for (auto handle : MemUtils::GetLoadedModules())
	{
		std::wstring fullName = MemUtils::GetModulePath(handle);
		if ( CanHook(fullName) )
		{
			void* base;
			size_t size;

			if (MemUtils::GetModuleInfo(handle, &base, &size))
			{
				EngineDevMsg("Hooking %s (start: %p; size: %x)...\n", Convert( GetFileName(fullName) ).c_str(), base, size);
				Hook(GetFileName(fullName), handle, base, size, needToIntercept);
				break;
			}
		}
	}
}
