#include "sptlib-stdafx.hpp"

#include "sptlib.hpp"
#include "memutils.hpp"
#include "IHookableNameFilter.hpp"

bool IHookableNameFilter::CanHook(const std::wstring& moduleFullName)
{
	return (m_Names.find( GetFileName(moduleFullName) ) != m_Names.end());
}

void IHookableNameFilter::Clear()
{
	IHookableModule::Clear();
}

void IHookableNameFilter::TryHookAll()
{
	for (auto name : m_Names)
	{
		void* handle;
		void* start;
		size_t size;
		if (MemUtils::GetModuleInfo(name, &handle, &start, &size))
		{
			EngineDevMsg("Hooking %s (start: %p; size: %x)...\n", Convert(name).c_str(), start, size);
			Hook(name, handle, start, size);
			break;
		}
	}
}
