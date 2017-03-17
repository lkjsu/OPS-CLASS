#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <file_syscall.h>
#include <vnode.h>
#include <vfs.h>
#include <synch.h>
#include <copyinout.h>
#include <kern/fcntl.h>
#include <fs.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <proc_syscall.h>
#include <limits.h>
#include <syscall.h>
#include <thread.h>
#include <../arch/mips/include/trapframe.h>
#include <kern/wait.h>
pid_t 
sys_getpid(pid_t* retval){
	*retval = curproc->pid;
	return 0;
}
pid_t 
sys_fork(struct trapframe * trap, pid_t* retval){
	int err = 0;
	struct trapframe *child_trap = kmalloc(sizeof(struct trapframe));
	if(child_trap == NULL){
		return -1;
	}
	memcpy(child_trap, trap, sizeof(struct trapframe));
	struct proc *child_proc;
	child_proc = proc_create_child("[ch]");
	if(child_proc == NULL){
		return -1;
	}
	//struct addrspace *child_addr = NULL; 
	err = as_copy(curproc->p_addrspace, &child_proc->p_addrspace);
	if(err){
		return err;
	}
	int i;
	for(i=0; i <= OPEN_MAX; i++){
		child_proc ->file_table[i] =curproc->file_table[i];
	}
	child_proc -> ppid = curproc->pid;
	thread_fork("ch",child_proc, fork_func, child_trap,( unsigned long) &child_proc->p_addrspace);
	*retval = child_proc->pid;
	return 0;
}

void 
fork_func(void *data1, unsigned long data2){
	struct trapframe *trap_f =  data1;
	//struct addrspace *addr = (struct addrspace *) data2;
	struct trapframe tf = *trap_f;
	//&tf->tf_v0 = 0;
	//&tf->tf_a3 = 0;      /* signal no error */
	//&tf->tf_epc += 4;
	//mips_usermode(&tf);
	
	enter_forked_process(&tf);
	//curproc->p_addrspace = addr;
	//as_activate ();
	mips_usermode(&tf);
	(void)data2;
	
}

void
sys_exit(int exitcode){
	int statsss = _MKWAIT_EXIT(exitcode);
	(void)statsss;
	thread_exit();
}
//pid_t 
//sys_waitpid(pid_t pid, int *returncode, int flags ,int*retval ){
	
//}
