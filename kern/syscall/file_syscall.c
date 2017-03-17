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
			return EBADF;;
		}
		char *std_type = kstrdup("con:");
		if(std_type == NULL){
			kfree(curproc->file_table[file_desc]);
			return EBADF;
		}
		curproc->file_table[file_desc]->name = kstrdup("con:");
		if(curproc->file_table[file_desc]->name == NULL){
			kfree(curproc->file_table[file_desc]);
			return EBADF;
		}
		
		curproc->file_table[file_desc]-> offset = 0;
		curproc->file_table[file_desc] -> refrence = 1;
		int err;
		
		if(file_desc == 0){
			curproc->file_table[file_desc] ->flag = O_RDONLY;
			
			err = vfs_open(std_type,O_RDONLY, 0664, &curproc->file_table[file_desc] ->file_node);
			if(!err){
				curproc->file_table[file_desc]->file_lock = lock_create("read_only");
				if (curproc->file_table[file_desc] ->file_lock ==NULL){
					lock_destroy(curproc->file_table[file_desc] ->file_lock);
					err = -1;
				}
			}
			if(err){
				vfs_close(curproc->file_table[file_desc] ->file_node);
				kfree(std_type);
				kfree(curproc->file_table[file_desc]->name);
				kfree(curproc->file_table[file_desc]);
				curproc->file_table[file_desc] = NULL;
				return err;
			}
		}
		
		else if(file_desc==1){
			curproc->file_table[file_desc] ->flag = O_WRONLY;
			err = vfs_open(std_type,O_WRONLY, 0664, &curproc->file_table[file_desc] ->file_node);
			if(!err){
				curproc->file_table[file_desc]->file_lock = lock_create("write_only");
				if (curproc->file_table[file_desc] ->file_lock ==NULL){
					lock_destroy(curproc->file_table[file_desc] ->file_lock);
					err = -1;
				}
			}
			if(err){
				vfs_close(curproc->file_table[0] ->file_node);
				kfree(curproc->file_table[0]->name);
				kfree(curproc->file_table[0]);
				curproc->file_table[0] = NULL;
				kfree(std_type);
				vfs_close(curproc->file_table[file_desc] ->file_node);
				kfree(curproc->file_table[file_desc]->name);
				kfree(curproc->file_table[file_desc]);
				curproc->file_table[file_desc] = NULL;
				
				return err;
			}
		}
		
		else if(file_desc==2){
			curproc->file_table[file_desc] ->flag = O_WRONLY;
			err = vfs_open(std_type, O_WRONLY, 0664, &curproc->file_table[file_desc] ->file_node);
			if(!err){
				curproc->file_table[file_desc]->file_lock = lock_create("error");
				if (curproc->file_table[file_desc] ->file_lock ==NULL){
					lock_destroy(curproc->file_table[file_desc] ->file_lock);
					err = -1;
				}
			}
			if(err){
				vfs_close(curproc->file_table[0] ->file_node);
				kfree(curproc->file_table[0]->name);
				kfree(curproc->file_table[0]);
				curproc->file_table[0] = NULL;
				kfree(std_type);
				vfs_close(curproc->file_table[1] ->file_node);
				kfree(curproc->file_table[1]->name);
				kfree(curproc->file_table[1]);
				curproc->file_table[1] = NULL;
				kfree(std_type);
				vfs_close(curproc->file_table[file_desc] ->file_node);
				kfree(curproc->file_table[file_desc]->name);
				kfree(curproc->file_table[file_desc]);
				curproc->file_table[file_desc] = NULL;
				return err;
			}
		}
	}
	return 0;
}
int 
sys_open(const char *filename, int flags, int *retval){
	if (filename == NULL){
		 *retval = -1;
		return EFAULT;
	}
	int file_desc;
	int new_flag = flags;
	off_t file_offset = 0;
	int err;
	size_t size;
	
	char* copy = kmalloc(sizeof(filename)*PATH_MAX );
	if(copy == NULL){
		 *retval = -1;
		 return -1;
	}
	err = copyinstr((const_userptr_t)filename, copy, PATH_MAX,&size);
	if(err){
		kfree(copy);
		*retval = -1;
		return err;
	}
	if(strlen(filename)==0){
		*retval = -1;
		kfree(copy);
		return EINVAL;
	}
		if((flags & O_WRONLY && flags  & O_RDONLY  )|| (flags & O_RDONLY && flags  & O_RDWR )|| (flags & O_WRONLY && flags  & O_RDWR  )){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = 1;
		return EINVAL	;
	}
			//if(((flags & O_RDONLY)  == O_RDONLY) && ((flags  & O_RDWR )== O_RDWR )){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		//*retval = 1;
		//return EINVAL	;
	//}
			//if(((flags & O_WRONLY)  == O_WRONLY )&&((flags  & O_RDWR )== O_RDONLY )){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		//*retval = 1;
		//return EINVAL	;
	//}

	for(file_desc=3;file_desc<OPEN_MAX ;file_desc++){
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
		/*	if((flags & O_APPEND)==O_APPEND){
				struct stat proc_stat;
				err = VOP_STAT(curproc->file_table[file_desc]->file_node, &proc_stat);
				if(err){
					*retval = -1;
					return err;
				}
				file_offset = proc_stat.st_size;
				}*/


			 err = vfs_open(copy,flags,0664,&curproc->file_table[file_desc]  ->file_node);
			 if(!err){
				 			curproc->file_table[file_desc] ->refrence=1;
			curproc->file_table[file_desc] ->offset=file_offset;
			curproc->file_table[file_desc] ->flag = new_flag;
			curproc->file_table[file_desc] ->file_lock = lock_create(filename);
			if(curproc->file_table[file_desc] ->file_lock == NULL){
				lock_destroy(curproc->file_table[file_desc] ->file_lock);
				 err = -1;
			}
			 }
			 if(err){
				 kfree(copy);
				 //lock_destroy(curproc->file_table[file_desc] ->file_lock);
				 kfree(curproc->file_table[file_desc]->name);
				 kfree(curproc->file_table[file_desc] );
				  *retval = -1;
				 return err;
			 }
			 //kfree(copy);
			 *retval = file_desc;
			return 0;
		}
	}
	*retval = -1;
	return ENFILE;
}
ssize_t 
sys_write(int filehandle, void *buf, size_t size, int *retval){
	int err;
	struct iovec iov;
	struct uio ku;
		 if(buf == NULL){
		 *retval = -1;
		 return EFAULT;
	 }
	 if(size <=0){
		 *retval = -1;
		 return EFAULT;
	 }
     void* buff = kmalloc(sizeof(buf));
	 if(buff == NULL){
		 kfree(buff);
		 *retval = -1;
		 return EFAULT;
	 }
	 	if(filehandle < 0 || filehandle >=OPEN_MAX){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EBADF;
	}
	 if(  curproc->file_table[filehandle] == NULL){
		 *retval = -1;
		return EBADF;
	}
	//lock_acquire( curproc->file_table[filehandle] ->file_lock);
	err =copyin((const_userptr_t)buf, buff,sizeof(buf));
	kfree(buff);
	if(err){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return err;
	}

	if((curproc->file_table[filehandle] ->flag & O_WRONLY)  != O_WRONLY && (curproc->file_table[filehandle] ->flag  & O_RDWR )!= O_RDWR ){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = 1;
		return EBADF;
	}
		if(curproc->file_table[filehandle] ->flag & O_RDONLY){
		*retval = 1;
		return EBADF;
	}
	lock_acquire( curproc->file_table[filehandle] ->file_lock);
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
		return err;
	}
	curproc->file_table[filehandle]->offset = curproc->file_table[filehandle]->offset + size - ku.uio_resid;
	lock_release( curproc->file_table[filehandle] ->file_lock);
	*retval = size - ku.uio_resid ;
	return 0;
}
ssize_t 
sys_read(int filehandle, void *buf, size_t size, int *retval){
	int err;
	struct iovec iov;
	struct uio ku;
		 if(buf == NULL){
		 *retval = -1;
		 return EFAULT;
	 }
	 if(size <=0){
		 *retval = -1;
		 return EFAULT;
	 }
     void* buff = kmalloc(sizeof(buf));
	 if(buff == NULL){
		 kfree(buff);
		 *retval = -1;
		 return EFAULT;
	 }
	 	if(filehandle < 0 || filehandle >=OPEN_MAX){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EBADF;
	}
	 if(  curproc->file_table[filehandle] == NULL){
		 *retval = -1;
		return EBADF;
	}
	//lock_acquire( curproc->file_table[filehandle] ->file_lock);
	err =copyin((const_userptr_t)buf, buff,sizeof(buf));
	kfree(buff);
	if(err){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return err;
	}

	if((curproc->file_table[filehandle] ->flag & O_RDONLY)  != O_RDONLY && (curproc->file_table[filehandle] ->flag  & O_RDWR )!= O_RDWR ){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = 1;
		return EBADF;
	}
	if(curproc->file_table[filehandle] ->flag & O_WRONLY){
		*retval = 1;
		return EBADF;
	}
	lock_acquire( curproc->file_table[filehandle] ->file_lock);
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
	ku.uio_rw = UIO_WRITE;*/
	err = VOP_READ( curproc->file_table[filehandle] ->file_node, &ku);
	if(err){
		lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return err;
	}
	curproc->file_table[filehandle]->offset = curproc->file_table[filehandle]->offset + size - ku.uio_resid;
	lock_release( curproc->file_table[filehandle] ->file_lock);
	*retval = size - ku.uio_resid ;
	return 0;
}

/*
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
	 	if(filehandle < 0 && filehandle >=OPEN_MAX){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EBADF;
	}
	 if(  curproc->file_table[filehandle] == NULL){
		 *retval = -1;
		return EBADF;
	}
	//lock_acquire( curproc->file_table[filehandle] ->file_lock);
	err =copyin(buf, buff,sizeof(buf));
	kfree(buff);
	if(err){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return err;
	}
	if((curproc->file_table[filehandle] ->flag & O_RDONLY) != O_RDONLY && (curproc->file_table[filehandle] ->flag & O_RDWR) != O_RDWR ){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EBADF;
	}
	//lock_acquire( curproc->file_table[filehandle] ->file_lock);
	uio_kinit(&iov, &ku, (userptr_t)buf, size,0, UIO_READ);
	ku.uio_space = curproc->p_addrspace;
	ku.uio_segflg = UIO_USERSPACE;
	iov.iov_ubase =(userptr_t) buf;
	iov.iov_len = size;
	ku.uio_iov = &iov;
	ku.uio_iovcnt = 1;
	ku.uio_resid = size;
	ku.uio_offset = curproc->file_table[filehandle] ->offset;
	ku.uio_space = curproc->p_addrspace;
	ku.uio_segflg = UIO_USERSPACE;
	ku.uio_rw = UIO_READ;
	lock_acquire( curproc->file_table[filehandle] ->file_lock);
	err = VOP_READ( curproc->file_table[filehandle] ->file_node, &ku);
	if(err){
		//lock_release( curproc->file_table[filehandle] ->file_lock);
		*retval = -1;
		return EIO;
	}
	//curproc->file_table[filehandle]->offset = curproc->file_table[filehandle]->offset + size;
	//lock_release( curproc->file_table[filehandle] ->file_lock);
	*retval = size- ku.uio_resid ;
	return 0;
}
*/
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

int 
sys_close(int filehandle, int *retval){
	if(filehandle<0 ||filehandle>=OPEN_MAX){
		*retval = -1;
		return EBADF;
	}
	if(curproc->file_table[filehandle] ==NULL){
		*retval = -1;
		return EBADF;
	}
	vfs_close(curproc->file_table[filehandle]->file_node);
	kfree(curproc->file_table[filehandle]->name);
	lock_destroy(curproc->file_table[filehandle]->file_lock);
	//kfree(curproc->file_table[filehandle]);
	curproc->file_table[filehandle] = NULL;
	return 0;
}

off_t 
sys_lseek(int filehandle, off_t pos, int code, off_t *retval){
	int err;
	if(filehandle < 0 || filehandle >=OPEN_MAX){
		*retval = -1;
		return EBADF;
	}
	if(curproc->file_table[filehandle] == NULL){
		return EBADF;
	}
/*		if(!VOP_ISSEEKABLE(curproc->file_table[filehandle]->file_node))
	 {
		*retval = -1;
		return ESPIPE;
	}*/
	lock_acquire(curproc->file_table[filehandle]->file_lock);
	if(!VOP_ISSEEKABLE(curproc->file_table[filehandle]->file_node))
	 {
		lock_release(curproc->file_table[filehandle]->file_lock);
		*retval = -1;
		return ESPIPE;
	}
	if(code == SEEK_SET){
		if(pos < 0){
			lock_release(curproc->file_table[filehandle]->file_lock);
			*retval = -1;
			return EINVAL;
		}
	// if(!VOP_ISSEEKABLE(curproc->file_table[filehandle]->file_node))
	 // {
		 // lock_release(curproc->file_table[filehandle]->file_lock);
		// *retval = -1;
		// return ESPIPE;
	// }
		curproc->file_table[filehandle]->offset = pos;

	}else if(code ==SEEK_CUR){
		if(curproc->file_table[filehandle]->offset + pos < 0){
			lock_release(curproc->file_table[filehandle]->file_lock);
			*retval = -1;
			return EINVAL;
		}
		// if(!VOP_ISSEEKABLE(curproc->file_table[filehandle]->file_node))
	 // {
		 // lock_release(curproc->file_table[filehandle]->file_lock);
		// *retval = -1;
		// return ESPIPE;
	// }
		
		curproc->file_table[filehandle]->offset += pos;

		
	}else if(code == SEEK_END){
		struct stat proc_stat;
		err = VOP_STAT(curproc->file_table[filehandle]->file_node, &proc_stat);
		if(err){
			lock_release(curproc->file_table[filehandle]->file_lock);
			*retval = -1;
			return err;
		}
		// if(!VOP_ISSEEKABLE(curproc->file_table[filehandle]->file_node))
	 // {
		 // lock_release(curproc->file_table[filehandle]->file_lock);
		// *retval = -1;
		// return ESPIPE;
	// }
		if( proc_stat.st_size + pos < 0){
			lock_release(curproc->file_table[filehandle]->file_lock);
			*retval = -1;
			return EINVAL;
		}
		curproc->file_table[filehandle]->offset = proc_stat.st_size + pos;

	}else{
		lock_release(curproc->file_table[filehandle]->file_lock);
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
	kfree(copy);
	if(err){
		*retval = -1;
		return err;
	}
	*retval =0;
	return 0;
}


int 
sys_dup2(int filehandle, int newhandle, int *retval){
	if(filehandle==newhandle){
		return newhandle;
	}
	if(filehandle< 0 || filehandle >=OPEN_MAX ||newhandle< 0 || newhandle>=OPEN_MAX ){
		*retval = -1;
		return EBADF;
	}
	curproc->file_table[newhandle] = curproc->file_table[filehandle ];
	*retval = newhandle;
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
/*
int
sys_remove(const char *filename,int *retval){
	vfs_remove((char *)filename);
	*retval = 0;
	return 0;
}
*/

