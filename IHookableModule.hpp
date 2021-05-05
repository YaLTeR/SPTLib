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
	virtual void* GetBase();
	virtual size_t GetLength();

	virtual void Hook(const std::wstring& moduleName, void* moduleHandle, void* moduleBase, size_t moduleLength, bool needToIntercept) = 0;
	virtual void Unhook() = 0;
	virtual void Clear();

	virtual void TryHookAll(bool needToIntercept) = 0;

	/*
	 * If I don't have these reinterpret_cast<uintptr_t&>'s here, the underlying std::async
	 * will be called a lot of times, each with a different argument type. This will lead to it
	 * generating a huge amount of duplicate code in the binary (over 100 KB). For some reason the
	 * optimizer does not merge the said code together. Anyway, Result is usually some function
	 * pointer, so this works well.
	 */
	template<typename Result, size_t N>
	inline auto FindAsync(
		Result& address,
		const std::array<patterns::PatternWrapper, N>& patterns)
	{
		return MemUtils::find_unique_sequence_async(
			reinterpret_cast<uintptr_t&>(address),
			m_Base,
			m_Length,
			patterns.cbegin(),
			patterns.cend());
	}

	template<typename Result, size_t N>
	inline auto FindAsync(
		Result& address,
		const std::array<patterns::PatternWrapper, N>& patterns,
		std::function<void(typename std::array<patterns::PatternWrapper, N>::const_iterator)> onFound)
	{
		return MemUtils::find_unique_sequence_async(
			reinterpret_cast<uintptr_t&>(address),
			m_Base,
			m_Length,
			patterns.cbegin(),
			patterns.cend(),
			onFound);
	}

	template<typename Result, size_t N>
	inline auto FindFunctionAsync(
		Result& address,
		const char* name,
		const std::array<patterns::PatternWrapper, N>& patterns)
	{
		return MemUtils::find_function_async(
			reinterpret_cast<uintptr_t&>(address),
			m_Handle,
			name,
			m_Base,
			m_Length,
			patterns.cbegin(),
			patterns.cend());
	}

	template<typename Result, size_t N>
	inline auto FindFunctionAsync(
		Result& address,
		const char* name,
		const std::array<patterns::PatternWrapper, N>& patterns,
		std::function<void(typename std::array<patterns::PatternWrapper, N>::const_iterator)> onFound)
	{
		return MemUtils::find_function_async(
			reinterpret_cast<uintptr_t&>(address),
			m_Handle,
			name,
			m_Base,
			m_Length,
			patterns.cbegin(),
			patterns.cend(),
			onFound);
	}

protected:
	void *m_Handle;
	void *m_Base;
	size_t m_Length;
	std::wstring m_Name;
	bool m_Intercepted;
};
