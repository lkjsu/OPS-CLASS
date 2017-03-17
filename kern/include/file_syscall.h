#ifndef _FILE_SYSCALL_H
#define _FILE_SYSCALL_H


struct file_handler{
	struct vnode *file_node;
	char *name;
	int flag;
	off_t offset;
	int refrence;
	struct lock *file_lock;
};

int stdio_fd(void);
int sys_open(const char *, int , int*);
ssize_t sys_read(int , void *, size_t,int* );
ssize_t sys_write(int ,  void *, size_t,int* );
int sys_close(int, int*);
off_t sys_lseek(int , off_t , int, off_t*);
int sys_dup2(int , int, int*);
ssize_t sys__getcwd(char *, size_t, int*);
int sys_chdir(const char *, int*);
#endif
