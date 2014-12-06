#include "../sptlib-stdafx.hpp"

#include "../sptlib.hpp"
#include "../MemUtils.hpp"
#include "../Hooks.hpp"
#include <dlfcn.h>
#include <link.h>

typedef void* (*_dlopen)(const char* filename, int flag);
typedef int (*_dlclose)(void* handle);

_dlopen ORIG_dlopen;
_dlclose ORIG_dlclose;

namespace Hooks
{
	extern "C" void* dlopen(const char* filename, int flag)
	{
		if (!ORIG_dlopen)
			ORIG_dlopen = reinterpret_cast<_dlopen>(dlsym(RTLD_NEXT, "dlopen"));

		auto rv = ORIG_dlopen(filename, flag);
		EngineDevMsg("Engine call: dlopen( \"%s\", %d ) => %p\n", filename, flag, rv);

		if (rv && filename)
			HookModule(Convert(filename));

		return rv;
	}

	extern "C" int dlclose(void* handle)
	{
		if (!ORIG_dlclose)
			ORIG_dlclose = reinterpret_cast<_dlclose>(dlsym(RTLD_NEXT, "dlclose"));

		for (auto it = modules.cbegin(); it != modules.cend(); ++it)
			if ((*it)->GetHandle() == handle)
				(*it)->Unhook();

		auto rv = ORIG_dlclose(handle);
		EngineDevMsg("Engine call: dlclose( %p ) => %d\n", handle, rv);

		return rv;
	}

	// typedef int (__attribute__((cdecl)) *_Init) (void* pEnginefuncs, int iVersion);
	// static _Init ORIG_Initialize = nullptr;

	// int it(dl_phdr_info* i, size_t s, void* data)
	// {
	// 	printf("Name: %s\n", i->dlpi_name);
	// 	if (i->dlpi_name[0] && strstr(i->dlpi_name, "cl_dlls/client.so"))
	// 		for (size_t x = 0; x < i->dlpi_phnum; ++x)
	// 		{
	// 			printf("\tSegment type: %p; addr: %p; size: %p; flags: %p\n", i->dlpi_phdr[x].p_type, i->dlpi_phdr[x].p_vaddr + i->dlpi_addr, i->dlpi_phdr[x].p_memsz, i->dlpi_phdr[x].p_flags);
	// 		}

	// 	return 0;
	// }

	// extern "C" int __attribute__((cdecl)) Initialize(void* pEnginefuncs, int iVersion)
	// {
	// 	dl_iterate_phdr([](dl_phdr_info* i, size_t s, void* data) -> int {
	// 		EngineDevMsg("\tName: %s\n", i->dlpi_name);
	// 		return 0;
	// 	}, NULL);

	// 	for (auto h : MemUtils::GetLoadedModules())
	// 	{
	// 		void *handle = nullptr, *base = nullptr;
	// 		size_t size = 0;
	// 		MemUtils::GetModuleInfo(MemUtils::GetModulePath(h), &handle, &base, &size);
	// 		EngineDevMsg("\th: %p; Handle: %p; base: %p; size: %zx; path: %s\n", h, handle, base, size, Convert(MemUtils::GetModulePath(h)).c_str());
	// 	}

	// 	return 0;
	// }

	void InitInterception(bool needToIntercept)
	{
		if (!ORIG_dlopen)
			ORIG_dlopen = reinterpret_cast<_dlopen>(dlsym(RTLD_NEXT, "dlopen"));

		if (!ORIG_dlclose)
			ORIG_dlclose = reinterpret_cast<_dlclose>(dlsym(RTLD_NEXT, "dlclose"));
		
		if (needToIntercept)
			MemUtils::Intercept(L"POSIX", {
				{ reinterpret_cast<void**>(&ORIG_dlopen), reinterpret_cast<void*>(dlopen) },
				{ reinterpret_cast<void**>(&ORIG_dlclose), reinterpret_cast<void*>(dlclose) }
			});
	}

	void ClearInterception(bool needToIntercept)
	{
		if (needToIntercept)
			MemUtils::RemoveInterception(L"POSIX", {
				{ reinterpret_cast<void**>(&ORIG_dlopen), reinterpret_cast<void*>(dlopen) },
				{ reinterpret_cast<void**>(&ORIG_dlclose), reinterpret_cast<void*>(dlclose) }
			});

		ORIG_dlopen = nullptr;
		ORIG_dlclose = nullptr;
	}
}
