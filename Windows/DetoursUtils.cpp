#include "../sptlib-stdafx.hpp"

#include "../sptlib.hpp"
#include "DetoursUtils.hpp"

namespace DetoursUtils
{
	using namespace std;

	static map<void*, void*> tramp_to_original;
	static mutex tramp_to_original_mutex;

	void AttachDetours(const wstring& moduleName, size_t n, const pair<void**, void*> funcPairs[])
	{
		size_t hook_count = 0;
		for (size_t i = 0; i < n; ++i)
		{
			void** target = funcPairs[i].first;
			void* detour = funcPairs[i].second;
			assert(target);

			if (*target && detour)
			{
				void* original = *target;
				auto status = MH_CreateHook(original, detour, target);
				if (status != MH_OK)
				{
					EngineWarning(
						"Error hooking %s!0x%p => 0x%p: %s.\n",
						Convert(moduleName).c_str(),
						original,
						detour,
						MH_StatusToString(status));
					continue;
				}

				status = MH_QueueEnableHook(original);
				if (status != MH_OK)
				{
					EngineWarning(
						"Error queueing a hook %s!0x%p => 0x%p for enabling: %s.\n",
						Convert(moduleName).c_str(),
						original,
						detour,
						MH_StatusToString(status));
					continue;
				}

				{
					lock_guard<mutex> lock(tramp_to_original_mutex);
					tramp_to_original[*target] = original;
				}
				hook_count++;
			}
		}

		if (hook_count == 0)
		{
			EngineDevMsg(
				"No %s functions to hook.\n",
				Convert(moduleName).c_str());
			return;
		}

		auto status = MH_ApplyQueued();
		if (status == MH_OK)
		{
			EngineDevMsg(
				"Hooked %u %s function%s.\n",
				static_cast<unsigned int>(hook_count),
				Convert(moduleName).c_str(),
				hook_count > 1 ? "s" : "");
		}
		else
		{
			EngineWarning(
				"Error applying %u queued hooks for %s: %s.\n",
				static_cast<unsigned int>(hook_count),
				Convert(moduleName).c_str(),
				MH_StatusToString(status));
		}
	}

	void DetachDetours(const wstring& moduleName, size_t n, void** const functions[])
	{
		size_t hook_count = 0;
		for (size_t i = 0; i < n; ++i)
		{
			void** tramp = functions[i];
			assert(tramp);

			if (*tramp)
			{
				void* original;
				{
					lock_guard<mutex> lock(tramp_to_original_mutex);
					original = tramp_to_original[*tramp];
					tramp_to_original.erase(*tramp);
				}

				auto status = MH_RemoveHook(original);
				if (status != MH_OK)
				{
					EngineWarning(
						"Error unhooking %s!0x%p => 0x%p: %s.\n",
						Convert(moduleName).c_str(),
						original,
						*tramp,
						MH_StatusToString(status));
					continue;
				}

				*tramp = original;
				hook_count++;
			}
		}

		if (hook_count)
		{
			EngineDevMsg(
				"Unhooked %u %s function%s.\n",
				static_cast<unsigned int>(hook_count),
				Convert(moduleName).c_str(),
				hook_count > 1 ? "s" : "");
		} else
		{
			EngineDevMsg(
				"No %s functions to unhook.\n",
				Convert(moduleName).c_str());
			return;
		}
	}
}
