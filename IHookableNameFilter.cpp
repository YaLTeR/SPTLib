#include "sptlib-stdafx.hpp"

#include "sptlib.hpp"
#include "MemUtils.hpp"
#include "IHookableNameFilter.hpp"

bool IHookableNameFilter::CanHook(const std::wstring& moduleFullName)
{
	const auto filename = GetFileName(moduleFullName);
	return (filename != GetName() && m_Names.find(filename) != m_Names.end());
}

void IHookableNameFilter::TryHookAll(bool needToIntercept)
{
	for (auto name : m_Names)
	{
		void* handle;
		void* start;
		size_t size;
		if (MemUtils::GetModuleInfo(name, &handle, &start, &size))
		{
			EngineDevMsg("Hooking %s (start: %p; size: %x)...\n", Convert(name).c_str(), start, size);
			Hook(name, handle, start, size, needToIntercept);
			break;
		}
	}
}
