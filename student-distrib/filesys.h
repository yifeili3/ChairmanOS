 #ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"
#define FILE_NAME_SIZE 32
#define RESERVED_SIZE1 24
#define INODE_BLOCK_SIZE 1023
#define RESERVED_SIZE2 52
#define DIR_ENTRY_SIZE 63
/*
 *
 * 4KB block: Boot block, inode, data_block
 *  Boot Block  4KB  |  Inode       4KB  |  Dir Entry   64B  |
 * ----------------  | ----------------  | ----------------- |
 * | # entries | 4B  | |   length  | 4B  | | file name | 32B |
 * -------------     | -------------     | -------------     |
 * | # inodes  | 4B  | |0th dblock#| 4B  | | file type | 4B  |
 * -------------     | -------------     | -------------     |
 * | # d block | 4B  | |1st dblock#| 4B  | | inode #   | 4B  |
 * -------------     |       .           | -------------     |
 * | reserved  | 52B |       .           | | Reserved  | 24B |
 * -------------     |       .           | ----------------- |
 * | direntry0 | 64B | -------------     |
 * -------------     | |1023dblock#| 4B  |
 * | direntry1 | 64B | ----------------- |
 *     
 */  

/* file struct for directory entry*/
typedef struct __attribute__((packed)) {
	uint8_t file_name[FILE_NAME_SIZE];
	uint32_t file_type;
	uint32_t inode_idx;
	uint8_t reserved[RESERVED_SIZE1];
} dentry_t;

/* file struct for inode_t*/
typedef struct __attribute__((packed)) {
	uint32_t length;
	uint32_t dblock_idx[INODE_BLOCK_SIZE];
} inode_t;

/* file struct for boot_block_t*/
typedef struct __attribute__((packed)) {
	uint32_t num_entry;
	uint32_t num_inode;
	uint32_t num_dblock;
	uint8_t reserved[RESERVED_SIZE2];
	dentry_t dir_entry[DIR_ENTRY_SIZE];
} boot_block_t;

/* initialize local file system*/
void fs_init(uint32_t addr);

/* read directory entry by name */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

/* read directory entry by index */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/* read data in the file*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* Test case functions */
int32_t _print_directory();
int32_t _test_read_data();

/* file open syscall*/
int32_t file_open(const uint8_t* filename);

/* file close syscall*/
int32_t file_close(int32_t file_desc);

/* file read syscall*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

/* file write syscall*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

/* directory open syscall*/
int32_t dir_open(const uint8_t* dirname);

/* directory close syscall*/
int32_t dir_close(int32_t file_desc);

/* directory write syscall*/
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

/* directory read syscall*/
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

#endif
