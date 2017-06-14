#include "sptlib-stdafx.hpp"

#include "IHookableModule.hpp"

void* IHookableModule::GetHandle()
{
	return m_Handle;
}

std::wstring IHookableModule::GetName()
{
	return m_Name;
}

void IHookableModule::Clear()
{
	m_Handle = nullptr;
	m_Base = nullptr;
	m_Length = 0;
	m_Name.clear();
	m_Intercepted = false;
}
