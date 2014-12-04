#include "sptlib-stdafx.hpp"

#include <detours.h>
#include "DetoursUtils.hpp"
#include "sptlib.hpp"

#pragma comment (lib, "detours.lib")

void DetoursUtils::AttachDetours( const std::wstring& moduleName, const std::vector<std::pair<PVOID*, PVOID>>& functions )
{
	unsigned int detourCount = 0;
	for (auto funcPair : functions)
	{
		PVOID *pFunctionToDetour = funcPair.first;
		PVOID functionToDetourWith = funcPair.second;

		if ((pFunctionToDetour && *pFunctionToDetour) && functionToDetourWith)
		{
			// We have something to detour!
			if (detourCount == 0)
			{
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
			}

			detourCount++;
			DetourAttach(pFunctionToDetour, functionToDetourWith);
		}
	}

	if (detourCount == 0)
	{
		EngineDevMsg("No %s functions to detour!\n", Convert(moduleName).c_str());
	}
	else
	{
		LONG error = DetourTransactionCommit();
		if (error == NO_ERROR)
		{
			EngineDevMsg("Detoured %d %s function(s).\n", detourCount, Convert(moduleName).c_str());
		}
		else
		{
			EngineWarning("Error detouring %d %s function(s): %d.\n", detourCount, Convert(moduleName).c_str(), error);
		}
	}
}

void DetoursUtils::DetachDetours( const std::wstring& moduleName, const std::vector<std::pair<PVOID*, PVOID>>& functions )
{
	unsigned int detourCount = 0;
	for (auto funcPair : functions)
	{
		PVOID *pFunctionToUndetour = funcPair.first;
		PVOID functionReplacement = funcPair.second;

		if ((pFunctionToUndetour && *pFunctionToUndetour) && functionReplacement)
		{
			// We have something to undetour!
			if (detourCount == 0)
			{
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
			}

			detourCount++;
			DetourDetach(pFunctionToUndetour, functionReplacement);
		}
	}

	if (detourCount == 0)
	{
		EngineDevMsg("No %s functions to undetour!\n", Convert(moduleName).c_str());
		return;
	}
	else
	{
		LONG error = DetourTransactionCommit();
		if (error == NO_ERROR)
		{
			EngineDevMsg("Removed %d %s function detour(s).\n", detourCount, Convert(moduleName).c_str());
		}
		else
		{
			EngineWarning("Error removing %d %s function detour(s): %d.\n", detourCount, Convert(moduleName).c_str(), error);
		}
	}
}
