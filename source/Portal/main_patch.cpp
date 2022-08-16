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
#include "nn/account.h"
#include "nn/nifm.h"
#include <string>
#include <algorithm>

uintptr_t TextRegionOffset = 0;

ptrdiff_t returnInstructionOffset(uintptr_t LR) {
	return LR - TextRegionOffset;
}

extern "C" {
	FILE* fopen ( const char * filename, const char * mode );
	FILE* fopen_nx ( const char * filename, const char * mode );
	int fseek( FILE * stream, long offset, int origin );
	size_t fread( void * buffer, size_t size, size_t count, FILE * stream );
	int fclose( FILE * stream );
	long ftell( FILE * stream );
	#ifdef PORTAL2
	void* _ZN2nn3mem17StandardAllocator8AllocateEm(void* _this, size_t size);
	#endif
}

namespace nn { 
	
	namespace account {

		uint64_t (*EnsureNetworkServiceAccountAvailable_original)(nn::account::UserHandle const& userHandle);
		uint64_t EnsureNetworkServiceAccountAvailable_hook (nn::account::UserHandle const& userHandle) {
			if (R_FAILED(EnsureNetworkServiceAccountAvailable_original(userHandle)))
				return 0x27C;
			return 0;
		}
	}

	namespace nifm {
		uint64_t (*IsNetworkAvailable_original)();
		uint64_t IsNetworkAvailable_hook() {
			return 0;
		}
	}
}

struct fopen2Struct {
	uint64_t buffer_size;
	void* buffer;
};

bool formatPath (const char* path, char* filepath) {
	if (!strncmp(path, "/", 1))
		snprintf(&filepath[0], 255, "rom:%s", path);
	else if (!strncmp(path, "nxcontent", 9))
		snprintf(&filepath[0], 255, "rom:/%s", path);
	else if (!strncmp(path, "platform", 8))
		snprintf(&filepath[0], 255, "rom:/%s", path);
	#ifdef PORTAL
	else if (!strncmp(path, "hl2", 3))
		snprintf(&filepath[0], 255, "rom:/%s", path);
	else if (!strncmp(path, "portal", 6))
		snprintf(&filepath[0], 255, "rom:/%s", path);
	#endif
	#ifdef PORTAL2
	else if (!strncmp(path, "update", 6))
		snprintf(&filepath[0], 255, "rom:/%s", path);
	else if (!strncmp(path, "portal2_dlc1", 12))
		snprintf(&filepath[0], 255, "rom:/%s", path);
	else if (!strncmp(path, "portal2_dlc2", 12))
		snprintf(&filepath[0], 255, "rom:/%s", path);
	else if (!strncmp(path, "portal2", 7))
		snprintf(&filepath[0], 255, "rom:/%s", path);
	#endif
	else 
		return false;
	return true;
}

void (*fopen2_original)(fopen2Struct* _struct, void* x1, const char* path);
void fopen2_hook(fopen2Struct* _struct, void* x1, const char* path){
	char filepath[256] = "";
	bool formatted = formatPath(path, &filepath[0]);
	if (!formatted)
		return fopen2_original(_struct, x1, path);

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
	void* Allocator = (void*)(TextRegionOffset+0x861078);
	void* new_buffer = _ZN2nn3mem17StandardAllocator8AllocateEm(Allocator, size);
	#endif

	fread(new_buffer, size, 1, file);
	fclose(file);

	_struct->buffer_size = size;
	_struct->buffer = new_buffer;
}

FILE* (*fopen_nx_original)(const char* path, const char* mode);
FILE* fopen_nx_hook(const char* path, const char* mode) {
	nn::fs::FileHandle filehandle;
	char filepath[256] = "";
	bool formatted = formatPath(path, &filepath[0]);
	if (!formatted)
		return fopen_nx_original(path, mode);

	if(R_FAILED(nn::fs::OpenFile(&filehandle, filepath, nn::fs::OpenMode_Read)))
		return fopen_nx_original(path, mode);
	nn::fs::CloseFile(filehandle);
	return fopen(filepath, mode);
}

void SPRB_main()
{
	TextRegionOffset = (uintptr_t)skyline::utils::getRegionAddress(skyline::utils::region::Text);

	#ifdef PORTAL
	A64HookFunction((void*)(TextRegionOffset + 0x7D978), reinterpret_cast<void*>(fopen2_hook), (void**)&fopen2_original);
	#endif
	#ifdef PORTAL2
	A64HookFunction((void*)(TextRegionOffset + 0x182F50), reinterpret_cast<void*>(fopen2_hook), (void**)&fopen2_original);
	#endif


	A64HookFunction((void**)&fopen_nx, reinterpret_cast<void*>(fopen_nx_hook), (void**)&fopen_nx_original);

	//Hooks needed to run game with connected internet and blocked Nintendo servers
	A64HookFunction((void**)&nn::account::EnsureNetworkServiceAccountAvailable, reinterpret_cast<void*>(nn::account::EnsureNetworkServiceAccountAvailable_hook), (void**)&nn::account::EnsureNetworkServiceAccountAvailable_original);

	A64HookFunction((void**)&nn::nifm::IsNetworkAvailable, reinterpret_cast<void*>(nn::nifm::IsNetworkAvailable_hook), (void**)&nn::nifm::IsNetworkAvailable_original);

}
