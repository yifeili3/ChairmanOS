typedef struct __attribute__((packed)) {
	syscall_jump_table* file_op_ptr;
	uint32_t inode_ptr;
	uint32_t file_pos;
	uint32_t flags;
} file_element_t;

typedef struct __attribute__((packed)) {
	file_element_t file_array[8];
	int32_t cur_slot;
} pcb_t;

#define SUCCESS 1

typedef int32_t (*open_syscall)(const uint8_t*);
typedef int32_t (*read_syscall)(int32_t, uint8_t*, int32_t);
typedef int32_t (*write_syscall)(int32_t, const uint8_t*, int32_t);
typedef int32_t (*close_syscall)(int32_t);

typedef struct {
	open_syscall open;
	read_syscall read;
	write_syscall write;
	close_syscall close;
} syscall_jump_table;

syscall_jump_table rtc = {open_rtc, read_rtc, write_rtc, close_rtc};
syscall_jump_table file = {file_open, file_read, file_write, file_close};
syscall_jump_table dir = {dir_open, dir_read, dir_write, dir_close};
syscall_jump_table stdin = {NULL, terminal_read, NULL, NULL};
syscall_jump_table stdout = {NULL, NULL, terminal_write, NULL};

int32_t open(const uint8_t* filename)
{
	//find an empty fd
	file_element_t* file_array = some_process->file_array;

	int32_t fd = 0;
	while(file_array[fd].flags == 1)
		fd++;

	dentry_t dentry;
	if(fd == 7 || read_dentry_by_name(filename, &dentry) == -1)
		return -1;

	file_element_t curr_file = file_array[fd];

	//case RTC:
	if(dentry.file_type == 0){
		if(open_rtc(filename) == - 1)
			return -1;
		curr_file.jumptable = &rtc;
		curr_file.inode_ptr = 0;
		curr_file.file_pos = 0;
		curr_file.flags = 1;
	}

	//case directory:
	if(dentry.file_type == 1){
		if(dir_open(filename) == -1)
			return -1;
		curr_file->jumptable = &file;
		curr_file.inode_ptr = dentry.inode_idx;
		curr_file.file_pos = 0;
		curr_file.flags = 1;
	}
	
	//case regular file:
	if(dentry.file_type == 2){
		if(file_open(filename) == -1)
			return -1;	
		curr_file->jumptable = &dir;
		curr_file.inode_ptr = 0;
		curr_file.file_pos = 0;
		curr_file.flags = 1;
	}
	return fd;
}

int32_t close(int32_t fd)
{
	if(fd < 2 || fd > 7)
		return -1;

	file_element_t curr_file = some_process->file_array[fd];
	if(curr_file.flags == 0)
		return -1;

	curr_file->flags = 0;
	curr_file.inode_ptr = 0;
	curr_file.file_pos = 0;
	return SUCCESS;
}

int32_t read(int32_t fd, void* buf, int32_t count)
{
	if(fd <0 || fd > 7 || buf == NULL || nbytes < 0)
		return -1;

	file_element_t curr_file = some_process->file_array[fd];
	if(curr_file.flags == 0 || curr_file.file_op_ptr->read == NULL)
		return -1;

	return curr_file.file_op_ptr->read(fd, buf, count);
}

int32_t write(int32_t fd, const void* buf, int32_t count)
{
	if(fd <0 || fd > 7 || buf == NULL || nbytes < 0)
		return -1;

	file_element_t curr_file = some_process->file_array[fd];
	if(curr_file.flags == 0 || curr_file.file_op_ptr->write == NULL)
		return -1;

	return curr_file.file_op_ptr->write(fd, buf, count);
}

int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t read(int32_t fd, void* buf, int32_t count);
int32_t write(int32_t fd, const void* buf, int32_t count);

//TODO: open stdin, stdout automatically when starting a process