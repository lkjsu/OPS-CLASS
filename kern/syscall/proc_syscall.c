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
		*retval  =-1;
		return ENOMEM;
	}
	memcpy(child_trap, trap, sizeof(struct trapframe));
	struct proc *child_proc;
	child_proc = proc_create_child("ch");
	if(child_proc == NULL){
		*retval  =-1;
		return ENPROC;
	}
	struct addrspace *child_addr = NULL; 
	err = as_copy(curproc->p_addrspace, &child_addr);
	//err = as_copy(curproc->p_addrspace, &child_proc->p_addrspace);
	if(err){
		*retval  =-1;
		return err;
	}
	int i;
	for(i=0; i <= OPEN_MAX; i++){
		child_proc ->file_table[i] =curproc->file_table[i];
	}
	*retval = child_proc->pid;
	//child_proc -> ppid = curproc->pid;
	//child_proc->p_cwd = curproc->p_cwd;
	//curproc->p_numthreads+=1;
	//err = thread_fork("c",child_proc, fork_func, child_trap,( unsigned long) &child_proc->p_addrspace);
	err =thread_fork("fork",child_proc, fork_func, child_trap,( unsigned long) child_addr);
	if(err){
		*retval  =-1;
		return err;
	}
	return 0;
}

void 
fork_func(void *data1, unsigned long data2){
	struct trapframe *trap_f =  (struct trapframe*)data1;
	struct addrspace *addr = (struct addrspace *) data2;
	struct trapframe tf = *trap_f; //kmalloc(sizeof(struct trapframe));
	//memcpy(&tf,trap_f, sizeof(struct trapframe));
	kfree(trap_f);
	//&tf->tf_v0 = 0;
	//&tf->tf_a3 = 0;      /* signal no error */
	//&tf->tf_epc += 4;
	//mips_usermode(&tf);
	
	enter_forked_process(&tf);
	curproc->p_addrspace = addr;
	as_activate ();
	mips_usermode(&tf);
	//(void)data2;
	
}

void
sys_exit(int exitcode){
	 curproc->exit_code = _MKWAIT_EXIT(exitcode);
	//(void)statsss;
	//curproc->p_addrspace = NULL;
	lock_acquire(curproc->wait_lock);
	curproc->exit_stat = true;
	while(proc_table[curproc->ppid]->exit_stat== false){
		//V(proc_table[curproc->pid]->wait_sem);
		cv_signal(curproc->wait_cv, curproc->wait_lock);
	}
	lock_release(curproc->wait_lock);
	kfree(proc_table[curproc->pid]->p_name);
    kfree(proc_table[curproc->pid]);
    proc_table[curproc->pid] = NULL;
	thread_exit();
}

pid_t 
sys_waitpid(pid_t pid, int *returncode, int flags ,int*retval ){
	if(pid < PID_MIN || pid >= PID_MAX){
		*retval = -1;
		return -1;
	}
	lock_acquire(proc_table[pid]->wait_lock);
	if(flags & WNOHANG && proc_table[pid]->exit_stat == false){
			*retval = 0;
			//*returncode = 0;
			return 0;
	}
	if(curproc->pid != proc_table[pid]->ppid){
		return -1;
	}
	while(proc_table[pid]->exit_stat == false){
		//P(proc_table[pid]->wait_sem);
		cv_wait(proc_table[pid]->wait_cv, proc_table[pid]->wait_lock);
	}
	
	   // lock_acquire(proc_table[pid]->exitlock);

    *returncode = proc_table[pid]->exit_code;

    lock_release(proc_table[pid]->wait_lock);

    lock_destroy(proc_table[pid]->wait_lock);
	cv_destroy(proc_table[pid]->wait_cv);
    //sem_destroy(proc_table[pid]->wait_sem);
    //as_destroy(proc_table[pid]->p_addrspace);
    proc_table[pid]->p_addrspace = NULL;
    kfree(proc_table[pid]->p_name);
    kfree(proc_table[pid]);
    proc_table[pid] = NULL;

    return pid;
}
