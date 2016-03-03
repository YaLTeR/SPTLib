#pragma once

#include <cstddef>
#include <string>
using std::size_t;

#include "MemUtils.hpp"
#include "patterns.hpp"

class IHookableModule
{
public:
	virtual ~IHookableModule() {}
	virtual bool CanHook(const std::wstring& moduleFullName) = 0;
	virtual void* GetHandle();
	virtual std::wstring GetName();

	virtual void Hook(const std::wstring& moduleName, void* moduleHandle, void* moduleBase, size_t moduleLength, bool needToIntercept) = 0;
	virtual void Unhook() = 0;
	virtual void Clear();

	virtual void TryHookAll(bool needToIntercept) = 0;

	template<typename Result, size_t N>
	auto FindFunctionAsync(
		Result& address,
		const char* name,
		const std::array<patterns::PatternWrapper, N>& patterns,
		const std::function<void(typename std::array<patterns::PatternWrapper, N>::const_iterator)> onFound = [](auto it) {})
	{
		return MemUtils::find_function_async(address, m_Handle, name, m_Base, m_Length, patterns, onFound);
	}

	template<typename Result, size_t N>
	auto FindAsync(
		Result& address,
		const std::array<patterns::PatternWrapper, N>& patterns,
		const std::function<void(typename std::array<patterns::PatternWrapper, N>::const_iterator)> onFound = [](auto it) {})
	{
		return MemUtils::find_unique_sequence_async(address, m_Base, m_Length, patterns, onFound);
	}

protected:
	void *m_Handle;
	void *m_Base;
	size_t m_Length;
	std::wstring m_Name;
	bool m_Intercepted;
};
