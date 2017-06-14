#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "IHookableModule.hpp"
using std::size_t;

class IHookableNameFilterOrdered : public IHookableModule
{
public:
	IHookableNameFilterOrdered(const std::vector<std::wstring>& moduleNames) : m_Names(moduleNames), m_HookedNumber(m_Names.size()) {};
	virtual bool CanHook(const std::wstring& moduleFullName);
	virtual void TryHookAll(bool needToIntercept);
	virtual void Clear();

protected:
	std::vector<std::wstring> m_Names;
	size_t m_HookedNumber;
};
