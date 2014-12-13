#pragma once

#include <set>
#include <string>

#include "IHookableModule.hpp"

class IHookableDirFilter : public IHookableModule
{
public:
	IHookableDirFilter(const std::set<std::wstring>& dirNames) : m_DirNames(dirNames) {};
	virtual bool CanHook(const std::wstring& moduleFullName);
	virtual void TryHookAll(bool needToIntercept);

protected:
	std::set<std::wstring> m_DirNames;
};
