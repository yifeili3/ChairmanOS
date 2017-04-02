#include "filesys.h"
#include "lib.h"
#include "keyboard.h"
#include "syscall.h"
#define BLOCKSIZE 	4096
#define MAXCHAR		32
#define NAMESIZE	100
#define DATASIZE	6000
#define SPACE 		0x20
/* -------------------------Local func and struct---------------------- */
/* Function for finding available file descriptor in pcb */
// static int32_t _find_avail_fd() {
// 	int i;
// 	for (i = 0; i < 8; i++) 
// 		if (pcb->file_array[i].flags = 0)
// 			return i;
// 	return -1;
// }

/* Local struct of filesystem */
static uint32_t _fs_base;
static uint32_t _inode_base;
static uint32_t _num_inode;
static uint32_t _data_base;
static uint32_t _num_dentry;
static uint32_t _num_dblock;
static boot_block_t* _boot_block_ptr;
 
/* -------------------------5.2.2 functions---------------------------- */
/*
 * 	fs_init
 * DESCRIPTION: Initialize local file system variables to decribe the cur-
 *				rent FS. 
 * INPUTS: @addr, address of boot block
 * OUTPUTS: none
 * RETURN VALUES: none
 * SIDE EFFECTS: modify local file system variables
 */
void fs_init(uint32_t addr) {
	_fs_base = addr;
	_boot_block_ptr = (boot_block_t *)addr;
	_num_dentry = _boot_block_ptr->num_entry;
	_inode_base = addr + BLOCKSIZE;
	_num_inode = _boot_block_ptr->num_inode;
	_data_base = _inode_base + _num_inode * BLOCKSIZE;
	_num_dblock = _boot_block_ptr->num_dblock;
}

/*
 * 	read_dentry_by_name
 * DESCRIPTION: read the specific dentry in boot block by name reference
 * INPUTS: @fname, filename to be read; @dentry, ptr to read into
 * OUTPUS: a duplicate dentry pointed to by @dentry
 * RETURN VALUES: 0 on success, -1 on failure
 * SIDE EFFECTS: none
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
	int i;
	/* traverse through all dentries to locate fname */
	for (i = 0; i < _num_dentry; i++) {
		/* if fname is found, copy info into dentry passed in */
		if (strncmp((int8_t*)fname, (int8_t*)((_boot_block_ptr)->dir_entry[i]).file_name, MAXCHAR-1) == 0) {
			strcpy((int8_t*)dentry->file_name, (int8_t*)fname);
			dentry->file_type = ((_boot_block_ptr)->dir_entry[i]).file_type;
			dentry->inode_idx = ((_boot_block_ptr)->dir_entry[i]).inode_idx;
			return 0;
		}
	}
	/* not found, return failure */
	return -1;
}
/*
 * 	read_dentry_by_index
 * DESCRIPTION: read the specific dentry in boot block by index reference
 * INPUTS: @index, the ith dentry to be read; @dentry, ptr to read into
 * OUTPUS: a duplicate dentry pointed to by @dentry
 * RETURN VALUES: 0 on success, -1 on failure
 * SIDE EFFECTS: none
 */
 int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
 	/* Bound range check */
	if (index >= _num_dentry)
		return -1;
	/* Within range, copy info */
	strcpy((int8_t*)dentry->file_name, (int8_t*)(_boot_block_ptr->dir_entry[index]).file_name);
	dentry->file_type = (_boot_block_ptr->dir_entry[index]).file_type;
	dentry->inode_idx = (_boot_block_ptr->dir_entry[index]).inode_idx;
	return 0;
}

/*
 *	read_data
 * DESCRIPTION: read data from inode
 * INPUTS: @inode, the ith inode that data blocks reside in
 *		   @offset, the starting byte to read within inode
 *		   @buf, address to read data into
 * 		   @length, total number of bytes to read
 * OUTPUS: none
 * RETURN VALUES: bytes read if success or -1 if failure
 * SIDE EFFECTS: none;
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf_i, uint32_t length) {
	inode_t* inode_ptr = (inode_t*)(_inode_base + BLOCKSIZE * inode);	/* Pointer to inode being read */
	uint32_t block_index;		/* Index of data block to be read */
	uint32_t length_reading;	/* # bytes to read within current data block */
	uint32_t byte_read = 0;		/* Total # bytes read so far */
	uint32_t read_start;		/* Actual address of reading */
	uint8_t* buf = buf_i;
	//printf("File size: %d\n", inode_ptr->length);
	/* Bound range check */
	if (inode >= _num_inode) 	
		return -1;
	if (offset >= inode_ptr->length) 	
		return 0;	// changed to 0, so to end when the file read ends
	/* If there is no enough bytes in this inode, read all it has */
	if (length + offset >= inode_ptr->length)	length = inode_ptr->length - offset;
	/* If more to read, continue reading process */
	while(length > 0) {
		/* set block index of this cycle */
		block_index = inode_ptr->dblock_idx[offset/BLOCKSIZE]; 
		/* Bound range check */
		if (block_index >= _num_dblock)	
			return -1;
		/* set reading address from block index */
		read_start = BLOCKSIZE * block_index + _data_base + offset % BLOCKSIZE;
		/* set bytes to read from this block */
		length_reading = BLOCKSIZE - offset % BLOCKSIZE;
		if (offset%BLOCKSIZE + length < BLOCKSIZE)	length_reading = length;
		memcpy(buf, (int8_t*)read_start, length_reading);
		/* Update conditions for next cycle */
		offset += length_reading; 	
		byte_read += length_reading;
		length -= length_reading;
		buf = buf + length_reading;
	}
	return byte_read;
}

/* ------------------------Test case---------------------------*/
/*
 * _print_directory
 *	DESCRIPTION: Print all file names in current directory
 *	INPUTS: none
 *	OUTPUTS: 0 on success, -1 on failure
 */
int32_t _print_directory() {
	dentry_t temp0;
	uint8_t space[MAXCHAR];
	uint32_t i;
	/* space chars for align */
	for (i = 0; i < MAXCHAR; i++) space[i] = SPACE;
	printf("Number of entries: %d\n", _num_dentry);
	printf("Number of inodes: %d\n", _num_inode);
	printf("Number of data blocks: %d\n", _num_dblock);
	printf("List of files in current directory: \n");
	printf("Name                            | FileType | Inode# \n");
	for (i = 0; i < _num_dentry; i++) {
		read_dentry_by_index(i, &temp0);
		terminal_write(0, temp0.file_name, NAMESIZE);
		terminal_write(0, space, MAXCHAR - strlen((int8_t*)temp0.file_name));
		printf("|    %d     |", temp0.file_type);
		printf("   %d\n", temp0.inode_idx);
	}
	return 0;
}

/*
 * _test_read_data()
 *	DESCRIPTION: Test read data from a file
 *	INPUTS: none
 *	OUTPUTS: Print data on terminal
 *	RETURN VALUES: 0 on success, -1 on failure
 */
int32_t _test_read_data() {
	dentry_t temp;
	uint8_t buf[DATASIZE];
	int bytesW;
	if (read_dentry_by_name((uint8_t*)"verylargetxtwithverylongname.txt", &temp) == -1)
		return -1;
	printf("Read data test case: ");
	//puts((int8_t*)temp.file_name);
	terminal_write(0, temp.file_name, NAMESIZE);
	printf("\n");
	printf("Number of bytes by read_data: %d\n", (bytesW = read_data(temp.inode_idx, 0, buf, DATASIZE)));
	
	int i;
	for (i = 0; i < bytesW; i++) {
		//putc(buf[i]);
	}
	/* Print hex */
	// for (i = 0; i < 1000; i++) {
	// 	printf("0x%x ", buf[i]);
	// }
	/* Print with terminal write */
	//write(0, buf, bytesW);
	printf("\nNumber of bytes: %d\n", bytesW);
	// if(read_data(inode_addr, 0, buf, 20) > 0)
	// 	puts(buf);
	// else
	// 	printf("failed\n");
	return 0;
}

/* -------------------------Extra Functions--------------------------- */
/*
 * file_open
 *	DESCRIPTION: opening file by inserting into file array of pcb
 *	INPUTS: @filename, file to be opened
 *	OUTPUTS: none
 * 	RETURN VALUES: 0 on success, -1 on failure
 */
int32_t file_open(const uint8_t* filename) {
	// file name validity check
	if (filename[0] =='\0'|| filename[0] ==' ') {
		return -1;
	}
	/* Check if file exists */
	dentry_t temp;
	if (read_dentry_by_name(filename,&temp)==-1){
		//puts("File does not exist.\n");
		return -1;
	}
	return 0;
}

/*
 * file_close
 *	DESCRIPTION: Called to close a file with given fd
 *	INPUTS: @fd, file descriptor
 * 	OUTPUTS: none
 * 	RETURN VALUES: 0 on success, -1 on failure
 */
int32_t file_close(int32_t fd) {
	return 0;
}

/*
 * file_write
 *	DESCRIPTION: Called to write content in buf to a file with given fd
 *	INPUTS: @fd, file descriptor; @buf, content buffer;
 *			@nbytes, #bytes to be written
 * 	OUTPUTS: none
 * 	RETURN VALUES: # of bytes written on success, -1 on failure
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
	return -1;
}

/*
 * file_read
 *	DESCRIPTION: called to read file into a buffer with given fd
 *	INPUTS: @fd, file descriptor; @buf, content buffer; @nbytes, # to be read
 * 	OUTPUTS: none
 *	RETURN VAlUES: #of bytes read on success or -1 on failure
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
	// get the current pcb
	pcb_t* cur_proc = (pcb_t*)(EIGHTMB - EIGHTKB*(current_pid + 1));

	//sanity check for fd
	if (fd <= 1 || fd >= FDSIZE)
		return -1;

	// call read data function to read the data in file
	int32_t bytesR;
	if ((bytesR = read_data(cur_proc->file_array[fd].inode, cur_proc->file_array[fd].file_pos, (uint8_t*)buf, nbytes))==-1)
		return -1;

	// update file position
	cur_proc->file_array[fd].file_pos += bytesR;
	return bytesR;
}

/*
 * dir_open
 *	DESCRIPTION: opening directory by inserting into file array of pcb
 *	INPUTS: @dirname, directory to be opened
 *	OUTPUTS: none
 * 	RETURN VALUES: 0 on success, -1 on failure
 */
int32_t dir_open(const uint8_t* dirname) {
	return 0;
}
/*
 * dir_close
 *	DESCRIPTION: Called to close a directory with given fd
 *	INPUTS: @fd, file descriptor
 * 	OUTPUTS: none
 * 	RETURN VALUES: 0 on success, -1 on failure
 */
int32_t dir_close(int32_t file_desc) {
	return 0;
}
/*
 * dir_write
 *	DESCRIPTION: Called to write content in buf to a directory with given fd
 *	INPUTS: @fd, file descriptor; @buf, content buffer;
 *			@nbytes, #bytes to be written
 * 	OUTPUTS: none
 * 	RETURN VALUES: # of bytes written on success, -1 on failure
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
	return -1;
}

/*
 * dir_read
 *	DESCRIPTION: called to read directory into a buffer with given fd
 *	INPUTS: @fd, file descriptor; @buf, content buffer; @nbytes, # to be read
 * 	OUTPUTS: none
 *	RETURN VAlUES: #of bytes read on success or -1 on failure
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
	/* get the current pcb */
	pcb_t* cur_proc = (pcb_t*)(EIGHTMB - EIGHTKB * (current_pid + 1));

	/* sanity check for valid fd */
	if (fd <= STATICFD || fd >= FDSIZE)
		return -1;

	/* Get corresponding idx and check range */
	uint32_t dentry_idx = (cur_proc->file_array[fd].file_pos);
	
	/* Increment to next file and read file name into buf */
	cur_proc->file_array[fd].file_pos++;
	uint8_t* fname = (uint8_t*)((_boot_block_ptr)->dir_entry[dentry_idx]).file_name;
	strncpy((int8_t*)buf, (int8_t*)fname, nbytes);

	/* File index range check */
	if (dentry_idx >= _num_dentry) {
		cur_proc->file_array[fd].file_pos = 0;
		return 0;
	}

	return strlen((int8_t*)fname);
}
