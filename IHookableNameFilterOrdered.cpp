#include "sptlib-stdafx.hpp"

#include "sptlib.hpp"
#include "MemUtils.hpp"
#include "IHookableNameFilterOrdered.hpp"

bool IHookableNameFilterOrdered::CanHook(const std::wstring& moduleFullName)
{
	// The closer element is to the start, the more priority it gets.
	size_t number = 0;
	bool found = false;
	auto filename = GetFileName(moduleFullName);
	for (auto name : m_Names)
	{
		if (name == filename)
		{
			found = true;
			break;
		}
		number++;
	}

	return found && number < m_HookedNumber;
}

void IHookableNameFilterOrdered::TryHookAll(bool needToIntercept)
{
	size_t number = 0;
	for (auto name : m_Names)
	{
		void* handle;
		void* start;
		size_t size;
		if (MemUtils::GetModuleInfo(name, &handle, &start, &size))
		{
			EngineDevMsg("Hooking %s (start: %p; size: %x)...\n", Convert(name).c_str(), start, size);
			Hook(name, handle, start, size, needToIntercept);
			m_HookedNumber = number;
			break;
		}
		number++;
	}
}

void IHookableNameFilterOrdered::Clear()
{
	m_HookedNumber = m_Names.size();
	IHookableModule::Clear();
}
