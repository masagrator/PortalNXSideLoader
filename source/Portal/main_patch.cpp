#if defined(PORTAL) && defined(PORTAL2)
	#error You cannot define PORTAL and PORTAL2 at the same time!
#endif

#if !defined(PORTAL) && !defined(PORTAL2)
	#error You must define if you are using PORTAL or PORTAL2!
#endif

#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/utils/cpputils.hpp"
#include "skyline/inlinehook/memcpy_controlled.hpp"
#include "nn/fs.h"
#include "nn/nifm.h"
#include <sys/stat.h>

//Enabling PORTAL_LOG makes loading game files drastically slower mainly because of stat_nx
#ifdef PORTAL_LOG
bool nx_lock = false;
bool stat_lock = false;
FILE* nx_log = 0;
FILE* stat_log = 0;
#endif

uintptr_t TextRegionOffset = 0;

ptrdiff_t returnInstructionOffset(uintptr_t LR) {
	return LR - TextRegionOffset;
}

extern "C" {
	int stat_nx(const char* pathname, struct stat* statbuf);
	FILE* fopen_nx ( const char * filename, const char * mode );
	#ifdef PORTAL2
	void* _ZN2nn3mem17StandardAllocator8AllocateEm(void* _this, size_t size);
	#endif
}

namespace nn { 

	namespace nifm {
		uint64_t (*Initialize_original)();
		uint64_t Initialize_hook() {
			return 0x27C;
		}
	}
}

char* strtolower(char* dest, const char* src, size_t len)
{
	if (!len)
		return 0;

	else {
		char* d = dest;
		while (*src && len-- > 0)
			*d++ = tolower(*src++);

		*d = 0;
		return dest;
	}
}

void formatPath (const char* path, char* filepath, bool NXCONTENT) {
	char temp[256] = "";
	if (!NXCONTENT) {
		if (!strncmp(path, "/", 1))
			sprintf(&temp[0], "rom:%s", path);
		else
			sprintf(&temp[0], "rom:/%s", path);
		strtolower(filepath, &temp[0], strlen(&temp[0]));
	}
	else {
		if (!strncmp(path, "/", 1))
			sprintf(&temp[0], "rom:/nxcontent%s", path);
		else
			sprintf(&temp[0], "rom:/nxcontent/%s", path);
		strtolower(filepath, &temp[0], strlen(&temp[0]));
	}
}

int (*stat_nx_original)(const char* pathname, struct stat* statbuf);
int stat_nx_hook(const char* pathname, struct stat* statbuf) {
	#ifdef PORTAL_LOG
	if (pathname) {
		while (stat_lock) 
			nn::os::SleepThread(nn::TimeSpan(1000000));
		stat_lock = true;
		stat_log = fopen("sdmc:/Portal_stat.txt", "a");
		if (stat_log) {
			fwrite((const void*)pathname, strlen(pathname), 1, stat_log);
			fwrite((const void*)&"\n", 1, 1, stat_log);
			fclose(stat_log);
		}
		stat_lock = false;
	}
	#endif
	int ret = stat_nx_original(pathname, statbuf);
	if (!ret)
		return ret;
	if (!strncmp(pathname, "/;", 2))
		return ret;
	char filepath[256] = "";
	formatPath(pathname, &filepath[0], true);
	struct stat temp_statbuf;
	int ret2 = stat(&filepath[0], &temp_statbuf);
	if (!ret2)
		return stat(&filepath[0], statbuf);
	formatPath(pathname, &filepath[0], false);
	ret2 = stat(&filepath[0], &temp_statbuf);
	if (!ret2)
		return stat(&filepath[0], statbuf);
	return ret;
}

struct fopen2Struct {
	uint64_t buffer_size;
	void* buffer;
};

void (*fopen2_original)(fopen2Struct* _struct, void* x1, const char* path);
void fopen2_hook(fopen2Struct* _struct, void* x1, const char* path){
	#ifdef PORTAL_LOG
	static bool log_init = false;
	if (!log_init) {
		nn::fs::MountSdCardForDebug("sdmc");
		nx_log = fopen("sdmc:/Portal.txt", "w");
		stat_log = fopen("sdmc:/Portal_stat.txt", "w");
		fclose(nx_log);
		fclose(stat_log);
		log_init = true;
	}
	#endif
	char filepath[256] = "";
	formatPath(path, &filepath[0], false);

	FILE* file = fopen(filepath, "r");
	if(!file)
		return fopen2_original(_struct, x1, path);

	int32_t size = 0;
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	#ifdef PORTAL
	void* new_buffer = malloc(size);
	#endif
	#ifdef PORTAL2
	void* Allocator = (void*)(TextRegionOffset+0x860078);
	void* new_buffer = _ZN2nn3mem17StandardAllocator8AllocateEm(Allocator, size);
	#endif

	fread(new_buffer, size, 1, file);
	fclose(file);

	_struct->buffer_size = size;
	_struct->buffer = new_buffer;
}

char filepath[256] = "";

FILE* (*fopen_nx_original)(const char* path, const char* mode);
FILE* fopen_nx_hook(const char* path, const char* mode) {
	#ifdef PORTAL_LOG
	if (path && mode) {
		while (nx_lock) 
			nn::os::SleepThread(nn::TimeSpan(1000000));
		nx_lock = true;
		nx_log = fopen("sdmc:/Portal.txt", "a");
		if (nx_log) {
			fwrite((const void*)path, strlen(path), 1, nx_log);
			fwrite((const void*)&" ", 1, 1, nx_log);
			fwrite((const void*)mode, strlen(mode), 1, nx_log);
			fwrite((const void*)&"\n", 1, 1, nx_log);
			fclose(nx_log);
		}
		nx_lock = false;
	}
	#endif
	if (strstr(mode, "w") || strstr(mode, "+") || strstr(mode, "a"))
		return fopen_nx_original(path, mode);

	nn::fs::FileHandle filehandle;
	formatPath(path, &filepath[0], true);

	if(R_FAILED(nn::fs::OpenFile(&filehandle, &filepath[0], nn::fs::OpenMode_Read))) {
		formatPath(path, &filepath[0], false);

		if(R_FAILED(nn::fs::OpenFile(&filehandle, &filepath[0], nn::fs::OpenMode_Read)))
			return fopen_nx_original(path, mode);
	}
	nn::fs::CloseFile(filehandle);
	return fopen(&filepath[0], mode);
}

void Portal_main()
{
	TextRegionOffset = (uintptr_t)skyline::utils::getRegionAddress(skyline::utils::region::Text);

	#ifdef PORTAL
	A64HookFunction((void*)(TextRegionOffset + 0x7D978), reinterpret_cast<void*>(fopen2_hook), (void**)&fopen2_original);
	#endif
	#ifdef PORTAL2
	A64HookFunction((void*)(TextRegionOffset + 0x182F50), reinterpret_cast<void*>(fopen2_hook), (void**)&fopen2_original);
	#endif


	A64HookFunction((void**)&fopen_nx, reinterpret_cast<void*>(fopen_nx_hook), (void**)&fopen_nx_original);
	A64HookFunction((void**)&stat_nx, reinterpret_cast<void*>(stat_nx_hook), (void**)&stat_nx_original);

	#ifdef PORTAL2
		#ifdef PDEBUG
	//Hook needed to run game with connected internet and blocked Nintendo servers

	A64HookFunction((void**)&nn::nifm::Initialize, reinterpret_cast<void*>(nn::nifm::Initialize_hook), (void**)&nn::nifm::Initialize_original);
		#endif
	#endif

}
