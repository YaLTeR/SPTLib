#pragma once

#include "sptlib-stdafx.hpp"

namespace MemUtils
{
	typedef unsigned char byte;

	typedef struct
	{
		std::string build;
		std::vector<byte> pattern;
		std::string mask;
	} pattern_def_t;

	typedef std::vector<pattern_def_t> ptnvec;
	typedef std::vector<pattern_def_t>::size_type ptnvec_size;

	const ptnvec_size INVALID_SEQUENCE_INDEX = std::numeric_limits<ptnvec_size>::max();

	bool GetModuleInfo(const std::wstring& moduleName, void** moduleHandle, void** moduleBase, size_t* moduleSize);
	bool GetModuleInfo(const void* moduleHandle, void** moduleBase, size_t* moduleSize);
	std::wstring GetModulePath(void* moduleHandle);

	std::vector<void*> GetLoadedModules();

	inline bool DataCompare(const byte* data, const byte* pattern, const char* mask);
	void* FindPattern(const void* start, size_t length, const byte* pattern, const char* mask);

	ptnvec_size FindUniqueSequence(const void* start, size_t length, const ptnvec& patterns, void** pAddress = nullptr);

	void ReplaceBytes(void* addr, size_t length, const byte* newBytes);
	void* HookVTable(void** vtable, size_t index, const void* function);
}
