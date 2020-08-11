//Dewansh Kr Singh
//170241

#include<stdio.h>
#include <dirent.h>
#include<unistd.h>  
#include<string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc , char ** argv)
{
    if(!strcmp(argv[1],"@"))
    {
        int fd[2];
        pid_t p;
        //handling pipe error
        if (pipe(fd)==-1) 
            { 
                fprintf(stderr,"Pipe Failed"); 
                return 1; 
            } 
        p=fork();
        //fork error handling
        if (p < 0) 
        { 
            fprintf(stderr, "fork Failed" ); 
            return 1; 
        } 
        //parent process
        if(p>0)
        {
            close(fd[0]);
            dup2(fd[1],1);
            char *args[]={"grep","-rF",argv[2],argv[3],NULL};
            execvp(args[0],args);
        }
        //child process
        else
        {
            close(fd[1]);
            dup2(fd[0],0);
            char *arg[]={"wc","-l",NULL};
            execvp(arg[0],arg);
        }
        
        
    }
    else if(!strcmp(argv[1],"$"))
    {
        int fd1[2];
        int fd2[2];
        pid_t p;
        //pipe1 error handling
        if (pipe(fd1)==-1) 
        { 
            fprintf(stderr, "Pipe Failed" ); 
            return 1; 
        } 
        //pipe2 error handling
        if (pipe(fd2)==-1) 
        { 
            fprintf(stderr, "Pipe Failed" ); 
            return 1; 
        } 
        p=fork();

        //fork error handling
        if (p < 0) 
        { 
            fprintf(stderr, "fork Failed" ); 
            return 1; 
        } 

        //parent process
        if(p>0)
        {
           
            close(fd1[0]);
            dup2(fd1[1],1);
            char *args[]={"grep","-rF",argv[2],argv[3],NULL};
            execvp(args[0],args);
        }
        //child process
        else
        {
            
            close(fd1[1]);
            pid_t f;
            f=fork();
            //fork error handling
            if (f < 0) 
            { 
                fprintf(stderr, "fork Failed" ); 
                return 1; 
            } 
            //childs parent 
            if(f>0)
            {
                
                close(fd2[0]);
                int f = open(argv[4],O_CREAT | O_TRUNC | O_RDWR, 0700);
                char buff[1];
                while(read(fd1[0],buff,1))
                {
                    write(f,buff,1);
                    write(fd2[1],buff,1);
                    memset(buff, 0, 1);
                }
                
            }
            //childs child
            else
            {
                
                close(fd2[1]);
                dup2(fd2[0],0);
                char *args[0];
                //storing the command to be executed in last in args string
                for(int i = 5;i<argc;i++)
                {
                    args[i-5]= argv[i];
                }
                args[argc-5]=NULL;
                execvp(args[0],args);
                
                
            }
            
        }
    }
    
    else
    {
        printf("unknown command");
    }
    return 0;
}
