#pragma once

#include "sptlib-stdafx.hpp"
#include "IHookableModule.hpp"

class IHookableDirFilter : public IHookableModule
{
public:
	IHookableDirFilter(const std::set<std::wstring>& dirNames) : m_DirNames(dirNames) {};
	virtual bool CanHook(const std::wstring& moduleFullName);
	virtual void Clear();
	virtual void TryHookAll();

protected:
	std::set<std::wstring> m_DirNames;
};
