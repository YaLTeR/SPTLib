#include "../sptlib-stdafx.hpp"

#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <linux/limits.h>
#include <unistd.h>
#include "../sptlib.hpp"
#include "../MemUtils.hpp"

typedef void* (*_dlopen)(const char* filename, int flag);
typedef int (*_dlclose)(void* handle);

extern _dlopen ORIG_dlopen;
extern _dlclose ORIG_dlclose;

namespace MemUtils
{
	namespace detail
	{
		void Intercept(const std::wstring& moduleName, size_t n, const std::pair<void**, void*> funcPairs[])
		{
			// Not implemented.
		}

		void RemoveInterception(const std::wstring& moduleName, size_t n, void** const functions[])
		{
			// Not implemented.
		}
	}

	void MarkAsExecutable(void* addr)
	{
		// Not implemented.
	}

	void ReplaceBytes(void* addr, size_t length, const uint8_t* newBytes)
	{
		static auto pagesize = sysconf(_SC_PAGESIZE);
		size_t protectLength = length + (reinterpret_cast<uintptr_t>(addr) % pagesize);
		void *protectAddr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(addr) - (reinterpret_cast<uintptr_t>(addr) % pagesize));
		if (mprotect(protectAddr, protectLength, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
			EngineDevWarning("Failed to protect memory: %s\n", strerror(errno));

		for (size_t i = 0; i < length; ++i)
			*(reinterpret_cast<uint8_t*>(addr)+i) = newBytes[i];

		// TODO: Restore original protect.
	}

	bool GetModuleInfo(void* moduleHandle, void** moduleBase, size_t* moduleSize)
	{
		auto path = Convert(GetModulePath(moduleHandle));
		if (!path.size())
			return false;

		struct wrapper {
			wrapper(FILE* f) : file(f) {};
			~wrapper() {
				if (file)
					fclose(file);
			}
			operator FILE*() const
			{
				return file;
			}

			FILE* file;
		} mapsFile(fopen("/proc/self/maps", "r"));

		if (!mapsFile)
		{
			EngineDevWarning("Could not open /proc/self/maps: %s\n", strerror(errno));
			return false;
		}

		void *base = nullptr;
		size_t size = 0;
		bool found = false;
		char buf[PATH_MAX + 1024];
		while (!feof(mapsFile))
		{
			void *start, *end;
			char filename[PATH_MAX + 1];
			
			if (fgets(buf, sizeof(buf), mapsFile) == nullptr)
			{
				EngineDevWarning("Could not read /proc/self/maps.\n");
				return false;
			}

			if (sscanf(buf, "%p-%p %*s %*s %*s %*s %s", &start, &end, filename) == 3)
			{
				if (!strcmp(filename, path.c_str()))
				{
					if (!found)
						base = start;

					found = true;
					size = (reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(base));

					// EngineDevMsg("\t\tBuf: %s", buf);
				}
				else if (found)
					break;
			}
			else if (found)
				break;
		}

		if (!found)
			return false;

		if (moduleBase)
			*moduleBase = base;

		if (moduleSize)
			*moduleSize = size;

		return true;
	}

	bool GetModuleInfo(const std::wstring& moduleName, void** moduleHandle, void** moduleBase, size_t* moduleSize)
	{
		auto fileName = GetFileName(moduleName);
		std::pair<void*, const std::wstring*> p = { nullptr, &fileName };
		dl_iterate_phdr([](dl_phdr_info* i, size_t s, void* data) -> int {
			if (i->dlpi_name[0])
			{
				auto handle = ORIG_dlopen(i->dlpi_name, RTLD_LAZY | RTLD_NOLOAD);
				ORIG_dlclose(handle);

				auto p = reinterpret_cast<std::pair<void*, const std::wstring*>*>(data);
				if (!p->second->compare(GetFileName(Convert(std::string(i->dlpi_name)))))
					p->first = handle;
			}

			return 0;
		}, &p);

		if (!p.first)
			return false;

		if (moduleHandle)
			*moduleHandle = p.first;

		GetModuleInfo(p.first, moduleBase, moduleSize);

		return true;
	}

	std::wstring GetModulePath(void* moduleHandle)
	{
		std::pair<void*, std::string> p = { moduleHandle, std::string() };
		dl_iterate_phdr([](dl_phdr_info* i, size_t s, void* data) -> int {
			if (i->dlpi_name[0])
			{
				auto handle = ORIG_dlopen(i->dlpi_name, RTLD_LAZY | RTLD_NOLOAD);
				ORIG_dlclose(handle);

				auto p = reinterpret_cast<std::pair<void*, std::string>*>(data);
				if (handle == p->first)
					p->second.assign(i->dlpi_name);
			}

			return 0;
		}, &p);

		return NormalizePath(Convert(p.second));
	}

	std::vector<void*> GetLoadedModules()
	{
		std::vector<void*> out;

		dl_iterate_phdr([](dl_phdr_info* i, size_t s, void* data) -> int {
			if (i->dlpi_name[0])
			{
				auto handle = ORIG_dlopen(i->dlpi_name, RTLD_LAZY | RTLD_NOLOAD);
				ORIG_dlclose(handle);
				reinterpret_cast<std::vector<void*>*>(data)->push_back(handle);
			}

			return 0;
		}, &out);

		return out;
	}

	void* GetSymbolAddress(void* moduleHandle, const char* functionName)
	{
		return dlsym(moduleHandle, functionName);
	}
}
