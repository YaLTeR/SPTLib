#include "sptlib-stdafx.hpp"

#include "MemUtils.hpp"
#include "sptlib.hpp"

namespace MemUtils
{
	static std::unordered_map< void*, std::unordered_map<void*, void*> > symbolLookupHooks;
	static std::mutex symbolLookupHookMutex;

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
