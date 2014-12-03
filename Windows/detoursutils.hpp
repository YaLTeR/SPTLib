#pragma once

#include "sptlib-stdafx.hpp"

namespace DetoursUtils
{
	void AttachDetours(const std::wstring &moduleName, const std::vector<std::pair<PVOID*, PVOID>>& functions);
	void DetachDetours(const std::wstring &moduleName, const std::vector<std::pair<PVOID*, PVOID>>& functions);
}
