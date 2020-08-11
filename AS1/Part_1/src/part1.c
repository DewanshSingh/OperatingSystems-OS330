//Dewansh Kr Singh
//170241
#include<stdio.h>
#include <dirent.h>
#include<unistd.h>  
#include<string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>


//recursively searches through all the files in the path "bp" for "search" keyword
void printdirmap(char * bp,char * search)
{
    char p[1000];
    struct dirent *de;
    DIR * dr = opendir(bp);
    //if its a null direcrtory then simply return 
    if(dr == NULL) return ;
    
    while((de=readdir(dr))!=NULL)
    {
        //excluding the . amd .. directory links 
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            //creating and storing the new path info in p
            strcpy(p, bp);
            int len = strlen(bp);            
            if( p[len-1] != '/' )
                strcat(p, "/");
            
            strcat(p, de->d_name);
            
            struct stat buf;
            stat(p, &buf);
            //if its a reg file we search fot keyword
            if(S_ISREG(buf.st_mode))
            {
                char c[1];
                int fd = open(p,O_RDWR);
                c[0] = 'a';
                
                while(read(fd,c,1)!=0)
                {
                    int i=0;
                    char length[100000];
                    length[i]=c[0];
                    i++;
                    while(c[0]!='\n' && read(fd,c,1)!=0)
                    {
                        length[i]=c[0];
                        i++;
                    }
                    length[i]=0;
                    if(strstr(length,search))
                    {
                  
                        printf("%s:",p);
                     
                        printf("%s",length);
                        
                    }
                }
                
            }
            //if its a directory then recusrsively call the function with new path p
            else if(S_ISDIR(buf.st_mode))
                printdirmap(p,search);
        }
        
    }
    
    closedir(dr);
}

int main(int argc , char ** argv)
{
    if(argc!=3)
    {
        printf("ERROR:Takes only two Arguments");
        return 0;
    }
    
    struct stat buf;
    stat(argv[2], &buf);
    //if given path not a directory but a reg file 
    if(S_ISREG(buf.st_mode))
    {
        char c[1];
        int fd = open(argv[2],O_RDWR);
        c[0] = 'a';
        
        while(read(fd,c,1)!=0)
        {
            int i=0;
            char length[100000];
            length[i]=c[0];
            i++;
            while(c[0]!='\n' && read(fd,c,1)!=0)
            {
                length[i]=c[0];
                i++;
            }
            length[i]=0;
            if(strstr(length,argv[1]))
            {
                
                printf("%s:",argv[2]);
                printf("%s",length);
                
            }
        }
        
    }

    //if given path is a directory
    else if(S_ISDIR(buf.st_mode))
        printdirmap(argv[2],argv[1]);  
    
    return 0;
}

