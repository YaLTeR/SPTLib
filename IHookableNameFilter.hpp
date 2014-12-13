#pragma once

#include <set>
#include <string>

#include "IHookableModule.hpp"

class IHookableNameFilter : public IHookableModule
{
public:
	IHookableNameFilter(const std::set<std::wstring>& moduleNames) : m_Names(moduleNames) {};
	virtual bool CanHook(const std::wstring& moduleFullName);
	virtual void TryHookAll(bool needToIntercept);

protected:
	std::set<std::wstring> m_Names;
};
