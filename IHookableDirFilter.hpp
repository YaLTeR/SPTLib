#include "sptlib-stdafx.hpp"
#pragma once

#include <set>
#include "IHookableModule.hpp"

class IHookableDirFilter : public IHookableModule
{
public:
	IHookableDirFilter(const std::set<std::wstring>& dirNames) : dirNames(dirNames) {};
	virtual bool CanHook(const std::wstring& moduleFullName);
	virtual void Clear();
	virtual void TryHookAll();

protected:
	std::set<std::wstring> dirNames;
};
