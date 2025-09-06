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
		struct HookPair
		{
			void** pTramp;
			void* original;
		};
		std::vector<HookPair> hooks;
		hooks.reserve(n);

		{
			lock_guard<mutex> lock(tramp_to_original_mutex);
			for (size_t i = 0; i < n; ++i)
			{
				void** tramp = functions[i];
				if (!tramp || !*tramp)
					continue;

				auto it = tramp_to_original.find(*tramp);
				if (it == tramp_to_original.end())
				{
					EngineWarning("Error unhooking %s!0x%p missing mapping for trampoline.\n",
					              Convert(moduleName).c_str(),
					              *tramp);
					continue;
				}
				hooks.push_back({tramp, it->second});
				tramp_to_original.erase(it);
			}
		}

		// Even if queue disable failed, we will still call remove hook later. Report errors there.
		for (const auto& h : hooks)
			MH_QueueDisableHook(h.original);

		MH_ApplyQueued();

		size_t hook_count = 0;
		for (const auto& h : hooks)
		{
			auto status = MH_RemoveHook(h.original);
			if (status == MH_OK)
			{
				*h.pTramp = h.original;
				hook_count++;
			}
			else
			{
				EngineWarning("Error removing %s!0x%p => 0x%p: %s.\n",
				              Convert(moduleName).c_str(),
				              h.original,
				              *h.pTramp,
				              MH_StatusToString(status));
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
