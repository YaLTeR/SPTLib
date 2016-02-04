#include "../sptlib-stdafx.hpp"

#include "../sptlib.hpp"
#include "../Hooks.hpp"
#include "../MemUtils.hpp"

namespace Hooks
{
	typedef HMODULE(WINAPI *_LoadLibraryA) (LPCSTR lpLFileName);
	typedef HMODULE(WINAPI *_LoadLibraryW) (LPCWSTR lpFileName);
	typedef HMODULE(WINAPI *_LoadLibraryExA) (LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags);
	typedef HMODULE(WINAPI *_LoadLibraryExW) (LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);
	typedef BOOL(WINAPI *_FreeLibrary) (HMODULE hModule);

	static _LoadLibraryA   ORIG_LoadLibraryA;
	static _LoadLibraryW   ORIG_LoadLibraryW;
	static _LoadLibraryExA ORIG_LoadLibraryExA;
	static _LoadLibraryExW ORIG_LoadLibraryExW;
	static _FreeLibrary    ORIG_FreeLibrary;

	static HMODULE WINAPI HOOKED_LoadLibraryA(LPCSTR lpFileName)
	{
		HMODULE rv = ORIG_LoadLibraryA(lpFileName);

		if (DebugEnabled())
			EngineDevMsg("Engine call: LoadLibraryA( \"%s\" ) => %p\n", lpFileName, rv);

		if (rv != NULL)
		{
			HookModule(Convert(lpFileName));
		}

		return rv;
	}

	static HMODULE WINAPI HOOKED_LoadLibraryW(LPCWSTR lpFileName)
	{
		HMODULE rv = ORIG_LoadLibraryW(lpFileName);

		if (DebugEnabled())
			EngineDevMsg("Engine call: LoadLibraryW( \"%s\" ) => %p\n", Convert(lpFileName).c_str(), rv);

		if (rv != NULL)
		{
			HookModule(std::wstring(lpFileName));
		}

		return rv;
	}

	static HMODULE WINAPI HOOKED_LoadLibraryExA(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags)
	{
		HMODULE rv = ORIG_LoadLibraryExA(lpFileName, hFile, dwFlags);

		if (DebugEnabled())
			EngineDevMsg("Engine call: LoadLibraryExA( \"%s\" ) => %p\n", lpFileName, rv);

		if (rv != NULL)
		{
			HookModule(Convert(lpFileName));
		}

		return rv;
	}

	static HMODULE WINAPI HOOKED_LoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags)
	{
		HMODULE rv = ORIG_LoadLibraryExW(lpFileName, hFile, dwFlags);

		if (DebugEnabled())
			EngineDevMsg("Engine call: LoadLibraryExW( \"%s\" ) => %p\n", Convert(lpFileName).c_str(), rv);

		if (rv != NULL)
		{
			HookModule(std::wstring(lpFileName));
		}

		return rv;
	}

	static BOOL WINAPI HOOKED_FreeLibrary(HMODULE hModule)
	{
		for (auto it = modules.cbegin(); it != modules.cend(); ++it)
		{
			if ((*it)->GetHandle() == hModule)
				(*it)->Unhook();
		}

		BOOL rv = ORIG_FreeLibrary(hModule);

		if (DebugEnabled())
			EngineDevMsg("Engine call: FreeLibrary( %p ) => %s\n", hModule, (rv ? "true" : "false"));

		return rv;
	}

	void InitInterception(bool needToIntercept)
	{
		ORIG_LoadLibraryA = LoadLibraryA;
		ORIG_LoadLibraryW = LoadLibraryW;
		ORIG_LoadLibraryExA = LoadLibraryExA;
		ORIG_LoadLibraryExW = LoadLibraryExW;
		ORIG_FreeLibrary = FreeLibrary;

		if (needToIntercept)
		{
			auto status = MH_Initialize();
			if (status != MH_OK)
			{
				EngineWarning("Failed to initialize MinHook: %s.\n", MH_StatusToString(status));
				return;
			}

			MemUtils::Intercept(L"WinAPI",
				ORIG_LoadLibraryA, HOOKED_LoadLibraryA,
				ORIG_LoadLibraryW, HOOKED_LoadLibraryW,
				ORIG_LoadLibraryExA, HOOKED_LoadLibraryExA,
				ORIG_LoadLibraryExW, HOOKED_LoadLibraryExW,
				ORIG_FreeLibrary, HOOKED_FreeLibrary);
		}
	}

	void ClearInterception(bool needToIntercept)
	{
		MemUtils::RemoveInterception(L"WinAPI",
			ORIG_LoadLibraryA,
			ORIG_LoadLibraryW,
			ORIG_LoadLibraryExA,
			ORIG_LoadLibraryExW,
			ORIG_FreeLibrary);

		auto status = MH_Uninitialize();
		if (status != MH_OK)
			EngineDevWarning("Failed to uninitialize MinHook: %s.\n", MH_StatusToString(status));
		
		ORIG_LoadLibraryA = nullptr;
		ORIG_LoadLibraryW = nullptr;
		ORIG_LoadLibraryExA = nullptr;
		ORIG_LoadLibraryExW = nullptr;
		ORIG_FreeLibrary = nullptr;
	}
}
