#pragma once

// This is used only by SPTLib itself and we know that every SPTLib .cpp file contains
// a sptlib-stdafx.hpp include in the very beginning, which has all the required headers.

namespace DetoursUtils
{
	void AttachDetours(const std::wstring& moduleName, const std::vector<std::pair<PVOID*, PVOID>>& functions);
	void DetachDetours(const std::wstring& moduleName, const std::vector<std::pair<PVOID*, PVOID>>& functions);
}
