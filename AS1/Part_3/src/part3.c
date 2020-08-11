//Dewansh Kr Singh
//170241

/*Implementation details 

in the main function , first the root dir is opened and readdir is applied untill its doen .
for each readdir if its a directory then forking is done else the size of file is added to final size of root dir
inside the child process a recursive fun is callled to get dir size and in the parent it reads this size and outputs 

*/





#include<stdio.h>
#include <dirent.h>
#include<unistd.h>  
#include<string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

//function dirsize takes in a directory path travereses through it recursively and and return its size

int dirsize(char * bp)
{
    int size = 0;
    char p[1000];
    struct dirent *de;
    DIR * dr = opendir(bp);
    //return the size if dr is null
    if(dr == NULL) return size;
    
    while((de=readdir(dr))!=NULL)
    {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            //creating the new path and storing it in p
            strcpy(p, bp);
            int len = strlen(bp);
            // printf("%c\n\n",p[len-1]);
            if( p[len-1] != '/' )
                strcat(p, "/");
            
            strcat(p, de->d_name);
            
            struct stat buf;
            stat(p, &buf);
            
            //if p is a dir then recurse 
            if(S_ISDIR(buf.st_mode))
                size += dirsize(p);
            else
            {
               //update size
                size=size+buf.st_size;
            }
        }
        
    }
    
    closedir(dr);
    return size;
}

int main(int argc , char ** argv)
{
    if(argc!=2)
    {
        printf("ERROR:Takes only two Arguments");
        return 0;
    }
    
    struct stat buf;
    stat(argv[1], &buf);
    int sum=0;

    //extracting the root directory name from path given
    

    const char * rootdir = strrchr(argv[1], '/');
    if(rootdir==NULL)
        rootdir=argv[1]-1;

    //if path is a directory 
    if(S_ISDIR(buf.st_mode))
    {
        
        char p[1000];
        struct dirent *de;
        DIR * dr = opendir(argv[1]);
        if(dr == NULL) return 0;
        while((de=readdir(dr))!=NULL)
        {
            if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
            {
                //storing new path in p
                p[0]=0;
                strcpy(p, argv[1]);
                int len = strlen(argv[1]);
                if( p[len-1] != '/' )
                    strcat(p, "/");
                
                strcat(p, de->d_name);

                //if sub dir is found
                if(S_ISDIR(buf.st_mode))
                {
                    
                    int fd[2];
                    pid_t f;
                    //handling pipe error
                    if (pipe(fd)==-1) 
                        { 
                            fprintf(stderr,"Pipe Failed"); 
                            return 1; 
                        } 
                    //this forking happens immediate subdir times to root dirc
                    f=fork();
                    //fork error handling
                    if (f < 0) 
                    { 
                        fprintf(stderr, "fork Failed" ); 
                        return 1; 
                    } 
                    
                    //parent process
                    if(f>0)
                    {
                        close(fd[1]);
                        char buf[18];
                        int tmp;
                        read(fd[0],&tmp,sizeof(tmp));
                        sum=sum+tmp;
                        printf("%s %d\n",de->d_name,tmp);
                    }
                    //child process
                    else
                    {
                        int size = 0;
                        size = dirsize(p);
                        close(fd[0]);
                        write(fd[1],&size,sizeof(size));
                        exit(0);
                    }
                }
                //if its a reg file
                //gets the sum of sizes of files in root dir
                else
                {
                    struct stat buf;
                    stat(p, &buf);
                    sum = sum + buf.st_size;
                }
                
            }
        }
        
        printf("%s %d\n",rootdir+1,sum);
    }
    
    
    return 0;
}

