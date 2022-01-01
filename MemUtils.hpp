#pragma once

#include "patterns.hpp"

namespace MemUtils
{
	using namespace patterns;

	bool GetModuleInfo(const std::wstring& moduleName, void** moduleHandle, void** moduleBase, size_t* moduleSize);
	bool GetModuleInfo(void* moduleHandle, void** moduleBase, size_t* moduleSize);
	std::wstring GetModulePath(void* moduleHandle);
	std::vector<void*> GetLoadedModules();
	void* GetSymbolAddress(void* moduleHandle, const char* functionName);

	inline uintptr_t find_pattern(const void* start, size_t length, const PatternWrapper& pattern)
	{
		if (length < pattern.length())
			return 0;

		auto p = static_cast<const uint8_t*>(start);
		for (auto end = p + length - pattern.length(); p <= end; ++p) {
			if (pattern.match(p))
				return reinterpret_cast<uintptr_t>(p);
		}

		return 0;
	}

	template<class Iterator>
	Iterator find_first_sequence(
		const void* start,
		size_t length,
		Iterator begin,
		Iterator end,
		uintptr_t& address)
	{
		for (auto pattern = begin; pattern != end; ++pattern) {
			address = find_pattern(start, length, *pattern);
			if (address)
				return pattern;
		}
		address = 0;
		return end;
	}

	template<typename Result, class Iterator>
	inline Iterator find_first_sequence(
		const void* start,
		size_t length,
		Iterator begin,
		Iterator end,
		Result& address)
	{
		uintptr_t addr;
		auto rv = find_first_sequence(start, length, begin, end, addr);
		// C-style cast... Because reinterpret_cast can't cast uintptr_t to integral types.
		address = (Result) addr;
		return rv;
	}

	template<class Iterator>
	Iterator find_unique_sequence(
		const void* start,
		size_t length,
		Iterator begin,
		Iterator end,
		uintptr_t& address)
	{
		auto pattern = find_first_sequence(start, length, begin, end, address);
		if (pattern != end) {
			// length != 0
			// start <= addr < start + length
			// 0 <= addr - start < length
			// 1 <= addr - start + 1 < length + 1
			// -length - 1 < -(addr - start + 1) <= -1
			// 0 <= length - (addr - start + 1) <= length - 1
			auto new_length = length - (address - reinterpret_cast<uintptr_t>(start) + 1);

			uintptr_t temp;
			if (find_first_sequence(reinterpret_cast<const void*>(address + 1), new_length, begin, end, temp) != end) {
				// Ambiguous.
				address = 0;
				return end;
			} else {
				return pattern;
			}
		}

		address = 0;
		return end;
	}

	template<typename Result, class Iterator>
	inline Iterator find_unique_sequence(
		const void* start,
		size_t length,
		Iterator begin,
		Iterator end,
		Result& address)
	{
		uintptr_t addr;
		auto rv = find_unique_sequence(start, length, begin, end, addr);
		// C-style cast... Because reinterpret_cast can't cast uintptr_t to integral types.
		address = (Result) addr;
		return rv;
	}

	template<typename Result, class Iterator>
	inline auto find_unique_sequence_async(
		Result& address,
		const void* start,
		size_t length,
		Iterator begin,
		Iterator end)
	{
		return std::async(find_unique_sequence<Result, Iterator>, start, length, begin, end, std::ref(address));
	}

	template<typename Result, class Iterator>
	auto find_unique_sequence_async(
		Result& address,
		const void* start,
		size_t length,
		Iterator begin,
		Iterator end,
		std::function<void(Iterator)> onFound)
	{
		return std::async([=, &address]() {
			auto it = find_unique_sequence(start, length, begin, end, address);
			if (address)
				onFound(it);
			return it;
		});
	}

	template<typename Result, class Iterator>
	auto find_function_async(
		Result& address,
		void* handle,
		const char* name,
		const void* start,
		size_t length,
		Iterator begin,
		Iterator end)
	{
		return std::async([=, &address]() {
			auto it = end;
			address = reinterpret_cast<Result>(GetSymbolAddress(handle, name));
			if (!address)
				it = find_unique_sequence(start, length, begin, end, address);
			return it;
		});
	}

	template<typename Result, class Iterator>
	auto find_function_async(
		Result& address,
		void* handle,
		const char* name,
		const void* start,
		size_t length,
		Iterator begin,
		Iterator end,
		std::function<void(Iterator)> onFound)
	{
		return std::async([=, &address]() {
			auto it = end;
			address = reinterpret_cast<Result>(GetSymbolAddress(handle, name));
			if (!address)
				it = find_unique_sequence(start, length, begin, end, address);
			if (address)
				onFound(it);
			return it;
		});
	}

	template<typename Iterator>
	std::vector<MatchedPattern> find_all_sequences(const void* start, size_t length, Iterator begin, Iterator end)
	{
		std::vector<MatchedPattern> result;
		for (auto pattern = begin; pattern != end; ++pattern)
		{
			const void* currentStart = start;
			size_t currentLength = length;
			uintptr_t address;
			do
			{
				address = find_pattern(currentStart, currentLength, *pattern);
				if (address)
				{
					MatchedPattern match;
					match.ptnIndex = pattern - begin;
					match.ptr = address;

					result.push_back(match);
					currentStart = reinterpret_cast<void*>(address + 1);
					currentLength = length
					                - (reinterpret_cast<uintptr_t>(currentStart)
					                   - reinterpret_cast<uintptr_t>(start));
				}
			} while (address);
		}

		return result;
	}

	template<typename Iterator>
	std::future<std::vector<MatchedPattern>> find_all_sequences_async(const void* start,
	                                                                  size_t length,
	                                                                  Iterator begin,
	                                                                  Iterator end)
	{
		return std::async(find_all_sequences<Iterator>, start, length, begin, end);
	}

	template<typename T>
	inline void MarkAsExecutable(T addr)
	{
		MarkAsExecutable(reinterpret_cast<void*>(addr));
	}
	void MarkAsExecutable(void* addr);

	void ReplaceBytes(void* addr, size_t length, const uint8_t* newBytes);
	void* HookVTable(void** vtable, size_t index, const void* function);

	void AddSymbolLookupHook(void* moduleHandle, void* original, void* target);
	void RemoveSymbolLookupHook(void* moduleHandle, void* original);
	void* GetSymbolLookupResult(void* handle, void* original);

	template<typename T>
	struct identity
	{
		typedef T type;
	};

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
