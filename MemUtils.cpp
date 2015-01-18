#include "sptlib-stdafx.hpp"

#include "MemUtils.hpp"
#include "sptlib.hpp"

namespace MemUtils
{
	static std::unordered_map< void*, std::unordered_map<void*, void*> > symbolLookupHooks;
	static std::mutex symbolLookupHookMutex;

	inline bool DataCompare(const byte* data, const byte* pattern, const char* mask)
	{
		for (; *mask != 0; ++data, ++pattern, ++mask)
			if (*mask == 'x' && *data != *pattern)
				return false;

		return (*mask == 0);
	}

	void* FindPattern(const void* start, size_t length, const byte* pattern, const char* mask)
	{
		auto maskLength = strlen(mask);
		for (size_t i = 0; i <= length - maskLength; ++i)
		{
			auto addr = reinterpret_cast<const byte*>(start) + i;
			if (DataCompare(addr, pattern, mask))
				return const_cast<void*>(reinterpret_cast<const void*>(addr));
		}

		return nullptr;
	}

	ptnvec_size FindUniqueSequence(const void* start, size_t length, const ptnvec& patterns, void** pAddress)
	{
		for (ptnvec_size i = 0; i < patterns.size(); i++)
		{
			auto address = FindPattern(start, length, patterns[i].pattern.data(), patterns[i].mask.c_str());
			if (address)
			{
				size_t newSize = length - (reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(start) + 1);
				if ( !FindPattern(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + 1), newSize, patterns[i].pattern.data(), patterns[i].mask.c_str()) )
				{
					if (pAddress)
						*pAddress = address;

					return i; // Return the number of the pattern.
				}
				else
				{
					if (pAddress)
						*pAddress = nullptr;

					return INVALID_SEQUENCE_INDEX; // Bogus sequence.
				}
			}
		}

		if (pAddress)
			*pAddress = nullptr;

		return INVALID_SEQUENCE_INDEX; // Didn't find anything.
	}

	std::future<ptnvec_size> Find(void** to, void* handle, const std::string& name, const void* start, size_t length, const ptnvec& patterns, const std::function<void(ptnvec_size)>& onFound, const std::function<void(void)>& onNotFound)
	{
		return std::async([=]() -> ptnvec_size {
			ptnvec_size ptnNumber = INVALID_SEQUENCE_INDEX;
			*to = GetSymbolAddress(handle, name.c_str());
			if (!*to)
				ptnNumber = FindUniqueSequence(start, length, patterns, to);
			if (*to)
				onFound(ptnNumber);
			else
				onNotFound();
			return ptnNumber;
		});
	}

	std::future<ptnvec_size> FindPatternOnly(void** to, const void* start, size_t length, const ptnvec& patterns, const std::function<void(ptnvec_size)>& onFound, const std::function<void(void)>& onNotFound)
	{
		return std::async([=]() -> ptnvec_size {
			auto ptnNumber = FindUniqueSequence(start, length, patterns, to);
			if (*to)
				onFound(ptnNumber);
			else
				onNotFound();
			return ptnNumber;
		});
	}

	void* HookVTable(void** vtable, size_t index, const void* function)
	{
		auto oldFunction = vtable[index];

		ReplaceBytes(&(vtable[index]), sizeof(void*), reinterpret_cast<byte*>(&function));

		return oldFunction;
	}

	void AddSymbolLookupHook(void* moduleHandle, void* original, void* target)
	{
		if (!original)
			return;

		std::lock_guard<std::mutex> lock(symbolLookupHookMutex);
		symbolLookupHooks[moduleHandle][original] = target;
	}

	void RemoveSymbolLookupHook(void* moduleHandle, void* original)
	{
		if (!original)
			return;

		std::lock_guard<std::mutex> lock(symbolLookupHookMutex);
		symbolLookupHooks[moduleHandle].erase(original);
	}

	void* GetSymbolLookupResult(void* handle, void* original)
	{
		if (!original)
			return nullptr;
		
		std::lock_guard<std::mutex> lock(symbolLookupHookMutex);
		auto hook = symbolLookupHooks[handle].find(original);
		if (hook == symbolLookupHooks[handle].end())
			return original;
		else
			return hook->second;
	}
}
