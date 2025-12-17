/**
 * @file    include/ultraos_fs.h
 * @brief   File system header definitions for UltraOS
 * @author  zeroday
 * @version 2.0.0
 * @date    2025-12-17
 * 
 * UltraOS - Advanced Multi-Architecture Operating System
 * Copyright (c) 2025 zeroday. All rights reserved.
 */

#ifndef ULTRAOS_FS_H
#define ULTRAOS_FS_H

#include <ultraos_types.h>

// File system constants
#define MAX_MOUNT_POINTS       32
#define MAX_PATH_LENGTH        4096
#define MAX_FILE_NAME_LENGTH   256
#define INODE_CACHE_SIZE       1024
#define FILE_CACHE_SIZE        512

// File system types
typedef enum {
    FS_TYPE_NONE = 0,
    FS_TYPE_TMPFS,          // Temporary file system
    FS_TYPE_DEVFS,          // Device file system
    FS_TYPE_PROCFS,         // Process file system
    FS_TYPE_EXT2,           // Extended file system 2
    FS_TYPE_EXT3,           // Extended file system 3
    FS_TYPE_EXT4,           // Extended file system 4
    FS_TYPE_FAT32,          // FAT32 file system
    FS_TYPE_NTFS,           // NTFS file system
    FS_TYPE_ISO9660,        // ISO9660 CD-ROM file system
    FS_TYPE_MAX
} fs_type_t;

// File open flags
#define O_RDONLY              0x00000000
#define O_WRONLY              0x00000001
#define O_RDWR                0x00000002
#define O_APPEND              0x00000008
#define O_CREAT               0x00000040
#define O_TRUNC               0x00000200
#define O_EXCL                0x00000800
#define O_NOCTTY              0x00000080
#define O_NONBLOCK            0x00000800
#define O_SYNC                0x00001000
#define O_DIRECT              0x00004000
#define O_NOFOLLOW            0x00020000
#define O_NOATIME             0x00040000
#define O_PATH                0x01000000
#define O_TMPFILE             0x00400000

// File mode bits
#define S_IRUSR               0x00000100
#define S_IWUSR               0x00000080
#define S_IXUSR               0x00000040
#define S_IRGRP               0x00000020
#define S_IWGRP               0x00000010
#define S_IXGRP               0x00000008
#define S_IROTH               0x00000004
#define S_IWOTH               0x00000002
#define S_IXOTH               0x00000001

#define S_ISUID               0x00000800
#define S_ISGID               0x00000400
#define S_ISVTX               0x00000200

// File types
#define S_IFMT                0xF0000000
#define S_IFREG               0x80000000
#define S_IFDIR               0x40000000
#define S_IFLNK               0xA0000000
#define S_IFBLK               0x60000000
#define S_IFCHR               0x20000000
#define S_IFIFO               0x10000000
#define S_IFSOCK              0xC0000000

// File seek modes
#define SEEK_SET              0
#define SEEK_CUR              1
#define SEEK_END              2

// File system statistics structure
typedef struct file_stat {
    uint64_t st_dev;         // Device ID
    uint64_t st_ino;         // Inode number
    uint32_t st_mode;        // File mode
    uint32_t st_nlink;       // Number of hard links
    uint32_t st_uid;         // User ID of owner
    uint32_t st_gid;         // Group ID of owner
    uint64_t st_rdev;        // Device ID (if special file)
    uint64_t st_size;        // Total size in bytes
    uint64_t st_blksize;     // Block size for I/O
    uint64_t st_blocks;      // Number of 512-byte blocks
    uint64_t st_atime;       // Time of last access
    uint64_t st_mtime;       // Time of last modification
    uint64_t st_ctime;       // Time of last status change
    uint64_t st_atime_ns;    // Nanoseconds component of atime
    uint64_t st_mtime_ns;    // Nanoseconds component of mtime
    uint64_t st_ctime_ns;    // Nanoseconds component of ctime
} file_stat_t;

// Directory entry structure
typedef struct dir_entry {
    char d_name[MAX_FILE_NAME_LENGTH];  // Directory entry name
    uint64_t d_ino;                     // Inode number
    uint32_t d_type;                    // File type
    uint32_t d_reclen;                  // Record length
} dir_entry_t;

// File structure
typedef struct file {
    char *path;                 // File path
    uint32_t flags;             // File flags
    uint32_t mode;              // File mode
    uint64_t position;          // Current file position
    uint64_t size;              // File size
    void *priv_data;            // Private file system data
    struct mount_point *mount_point;  // Mount point
    uint32_t ref_count;         // Reference count
} file_t;

// Mount point structure
typedef struct mount_point {
    const char *device;         // Device name
    const char *mountpoint;     // Mount point path
    fs_type_t type;             // File system type
    uint32_t flags;             // Mount flags
    bool is_mounted;            // Is mounted
    void *priv_data;            // Private mount data
} mount_point_t;

// File system type structure
typedef struct fs_type {
    fs_type_t type;             // File system type
    const char *name;           // File system name
    status_t (*mount)(mount_point_t *mp);
    status_t (*unmount)(mount_point_t *mp);
} fs_type_t;

// Function pointer types for file operations
typedef status_t (*fs_mount_func_t)(mount_point_t *mp);
typedef status_t (*fs_unmount_func_t)(mount_point_t *mp);

// Inode cache entry structure
typedef struct inode_cache_entry {
    uint64_t ino;               // Inode number
    void *inode;                // Inode data
    uint32_t ref_count;         // Reference count
    bool is_dirty;              // Is modified
    uint64_t access_time;       // Last access time
    list_head_t list;           // LRU list entry
    list_head_t hash;           // Hash table entry
} inode_cache_entry_t;

// Inode cache structure
typedef struct inode_cache {
    uint32_t size;              // Cache size
    uint32_t hit_count;         // Cache hits
    uint32_t miss_count;        // Cache misses
    list_head_t lru_list;       // LRU list
    list_head_t hash_table[INODE_CACHE_SIZE];  // Hash table
} inode_cache_t;

// File cache entry structure
typedef struct file_cache_entry {
    uint64_t ino;               // Inode number
    void *data;                 // File data
    uint32_t size;              // Data size
    uint32_t ref_count;         // Reference count
    bool is_dirty;              // Is modified
    uint64_t access_time;       // Last access time
    list_head_t list;           // LRU list entry
} file_cache_entry_t;

// File cache structure
typedef struct file_cache {
    uint32_t size;              // Cache size
    uint32_t hit_count;         // Cache hits
    uint32_t miss_count;        // Cache misses
    list_head_t lru_list;       // LRU list
} file_cache_t;

// Function prototypes

// Initialization
status_t fs_init(void);

// File system registration
status_t fs_register_type(fs_type_t type, const char *name,
                          fs_mount_func_t mount_func, fs_unmount_func_t unmount_func);

// Mount/unmount operations
status_t fs_mount(const char *device, const char *mountpoint, fs_type_t type, uint32_t flags);
status_t fs_unmount(const char *mountpoint);
mount_point_t* fs_find_mount_point(const char *path);
status_t fs_list_mounts(void);

// File operations
file_t* fs_open(const char *path, uint32_t flags, uint16_t mode);
status_t fs_close(file_t *file);
int64_t fs_read(file_t *file, void *buf, uint64_t count);
int64_t fs_write(file_t *file, const void *buf, uint64_t count);
int64_t fs_lseek(file_t *file, int64_t offset, int32_t whence);

// File/directory operations
status_t fs_stat(const char *path, file_stat_t *stat_buf);
status_t fs_mkdir(const char *path, uint16_t mode);
status_t fs_rmdir(const char *path);
status_t fs_unlink(const char *path);
status_t fs_rename(const char *old_path, const char *new_path);
int64_t fs_readdir(const char *path, dir_entry_t *entries, uint32_t max_entries);

// Cache management
void fs_init_inode_cache(void);
void fs_init_file_cache(void);

// Built-in file systems
status_t fs_register_builtin_fs(void);
status_t fs_mount_root(void);

// File system type specific functions (internal)
status_t tmpfs_mount(mount_point_t *mp);
status_t tmpfs_unmount(mount_point_t *mp);
status_t devfs_mount(mount_point_t *mp);
status_t devfs_unmount(mount_point_t *mp);
status_t procfs_mount(mount_point_t *mp);
status_t procfs_unmount(mount_point_t *mp);

// Utility macros
#define FS_ISREG(mode)     (((mode) & S_IFMT) == S_IFREG)
#define FS_ISDIR(mode)     (((mode) & S_IFMT) == S_IFDIR)
#define FS_ISLNK(mode)     (((mode) & S_IFMT) == S_IFLNK)
#define FS_ISBLK(mode)     (((mode) & S_IFMT) == S_IFBLK)
#define FS_ISCHR(mode)     (((mode) & S_IFMT) == S_IFCHR)
#define FS_ISFIFO(mode)    (((mode) & S_IFMT) == S_IFIFO)
#define FS_ISSOCK(mode)    (((mode) & S_IFMT) == S_IFSOCK)

#define FS_ACCESS(mode, access) (((mode) & (access)) != 0)
#define FS_READABLE(mode)  FS_ACCESS(mode, S_IRUSR | S_IRGRP | S_IROTH)
#define FS_WRITABLE(mode)  FS_ACCESS(mode, S_IWUSR | S_IWGRP | S_IWOTH)
#define FS_EXECUTABLE(mode) FS_ACCESS(mode, S_IXUSR | S_IXGRP | S_IXOTH)

#endif // ULTRAOS_FS_H