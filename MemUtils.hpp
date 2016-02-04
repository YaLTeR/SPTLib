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

	template<typename T>
	struct identity
	{
		typedef T type;
	};

	enum : ptnvec_size
	{
		INVALID_SEQUENCE_INDEX = std::numeric_limits<ptnvec_size>::max()
	};

	inline bool DataCompare(const byte* data, const byte* pattern, const char* mask);
	void* FindPattern(const void* start, size_t length, const byte* pattern, const char* mask);
	ptnvec_size FindUniqueSequence(const void* start, size_t length, const ptnvec& patterns, void** pAddress = nullptr);
	ptnvec_size FindFirstSequence(const void* start, size_t length, const ptnvec& patterns, void** pAddress = nullptr);
	std::future<ptnvec_size> Find(void** to, void* handle, const std::string& name, const void* start, size_t length, const ptnvec& patterns, const std::function<void(ptnvec_size)>& onFound, const std::function<void(void)>& onNotFound);
	std::future<ptnvec_size> FindPatternOnly(void** to, const void* start, size_t length, const ptnvec& patterns, const std::function<void(ptnvec_size)>& onFound, const std::function<void(void)>& onNotFound);
	
	void ReplaceBytes(void* addr, size_t length, const byte* newBytes);
	void* HookVTable(void** vtable, size_t index, const void* function);

	void AddSymbolLookupHook(void* moduleHandle, void* original, void* target);
	void RemoveSymbolLookupHook(void* moduleHandle, void* original);
	void* GetSymbolLookupResult(void* handle, void* original);

	bool GetModuleInfo(const std::wstring& moduleName, void** moduleHandle, void** moduleBase, size_t* moduleSize);
	bool GetModuleInfo(void* moduleHandle, void** moduleBase, size_t* moduleSize);
	std::wstring GetModulePath(void* moduleHandle);
	std::vector<void*> GetLoadedModules();
	void* GetSymbolAddress(void* moduleHandle, const char* functionName);

	namespace detail
	{
		void Intercept(const std::wstring& moduleName, size_t n, const std::pair<void**, void*> funcPairs[]);
		void RemoveInterception(const std::wstring& moduleName, size_t n, void** const functions[]);

		template<typename FuncType, size_t N>
		inline void Intercept(const std::wstring& moduleName, std::array<std::pair<void**, void*>, N>& funcPairs, FuncType& target, typename identity<FuncType>::type detour)
		{
			funcPairs[N - 1] = { reinterpret_cast<void**>(&target), reinterpret_cast<void*>(detour) };
			Intercept(moduleName, N, funcPairs.data());
		}

		template<typename FuncType, size_t N, typename... Rest>
		inline void Intercept(const std::wstring& moduleName, std::array<std::pair<void**, void*>, N>& funcPairs, FuncType& target, typename identity<FuncType>::type detour, Rest&... rest)
		{
			funcPairs[N - (sizeof...(rest) / 2 + 1)] = { reinterpret_cast<void**>(&target), reinterpret_cast<void*>(detour) };
			Intercept(moduleName, funcPairs, rest...);
		}
	}

	template<typename FuncType>
	inline void Intercept(const std::wstring& moduleName, FuncType& target, typename identity<FuncType>::type detour)
	{
		const std::pair<void**, void*> temp[] = { { reinterpret_cast<void**>(&target), reinterpret_cast<void*>(detour) } };
		detail::Intercept(moduleName, 1, temp);
	}

	template<typename FuncType, typename... Rest>
	inline void Intercept(const std::wstring& moduleName, FuncType& target, typename identity<FuncType>::type detour, Rest&... rest)
	{
		std::array<std::pair<void**, void*>, sizeof...(rest) / 2 + 1> funcPairs;
		funcPairs[0] = { reinterpret_cast<void**>(&target), reinterpret_cast<void*>(detour) };
		detail::Intercept(moduleName, funcPairs, rest...);
	}

	template<typename... FuncType>
	inline void RemoveInterception(const std::wstring& moduleName, FuncType&... functions)
	{
		void** const temp[] = { reinterpret_cast<void**>(&functions)... };
		detail::RemoveInterception(moduleName, sizeof...(functions), temp);
	}
}
