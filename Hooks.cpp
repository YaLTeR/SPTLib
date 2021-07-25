#include "sptlib-stdafx.hpp"

#include "Hooks.hpp"
#include "MemUtils.hpp"
#include "sptlib.hpp"

namespace Hooks
{
#ifdef _WIN32
	std::vector<IHookableModule*> modules;
#else
	// Priority must be higher than that of `Construct()` in Bunnymod XT's main_linux.cpp
	std::vector<IHookableModule*> modules __attribute__((init_priority(101)));
#endif

	static bool intercepted;

	bool DebugEnabled()
	{
		static auto sptlibDebug = std::getenv("SPTLIB_DEBUG");
		return sptlibDebug && (sptlibDebug[0] == '1');
	}

	void Init(bool needToIntercept)
	{
		_EngineDevMsg("SPTLib version " SPTLIB_VERSION ".\n");
		EngineDevMsg("Modules contain %d entries.\n", modules.size());

		InitInterception(needToIntercept);

		// Try hooking each module in case it is already loaded
		for (auto it = modules.cbegin(); it != modules.cend(); ++it)
		{
			auto p = *it;
			p->TryHookAll(needToIntercept);
		}

		intercepted = needToIntercept;
	}

	void Free()
	{
		EngineDevMsg("Modules contain %d entries.\n", modules.size());

		// Unhook everything
		for (auto it = modules.begin(); it != modules.end(); ++it)
		{
			EngineDevMsg("Unhooking %s...\n", Convert((*it)->GetName()).c_str());
			(*it)->Unhook();
		}

		ClearInterception(intercepted);

		Clear();
	}

	void Clear()
	{
		modules.clear();
	}

	void HookModule(std::wstring name)
	{
		void *handle = nullptr;
		void *base = nullptr;
		size_t size = 0;

		for (auto it = modules.cbegin(); it != modules.cend(); ++it)
		{
			if ((*it)->CanHook(name))
			{
				if (handle || MemUtils::GetModuleInfo(name, &handle, &base, &size))
				{
					EngineDevMsg("Hooking %s (start: %p; size: %x)...\n", Convert(name).c_str(), base, size);
					(*it)->Unhook(); // Unhook first since it might have been hooked (with a different DLL).
					(*it)->Hook(name, handle, base, size, intercepted);
				}
				else
				{
					EngineWarning("Unable to obtain the %s module info!\n", Convert(name).c_str());
					return;
				}
			}
		}

		if (!handle)
		{
			if (DebugEnabled())
				EngineDevMsg("Tried to hook an unlisted module: %s\n", Convert(name).c_str());
		}
	}

	void UnhookModule(std::wstring name)
	{
		bool unhookedSomething = false;
		for (auto it = modules.cbegin(); it != modules.cend(); ++it)
		{
			if ((*it)->GetName().compare(name) == 0)
			{
				EngineDevMsg("Unhooking %s...\n", Convert(name).c_str());
				(*it)->Unhook();
				unhookedSomething = true;
			}
		}

		if (!unhookedSomething)
		{
			EngineDevMsg("Tried to unhook an unlisted module: %s\n", Convert(name).c_str());
		}
	}

	void AddToHookedModules(IHookableModule* module)
	{
		if (!module)
		{
			EngineWarning("Tried to add a nullptr module!\n");
			return;
		}

		modules.push_back(module);
	}
}
