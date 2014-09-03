#include "sptlib-stdafx.hpp"

#include <detours.h>
#include "detoursutils.hpp"
#include "sptlib.hpp"

#pragma comment (lib, "detours.lib")

void AttachDetours( const std::wstring &moduleName, const std::vector<std::pair<PVOID*, PVOID>>& functions )
{
	// Check if we need to detour something.
	bool needToDetour = false;
	for (auto funcPair : functions)
	{
		PVOID *pFunctionToDetour = funcPair.first;
		PVOID functionToDetourWith = funcPair.second;

		if ((pFunctionToDetour && *pFunctionToDetour) && functionToDetourWith)
		{
			// We have something to detour!
			needToDetour = true;
			break;
		}
	}

	// We don't have anything to detour.
	if (!needToDetour)
	{
		EngineDevMsg("No %s functions to detour!\n", string_converter.to_bytes(moduleName).c_str());
		return;
	}

	DetourTransactionBegin();
	DetourUpdateThread( GetCurrentThread() );

	unsigned int detourCount = 0;
	for (auto funcPair : functions)
	{
		PVOID *pFunctionToDetour = funcPair.first;
		PVOID functionToDetourWith = funcPair.second;

		if ((pFunctionToDetour && *pFunctionToDetour) && functionToDetourWith)
		{
			DetourAttach( pFunctionToDetour, functionToDetourWith );
			detourCount++;
		}
	}

	LONG error = DetourTransactionCommit();
	if (error == NO_ERROR)
	{
		EngineDevMsg("Detoured %d %s function(s).\n", detourCount, string_converter.to_bytes(moduleName).c_str());
	}
	else
	{
		EngineWarning("Error detouring %d %s function(s): %d.\n", detourCount, string_converter.to_bytes(moduleName).c_str(), error);
	}
}

void DetachDetours( const std::wstring &moduleName, const std::vector<std::pair<PVOID*, PVOID>>& functions )
{
	// Check if we need to undetour something.
	bool needToUndetour = false;
	for (auto funcPair : functions)
	{
		PVOID *pFunctionToUndetour = funcPair.first;
		PVOID functionReplacement = funcPair.second;

		if ((pFunctionToUndetour && *pFunctionToUndetour) && functionReplacement)
		{
			// We have something to detour!
			needToUndetour = true;
			break;
		}
	}

	// We don't have anything to undetour.
	if (!needToUndetour)
	{
		EngineDevMsg("No %s functions to undetour!\n", string_converter.to_bytes(moduleName).c_str());
		return;
	}

	DetourTransactionBegin();
	DetourUpdateThread( GetCurrentThread() );

	unsigned int detourCount = 0;
	for (auto funcPair : functions)
	{
		PVOID *pFunctionToUndetour = funcPair.first;
		PVOID functionReplacement = funcPair.second;

		if ((pFunctionToUndetour && *pFunctionToUndetour) && functionReplacement)
		{
			DetourDetach( pFunctionToUndetour, functionReplacement );
			detourCount++;
		}
	}

	LONG error = DetourTransactionCommit();
	if (error == NO_ERROR)
	{
		EngineDevMsg("Removed %d %s function detour(s).\n", detourCount, string_converter.to_bytes(moduleName).c_str());
	}
	else
	{
		EngineWarning("Error removing %d %s function detour(s): %d.\n", detourCount, string_converter.to_bytes(moduleName).c_str(), error);
	}
}
