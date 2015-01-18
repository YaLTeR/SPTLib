#pragma once

#include <cstddef>
#include <functional>
#include <future>
#include <limits>
#include <string>
#include <utility>
#include <vector>
using std::size_t;

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

	inline bool DataCompare(const byte* data, const byte* pattern, const char* mask);
	void* FindPattern(const void* start, size_t length, const byte* pattern, const char* mask);
	ptnvec_size FindUniqueSequence(const void* start, size_t length, const ptnvec& patterns, void** pAddress = nullptr);
	std::future<ptnvec_size> Find(void** to, void* handle, const std::string& name, const void* start, size_t length, const ptnvec& patterns, const std::function<void(ptnvec_size)>& onFound, const std::function<void(void)>& onNotFound);
	
	void ReplaceBytes(void* addr, size_t length, const byte* newBytes);
	void* HookVTable(void** vtable, size_t index, const void* function);

	void Intercept(const std::wstring& moduleName, const std::vector<std::pair<void**, void*>>& functions);
	void RemoveInterception(const std::wstring& moduleName, const std::vector<std::pair<void**, void*>>& functions);
	void AddSymbolLookupHook(void* moduleHandle, void* original, void* target);
	void RemoveSymbolLookupHook(void* moduleHandle, void* original);
	void* GetSymbolLookupResult(void* handle, void* original);

	bool GetModuleInfo(const std::wstring& moduleName, void** moduleHandle, void** moduleBase, size_t* moduleSize);
	bool GetModuleInfo(void* moduleHandle, void** moduleBase, size_t* moduleSize);
	std::wstring GetModulePath(void* moduleHandle);
	std::vector<void*> GetLoadedModules();
	void* GetSymbolAddress(void* moduleHandle, const char* functionName);
}
