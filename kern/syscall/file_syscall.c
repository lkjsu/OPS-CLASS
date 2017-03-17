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
#include <limits.h>

int 
stdio_fd(){
	int file_desc;
	for (file_desc = 0; file_desc<3; file_desc++){
		curproc->file_table[file_desc] = kmalloc(sizeof(struct file_handler));
		if(curproc->file_table[file_desc] == NULL){
			return -1;
		}
		curproc->file_table[file_desc]->name = kstrdup("con:");
		if(curproc->file_table[file_desc]->name == NULL){
			kfree(curproc->file_table[file_desc]);
			return -1;
		}
		
		curproc->file_table[file_desc]-> offset = 0;
		curproc->file_table[file_desc] -> refrence = 1;
		int err;
		
		if(file_desc == 0){
			curproc->file_table[file_desc] ->flag = O_RDONLY;
			
			err = vfs_open(curproc->file_table[file_desc] ->name,O_RDONLY, 0664, &curproc->file_table[file_desc] ->file_node);
			if(!err){
				curproc->file_table[file_desc]->file_lock = lock_create("read_only");
				if (curproc->file_table[file_desc] ->file_lock ==NULL){
					lock_destroy(curproc->file_table[file_desc] ->file_lock);
					err = -1;
				}
			}
			if(err){
				vfs_close(curproc->file_table[file_desc] ->file_node);
				kfree(curproc->file_table[file_desc]->name);
				kfree(curproc->file_table[file_desc]);
				return err;
			}
		}
		
		else if(file_desc==1){
			curproc->file_table[file_desc] ->flag = O_WRONLY;
			err = vfs_open(curproc->file_table[file_desc] ->name,O_WRONLY, 0664, &curproc->file_table[file_desc] ->file_node);
			if(!err){
				curproc->file_table[file_desc]->file_lock = lock_create("write_only");
				if (curproc->file_table[file_desc] ->file_lock ==NULL){
					lock_destroy(curproc->file_table[file_desc] ->file_lock);
					err = -1;
				}
			}
			if(err){
				vfs_close(curproc->file_table[file_desc] ->file_node);
				kfree(curproc->file_table[file_desc]->name);
				kfree(curproc->file_table[file_desc]);
				return err;
			}
		}
		
		else if(file_desc==2){
			curproc->file_table[file_desc] ->flag = O_WRONLY;
			err = vfs_open(curproc->file_table[file_desc] ->name, O_WRONLY, 0664, &curproc->file_table[file_desc] ->file_node);
			if(!err){
				curproc->file_table[file_desc]->file_lock = lock_create("error");
				if (curproc->file_table[file_desc] ->file_lock ==NULL){
					lock_destroy(curproc->file_table[file_desc] ->file_lock);
					err = -1;
				}
			}
			if(err){
				vfs_close(curproc->file_table[file_desc] ->file_node);
				kfree(curproc->file_table[file_desc]->name);
				kfree(curproc->file_table[file_desc]);
				return err;
			}
		}
	}
	return 0;
}
int 
sys_open(const char *filename, int flags, int *retval){
	
	
	int file_desc;
	//int new_flag;
	off_t file_offset = 0;
	int err;
	size_t size;
	char* copy = kmalloc(sizeof(filename)*PATH_MAX );
	if(copy == NULL){
		 *retval = -1;
		 return -1;
	}
	if (filename == NULL){
		kfree(copy);
		 *retval = -1;
		return EFAULT;
	}
	//new_flag = O_CREAT | O_EXCL;
	err = copyinstr((const_userptr_t)filename, copy, PATH_MAX,&size);
	if(err){
		kfree(copy);
		*retval = -1;
		return err;
	}
	for(file_desc=3;file_desc<=OPEN_MAX ;file_desc++){
		//if((flags & new_flag)==new_flag && curproc->file_table[file_desc]->name==filename ){
		//	return EEXIST;
		//}
		if(curproc->file_table[file_desc]==NULL){
				//curproc->file_table[file_desc] = fh;
				//return file_desc;
				//break;
		
			//struct file_handler *fh;
			curproc->file_table[file_desc]  = kmalloc(sizeof(struct file_handler));
			if(curproc->file_table[file_desc]  == NULL){
				kfree(copy);
				return -1;
			}
			curproc->file_table[file_desc] ->name = kstrdup(filename);
			if(curproc->file_table[file_desc] ->name==NULL){
				kfree(copy);
				kfree(curproc->file_table[file_desc] );
				return -1;
			}
			//if((flags & O_APPEND)==O_APPEND){
				
			//}

			curproc->file_table[file_desc] ->refrence=1;
			curproc->file_table[file_desc] ->offset=file_offset;
			curproc->file_table[file_desc] ->flag = flags;
			curproc->file_table[file_desc] ->file_lock = lock_create(filename);
			if(curproc->file_table[file_desc] ->file_lock == NULL){
				 kfree(copy);
				 kfree(curproc->file_table[file_desc]->name);
				 kfree(curproc->file_table[file_desc] );
				 *retval = -1;
				 return -1;
			}
			 err = vfs_open(copy,flags,0664,&curproc->file_table[file_desc]  ->file_node);
			 if(err){
				 kfree(copy);
				 lock_destroy(curproc->file_table[file_desc] ->file_lock);
				 kfree(curproc->file_table[file_desc]->name);
				 kfree(curproc->file_table[file_desc] );
				  *retval = -1;
				 return EIO;
			 }
			 kfree(copy);
			 *retval = file_desc;
			return 0;
		}
	}
	*retval = -1;
	return EFAULT;
}
ssize_t 
sys_write(int filehandle, void *buf, size_t size, int *retval){
	int err;
	struct iovec iov;
	struct uio ku;
     void* buff = kmalloc(sizeof(buf));
	 if(buff == NULL){
		 kfree(buff);
		 *retval = -1;
		 return -1;
	 }
	 if(buf == NULL){
		 *retval = -1;
		 return EFAULT;
	 }
	 if(size <=0){
		 *retval = -1;
		 return EFAULT;
	 }
	 if(  curproc->file_table[filehandle] == NULL){
		 *retval = -1;
		return EBADF;
	}
	lock_acquire( curproc->file_table[filehandle] ->file_lock);
	err =copyin((const_userptr_t)buf, buff,sizeof(buf));
	kfree(buff);
	if(err){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return err;
	}
	if(filehandle < 0 || filehandle >OPEN_MAX){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EBADF;
	}
	if((curproc->file_table[filehandle] ->flag & O_WRONLY)  != O_WRONLY && (curproc->file_table[filehandle] ->flag  & O_RDWR )!= O_RDWR ){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = 1;
		return EBADF;
	}
	uio_kinit(&iov, &ku, (userptr_t)buf, size, curproc->file_table[filehandle] ->offset, UIO_WRITE);
	ku.uio_space = curproc->p_addrspace;
	ku.uio_segflg = UIO_USERSPACE;
	/*iov.iov_ubase =(userptr_t) buf;
	iov.iov_len = size;
	ku.uio_iov = &iov;
	ku.uio_iovcnt = 1;
	ku.uio_resid = size;
	ku.uio_offset = curproc->file_table[filehandle] ->offset;
	ku.uio_space = curproc->p_addrspace;
	ku.uio_segflg = UIO_USERSPACE;
	ku.uio_rw = UIO_WRITE;*/
	err = VOP_WRITE( curproc->file_table[filehandle] ->file_node, &ku);
	if(err){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EIO;
	}
	curproc->file_table[filehandle]->offset = curproc->file_table[filehandle]->offset + size - ku.uio_resid;
	lock_release( curproc->file_table[filehandle] ->file_lock);
	*retval = size - ku.uio_resid ;
	return 0;
}
/*
ssize_t 
sys_write(int filehandle, void *buf, size_t size, int *retval){
	int err;
	struct iovec iov;
	struct uio ku;
     void* buff = kmalloc(sizeof(buf));
	 if(buff == NULL){
		 kfree(buff);
		 *retval = -1;
		 return -1;
	 }
	 if(buf == NULL){
		 *retval = -1;
		 return EFAULT;
	 }
	 if(size <=0){
		 *retval = -1;
		 return EFAULT;
	 }
	 if(  curproc->file_table[filehandle] == NULL){
		 *retval = -1;
		return EBADF;
	}
	lock_acquire( curproc->file_table[filehandle] ->file_lock);
	err =copyin((const_userptr_t)buf, buff,sizeof(buf));
	kfree(buff);
	if(err){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return err;
	}
	if(filehandle < 0 || filehandle >128){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EBADF;
	}
	if((curproc->file_table[filehandle] ->flag & O_WRONLY)  != O_WRONLY && (curproc->file_table[filehandle] ->flag  & O_RDWR )!= O_RDWR ){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = 1;
		return EBADF;
	}
	//uio_kinit(&iov, &ku, (userptr_t)buf, size, curproc->file_table[filehandle] ->offset, UIO_WRITE);
	//ku.uio_space = curproc->p_addrspace;
	//ku.uio_segflg = UIO_USERSPACE;
	iov.iov_ubase =(userptr_t) buf;
	iov.iov_len = size;
	ku.uio_iov = &iov;
	ku.uio_iovcnt = 1;
	ku.uio_resid = size;
	ku.uio_offset = curproc->file_table[filehandle] ->offset;
	ku.uio_space = curproc->p_addrspace;
	ku.uio_segflg = UIO_USERSPACE;
	ku.uio_rw = UIO_WRITE;
	err = VOP_WRITE( curproc->file_table[filehandle] ->file_node, &ku);
	if(err){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EIO;
	}
	curproc->file_table[filehandle]->offset = curproc->file_table[filehandle]->offset + size - ku.uio_resid;
	lock_release( curproc->file_table[filehandle] ->file_lock);
	*retval = size - ku.uio_resid ;
	return 0;
}*/

ssize_t 
sys_read(int filehandle, void *buf, size_t size, int *retval){
	int err;
	struct iovec iov;
	struct uio ku;
     void* buff = kmalloc(sizeof(buf));
	 if(buff == NULL){
		 kfree(buff);
		 return -1;
	 }
	 if(buf == NULL){
		 *retval = -1;
		 return EFAULT;
	 }
	 if(size <=0){
		 *retval = -1;
		 return EFAULT;
	 }
	 if(  curproc->file_table[filehandle] == NULL){
		 *retval = -1;
		return EBADF;
	}
	lock_acquire( curproc->file_table[filehandle] ->file_lock);
	err =copyin(buf, buff,sizeof(buf));
	kfree(buff);
	if(err){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return err;
	}
	if(filehandle < 0 && filehandle >OPEN_MAX){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EBADF;
	}
	if((curproc->file_table[filehandle] ->flag & O_RDONLY) != O_RDONLY && (curproc->file_table[filehandle] ->flag & O_RDWR) != O_RDWR ){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EBADF;
	}
	uio_kinit(&iov, &ku, (userptr_t)buf, size, curproc->file_table[filehandle] ->offset, UIO_READ);
	ku.uio_space = curproc->p_addrspace;
	ku.uio_segflg = UIO_USERSPACE;
	/*iov.iov_ubase =(userptr_t) buf;
	iov.iov_len = size;
	ku.uio_iov = &iov;
	ku.uio_iovcnt = 1;
	ku.uio_resid = size;
	ku.uio_offset = curproc->file_table[filehandle] ->offset;
	ku.uio_space = curproc->p_addrspace;
	ku.uio_segflg = UIO_USERSPACE;
	ku.uio_rw = UIO_READ;*/
	err = VOP_READ( curproc->file_table[filehandle] ->file_node, &ku);
	if(err){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EIO;
	}
	//curproc->file_table[filehandle]->offset = curproc->file_table[filehandle]->offset + size;
	*retval = size- ku.uio_resid ;
	lock_release( curproc->file_table[filehandle] ->file_lock);

	return 0;
}

int 
sys_close(int filehandle, int *retval){
	if(filehandle<0 ||filehandle>OPEN_MAX){
		*retval = -1;
		return EBADF;
	}
	vfs_close(curproc->file_table[filehandle]->file_node);
	kfree(curproc->file_table[filehandle]->name);
	//kfree(curproc->file_table[filehandle]);
	curproc->file_table[filehandle] = NULL;
	return 0;
}

off_t 
sys_lseek(int filehandle, off_t pos, int code, off_t *retval){
	int err;
	if(filehandle < 0 || filehandle > OPEN_MAX){
		*retval = -1;
		return EBADF;
	}
	if(curproc->file_table[filehandle] == NULL){
		return EBADF;
	}
		if(!VOP_ISSEEKABLE(curproc->file_table[filehandle]->file_node))
	 {
		*retval = -1;
		return ESPIPE;
	}
	lock_acquire(curproc->file_table[filehandle]->file_lock);
	if(code == SEEK_SET){
		
		curproc->file_table[filehandle]->offset = pos;
		
	}else if(code ==SEEK_CUR){
		
		curproc->file_table[filehandle]->offset += pos;
		
	}else if(code == SEEK_END){
		struct stat proc_stat;
		err = VOP_STAT(curproc->file_table[filehandle]->file_node, &proc_stat);
		if(err){
			*retval = -1;
			return err;
		}
		curproc->file_table[filehandle]->offset = proc_stat.st_size + pos;
	}else{
		*retval = -1;
		return EINVAL;
	}
	
	*retval = curproc->file_table[filehandle]->offset;
	lock_release(curproc->file_table[filehandle]->file_lock);
	return 0;
}

int 
sys_chdir(const char *path, int *retval){
	if(path == NULL){
		*retval = -1;
		return EFAULT;
	}
		int err;
	size_t size;
	char* copy = kmalloc(sizeof(path));
	err = copyinstr((const_userptr_t)path, copy, sizeof(path),&size);
	if(err){
		*retval = -1;
		return err;
	}
	err = vfs_chdir(copy);
	if(err){
		*retval = -1;
		return err;
	}
	*retval =0;
	return 0;
}

int 
sys_dup2(int filehandle, int newhandle, int *retval){
	(void)filehandle;
	(void) newhandle;
	*retval = -1;
	return 0;
}

ssize_t 
sys__getcwd(char *buf, size_t buflen, int *retval){
	int err;
	struct uio ku;
     char* buff = kmalloc(sizeof(buf));
	 size_t size;
	 struct iovec iov;
	 if(buff == NULL){
		 *retval = -1;
		 kfree(buff);
		 return -1;
	 }
	 if(buf == NULL){
		 *retval = -1;
		 return EFAULT;
	 }
	 if(buflen <=0){
		 *retval = -1;
		 return EFAULT;
	 }
	 err = copyinstr((const_userptr_t)buf, buff, sizeof(buf),&size);
	 if(err){
		 kfree(buff);
		 *retval = -1;
	   	return err;
	 }
	uio_kinit(&iov, &ku, buf, buflen, 0, UIO_READ);
	ku.uio_space = curproc->p_addrspace;
	ku.uio_segflg = UIO_USERSPACE;
	err = vfs_getcwd(&ku);
	if (err){
		*retval = -1;
		return err;
	}
	*retval = buflen;
	return 0;
}



