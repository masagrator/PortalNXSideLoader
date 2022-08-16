/**
 * @file romfs_dev.h
 * @brief RomFS driver.
 * @author yellows8
 * @author mtheall
 * @author fincs
 * @copyright libnx Authors
 */
#pragma once

#include "../../services/fs.h"
#include "../../services/ncm_types.h"
#include "../../types.h"

/// RomFS header.
typedef struct {
    u64 headerSize;         ///< Size of the header.
    u64 dirHashTableOff;    ///< Offset of the directory hash table.
    u64 dirHashTableSize;   ///< Size of the directory hash table.
    u64 dirTableOff;        ///< Offset of the directory table.
    u64 dirTableSize;       ///< Size of the directory table.
    u64 fileHashTableOff;   ///< Offset of the file hash table.
    u64 fileHashTableSize;  ///< Size of the file hash table.
    u64 fileTableOff;       ///< Offset of the file table.
    u64 fileTableSize;      ///< Size of the file table.
    u64 fileDataOff;        ///< Offset of the file data.
} romfs_header;

/// RomFS directory.
typedef struct {
    u32 parent;      ///< Offset of the parent directory.
    u32 sibling;     ///< Offset of the next sibling directory.
    u32 childDir;    ///< Offset of the first child directory.
    u32 childFile;   ///< Offset of the first file.
    u32 nextHash;    ///< Directory hash table pointer.
    u32 nameLen;     ///< Name length.
    uint8_t name[];  ///< Name. (UTF-8)
} romfs_dir;

/// RomFS file.
typedef struct {
    u32 parent;      ///< Offset of the parent directory.
    u32 sibling;     ///< Offset of the next sibling file.
    u64 dataOff;     ///< Offset of the file's data.
    u64 dataSize;    ///< Length of the file's data.
    u32 nextHash;    ///< File hash table pointer.
    u32 nameLen;     ///< Name length.
    uint8_t name[];  ///< Name. (UTF-8)
} romfs_file;

/**
 * @brief Mounts the Application's RomFS.
 * @param name Device mount name.
 * @remark This function is intended to be used to access one's own RomFS.
 *         If the application is running as NRO, it mounts the embedded RomFS section inside the NRO.
 *         If on the other hand it's an NSO, it behaves identically to \ref romfsMountFromCurrentProcess.
 */
Result romfsMountSelf(const char* name);

/**
 * @brief Mounts RomFS from an open file.
 * @param file FsFile of the RomFS image.
 * @param offset Offset of the RomFS within the file.
 * @param name Device mount name.
 */
Result romfsMountFromFile(FsFile file, u64 offset, const char* name);

/**
 * @brief Mounts RomFS from an open storage.
 * @param storage FsStorage of the RomFS image.
 * @param offset Offset of the RomFS within the storage.
 * @param name Device mount name.
 */
Result romfsMountFromStorage(FsStorage storage, u64 offset, const char* name);

/**
 * @brief Mounts RomFS using the current process host program RomFS.
 * @param name Device mount name.
 */
Result romfsMountFromCurrentProcess(const char* name);

/**
 * @brief Mounts RomFS from a file path in a mounted fsdev device.
 * @param path File path.
 * @param offset Offset of the RomFS within the file.
 * @param name Device mount name.
 */
Result romfsMountFromFsdev(const char* path, u64 offset, const char* name);

/**
 * @brief Mounts RomFS from SystemData.
 * @param dataId SystemDataId to mount.
 * @param storageId Storage ID to mount from.
 * @param name Device mount name.
 */
Result romfsMountFromDataArchive(u64 dataId, NcmStorageId storageId, const char* name);

/// Unmounts the RomFS device.
Result romfsUnmount(const char* name);

/// Wrapper for \ref romfsMountSelf with the default "romfs" device name.
static inline Result romfsInit(void) { return romfsMountSelf("romfs"); }

/// Wrapper for \ref romfsUnmount with the default "romfs" device name.
static inline Result romfsExit(void) { return romfsUnmount("romfs"); }
