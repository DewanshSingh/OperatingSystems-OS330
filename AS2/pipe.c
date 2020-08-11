#include<pipe.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<entry.h>
#include<file.h>
/***********************************************************************
 * Use this function to allocate pipe info && Don't Modify below function
 ***********************************************************************/
struct pipe_info* alloc_pipe_info()
{
    struct pipe_info *pipe = (struct pipe_info*)os_page_alloc(OS_DS_REG);
    char* buffer = (char*) os_page_alloc(OS_DS_REG);
    pipe ->pipe_buff = buffer;
    return pipe;
}


void free_pipe_info(struct pipe_info *p_info)
{
    if(p_info)
    {
        os_page_free(OS_DS_REG ,p_info->pipe_buff);
        os_page_free(OS_DS_REG ,p_info);
    }
}
/*************************************************************************/
/*************************************************************************/


int pipe_read(struct file *filep, char *buff, u32 count)
{
    /**
    *  TODO:: Implementation of Pipe Read
    *  Read the contect from buff (pipe_info -> pipe_buff) and write to the buff(argument 2);
    *  Validate size of buff, the mode of pipe (pipe_info->mode),etc
    *  Incase of Error return valid Error code 
    */

    if(filep ==NULL || count <0 ) return -1;
    if((filep->mode)&0x1 && filep->pipe->is_ropen)
       {
          
            int i =filep->pipe->read_pos;
            if(count>filep->pipe->buffer_offset) 
		count = filep->pipe->buffer_offset;
            else
            {
                int j =0;
                while(j<count)
                {

                    buff[j] = *(filep->pipe->pipe_buff + i%4096);
                    i++;
                    j++;
                }
                filep->pipe->read_pos += count%4096;
		filep->pipe->buffer_offset -= count;
                return count;
            }
           
           }

    else
    return -EACCES;
}


int pipe_write(struct file *filep, char *buff, u32 count)
{
    /**
    *  TODO:: Implementation of Pipe Read
    *  Write the contect from   the buff(argument 2);  and write to buff(pipe_info -> pipe_buff)
    *  Validate size of buff, the mode of pipe (pipe_info->mode),etc
    *  Incase of Error return valid Error code 
    */
		
    if(filep==NULL || count <0 ) return -1;
    if((filep->mode)&0x2 && filep->pipe->is_wopen)
       {
            if( sizeof(buff) > 4096 )
            {
                return -EOTHERS;
            }

            int i =filep->pipe->write_pos;
            if(( filep->pipe->buffer_offset + count)>4096)
            {
                return -1;
            }

            else
            {
                int j=0;
                while(j<count)
                {

                    *(filep->pipe->pipe_buff + i%4096) = buff[j];
                    i++;
                    j++;
                }
                filep->pipe->write_pos += count%4096;
                filep->pipe->buffer_offset +=count;
                return count;
            }
           

           }

    else
    return -EACCES;
   
}

int create_pipe(struct exec_context *current, int *fd)
{
    /**
    *  TODO:: Implementation of Pipe Create
    *  Create file struct by invoking the alloc_file() function, 
    *  Create pipe_info struct by invoking the alloc_pipe_info() function
    *  fill the valid file descriptor in *fd param
    *  Incase of Error return valid Error code 
    */

    
    int newfd =0;
    struct file * filep1 = alloc_file();
    struct file * filep2 = alloc_file();

    struct pipe_info* pipe = alloc_pipe_info();
        
//read port 

        filep1->type = REGULAR;
        filep1->mode = O_READ;
        filep1->offp = 0;
        filep1->ref_count = 1;
        filep1->inode = NULL;

        filep1->fops->read = pipe_read;
        filep1->fops->write = pipe_write;
        filep1->fops->close = generic_close;

        filep1->pipe = pipe;

        filep1->pipe->read_pos = 0;
        filep1->pipe->write_pos=0;
        filep1->pipe->pipe_buff = NULL;
        filep1->pipe->buffer_offset =0;
        filep1->pipe->is_ropen = 1;
        filep1->pipe->is_wopen = 1;

        newfd = 0;
        while(current->files[newfd])
             newfd++; 
	if(newfd>=32) return -1; 
        current->files[newfd] = filep1;
        fd[0]=newfd;

//write port 
        
        filep2->type = REGULAR;
        filep2->mode = O_WRITE;
        filep2->offp = 0;
        filep2->ref_count = 1;
        filep2->inode = NULL;

        filep2->fops->read = pipe_read;
        filep2->fops->write = pipe_write;
        filep2->fops->close = generic_close;

        filep2->pipe = filep1->pipe;
	

        newfd = 0;
        while(current->files[newfd])
             newfd++;  
	if(newfd>=32) return -1;
        current->files[newfd] = filep2;
        fd[1]=newfd;
        return 0;

    
}

