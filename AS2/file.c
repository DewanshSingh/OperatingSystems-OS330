#include<types.h>
#include<context.h>
#include<file.h>
#include<lib.h>
#include<serial.h>
#include<entry.h>
#include<memory.h>
#include<fs.h>
#include<kbd.h>
#include<pipe.h>


/************************************************************************************/
/***************************Do Not Modify below Functions****************************/
/************************************************************************************/
void free_file_object(struct file *filep)
{
    if(filep)
    {
       os_page_free(OS_DS_REG ,filep);
       stats->file_objects--;
    }
}

struct file *alloc_file()
{
  
  struct file *file = (struct file *) os_page_alloc(OS_DS_REG); 
  file->fops = (struct fileops *) (file + sizeof(struct file)); 
  bzero((char *)file->fops, sizeof(struct fileops));
  stats->file_objects++;
  return file; 
}

static int do_read_kbd(struct file* filep, char * buff, u32 count)
{
  kbd_read(buff);
  return 1;
}

static int do_write_console(struct file* filep, char * buff, u32 count)
{
  struct exec_context *current = get_current_ctx();
  return do_write(current, (u64)buff, (u64)count);
}

struct file *create_standard_IO(int type)
{
  struct file *filep = alloc_file();
  filep->type = type;
  if(type == STDIN)
     filep->mode = O_READ;
  else
      filep->mode = O_WRITE;
  if(type == STDIN){
        filep->fops->read = do_read_kbd;
  }else{
        filep->fops->write = do_write_console;
  }
  filep->fops->close = generic_close;
  filep->ref_count = 1;
  return filep;
}

int open_standard_IO(struct exec_context *ctx, int type)
{
   int fd = type;
   struct file *filep = ctx->files[type];
   if(!filep){
        filep = create_standard_IO(type);
   }else{
         filep->ref_count++;
         fd = 3;
         while(ctx->files[fd])
             fd++; 
   }
   ctx->files[fd] = filep;
   return fd;
}
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/



void do_file_fork(struct exec_context *child)
{
   /*TODO the child fds are a copy of the parent. Adjust the refcount*/
  int i=0;
  while(i<32)
  {
    if(child->files[i])
    child->files[i]->ref_count++;
    i++;
  }
}

void do_file_exit(struct exec_context *ctx)
{
   /*TODO the process is exiting. Adjust the ref_count
     of files*/
  int i=0;
  while(i<32)
  {
    if(ctx->files[i])
    generic_close(ctx->files[i]);
    i++;
  }

}

long generic_close(struct file *filep)
{
  /** TODO Implementation of close (pipe, file) based on the type 
   * Adjust the ref_count, free file object
   * Incase of Error return valid Error code 
   * 
   */
     
    if(!filep) return -1;

    if(filep->type == PIPE)
    {
      if(filep->ref_count==0) return -1;
      filep->ref_count--;
        if(filep->ref_count==0)
        {
           free_pipe_info(filep->pipe);
           free_file_object(filep);
        }
      return 0;
     
    } 

     else
    {
      if(filep->ref_count==0) return -1;
      filep->ref_count--;
      if(filep->ref_count==0)
      free_file_object(filep);
      return 0;
    }
}

static int do_read_regular(struct file *filep, char * buff, u32 count)
{
   /** TODO Implementation of File Read, 
    *  You should be reading the content from File using file system read function call and fill the buf
    *  Validate the permission, file existence, Max length etc
    *  Incase of Error return valid Error code 
    * */

  if((filep->mode)&0x1 && filep!=NULL && count >=0 )
  {
    int read= flat_read(filep->inode, buff, count, &(filep->offp) );
    filep->offp = filep->offp + read;
    return read;
  }
  else
  return -EACCES;     
    
}


static int do_write_regular(struct file *filep, char * buff, u32 count)
{
    /** TODO Implementation of File write, 
    *   You should be writing the content from buff to File by using File system write function
    *   Validate the permission, file existence, Max length etc
    *   Incase of Error return valid Error code 
    * */
   if((filep->mode)&0x2 && filep!=NULL && count >=0)
   {
    int write= flat_write(filep->inode, buff, count, &(filep->offp) );
    if(write >=0)
    filep->offp = filep->offp + write;
    return write;
   }
    else
    return -EACCES;

}

static long do_lseek_regular(struct file *filep, long offset, int whence)
{
    /** TODO Implementation of lseek 
    *   Set, Adjust the ofset based on the whence
    *   Incase of Error return valid Error code 
    * */
  if(filep == NULL) return -EINVAL;
  if(filep->mode&0x1)
  {
      if(whence == SEEK_CUR)
      {
        filep->offp = filep->offp + offset;
        if(filep->offp > 4096)
          return -EINVAL;
        else
          return filep->offp;
           
      }

      if(whence == SEEK_SET)
      {
          if(offset>4096)
              return -EINVAL;
        filep->offp = offset;
        return filep->offp;
      }

      if(whence == SEEK_END)
      {
         filep->offp = filep->inode->file_size + offset;
          if(filep->offp > 4096)
              return -EINVAL;
          else
         return filep->offp;
      }

      else
        return -EINVAL ;
  }
  else
    return -EACCES;
}

extern int do_regular_file_open(struct exec_context *ctx, char* filename, u64 flags, u64 mode)
{ 
  /**  TODO Implementation of file open, 
    *  You should be creating file(use the alloc_file function to creat file), 
    *  To create or Get inode use File system function calls, 
    *  Handle mode and flags 
    *  Validate file existence, Max File count is 32, Max Size is 4KB, etc
    *  Incase of Error return valid Error code 
    * */
  struct inode* inode = lookup_inode(filename);
  struct file *filep = alloc_file();
     if(flags&0x8 || flags&0x1 || flags&0x2)
      {
        if(inode == NULL && flags&0x8 )
          {
            struct inode* new_inode = create_inode(filename,mode);
            filep->inode = new_inode;
          }
        else
          {
            filep->inode=inode;
          }
         
        filep->ref_count = 1;
        filep->type = REGULAR;
        filep->mode = mode;
        filep->offp = 0;
        filep->pipe = NULL;
        filep->fops->read = do_read_regular;
        filep->fops->write = do_write_regular;
        filep->fops->lseek = do_lseek_regular;
        filep->fops->close = generic_close;
        int fd = 0;
        while(ctx->files[fd])
             fd++;  
	if(fd>32)
            return -1;
        ctx->files[fd] = filep;
        return fd;
      }
     
    else
    return -EINVAL; 
}

int fd_dup(struct exec_context *current, int oldfd)
{
     /** TODO Implementation of dup 
      *  Read the man page of dup and implement accordingly 
      *  return the file descriptor,
      *  Incase of Error return valid Error code 
      * */
    if(!(current->files[oldfd]) || oldfd>=32 )
    return -EINVAL;
    
    else
      {
        int fd = 0;
              while(current->files[fd])
                   fd++;  
        if(fd>=32)
            return -1;
          
        current->files[fd] = current->files[oldfd];
        current->files[fd]->ref_count++;
        return fd;
      }


}


int fd_dup2(struct exec_context *current, int oldfd, int newfd)
{
  /** TODO Implementation of the dup2 
    *  Read the man page of dup2 and implement accordingly 
    *  return the file descriptor,
    *  Incase of Error return valid Error code 
    * */
    
     if(!(current->files[oldfd]) || oldfd>=32 || newfd>=32 )
      return -EINVAL;

    if(current->files[oldfd])
      {
        if( oldfd!=newfd && current->files[newfd] )
        {
            current->files[newfd]->fops->close(current->files[newfd]);
        }
        current->files[newfd] = current->files[oldfd];
        current->files[oldfd]->ref_count++;
        return newfd;
      }

    else return -EINVAL;
    
}


