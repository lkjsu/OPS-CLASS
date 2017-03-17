#ifndef _PROC_SYSCALL_H
#define _PROC_SYSCALL_H

#include <../arch/mips/include/trapframe.h>

pid_t sys_getpid(pid_t*);
//pid_t sys_fork(struct trapframe * , pid_t*);
pid_t sys_waitpid(pid_t pid, int *returncode, int flags ,int*);
void fork_func(void *, unsigned long);

pid_t sys_fork(struct trapframe * trap, pid_t* retval);
void sys_exit(int);
#endif