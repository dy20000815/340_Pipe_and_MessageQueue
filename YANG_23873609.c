//David Yang 23873609

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <time.h>
#include <sys/utsname.h>

int countLine(char line[], int size) //method of counting lines
{
    int count=0;
    for(int i=0; i<size; i++)
    {
        if (line[i]=='\n') //reach '\n' means one line ends
        {
            count++;
        }
    }
    return count;
}

int countWords(char line[], int size) //method of counting words
{
    int count=0;
    for(int i=0; i<size; i++)
    {
        if (line[i]==' '||line[i]=='\n') //reach '\n' or a whitespace means one word ends
        {
            count++;
        }
    }
    return count;
}

int countChars(char line[], int size)
{
    int count=0;
    for(int i=0; i<size; i++)
    {
        if ((line[i]>='a'&&line[i]<='z')||(line[i]>='A'&&line[i]<='Z'))
        {
            count++;
        }
    }
    return count;
}

int max(int a,int b) //finding max of 2 int
{
    if(a>=b)
    {
        return a;
    }
    else
    {
        return b;
    }
}


int main(int argc, char* argv[])
{


    if(argc<3) //at least there are buffer size and one input file.
    {
        printf("does not have enough argument!");
        return 1;
    }
    int option,buffer,index;//argv[index] of input files.
    char* inputF;
    if (strcmp(argv[1],"-n")==0) //new line count
    {
        option=1;
        index=3;
        buffer=atoi(argv[2]);
    }
    else if (strcmp(argv[1],"-c")==0)  //word counts
    {
        option=2;
        index=3;
        buffer=atoi(argv[2]);
    }
    else if (strcmp(argv[1],"-b")==0)  //character counts
    {
        option=3;
        index=3;
        buffer=atoi(argv[2]);
    }
    else if (strcmp(argv[1],"-l")==0)  //maximum line length.
    {
        option=4;
        index=3;
        buffer=atoi(argv[2]);
    }
    else if (strcmp(argv[1],"-ncbl")==0)  //-ncbl default, if there is no "-" argument.
    {
        option=0;
        buffer=atoi(argv[2]);
        index=3;
    }
    for(int i=index; i<argc; i++) //for loop to each out put file;
    {
        inputF=argv[index];
        struct mq_attr attr;
        attr.mq_maxmsg=50;
        attr.mq_msgsize=buffer;
        //creating pipe.
        int n,fd[2];
        char buf[buffer];
        pipe(fd);
        // Initialize the message queue.
        mqd_t mqd = mq_open("/msg_davidYang",  O_CREAT | O_RDWR, 0644, &attr);
        pid_t pid = fork();
        //check error.
        if(pid<0)
        {
            write(STDOUT_FILENO,"fork() fails\n",12);
            return 2;
        }
        else if(pid>0) //parent process
        {
            struct utsname os;//operating system info.
            if(uname(&os)<0)
            {
                printf("error for uname()");
                exit(-1);
            }
            printf("OS name is:\t %s \n",os.sysname);
            printf("OS version is:\t %s \n",os.version);
            printf("OS release is:\t %s \n",os.release);
            pid_t pid1=getpid();//pid
            printf("process id is:\t %ld\n", (long) pid1);
            pid_t pid2=getppid();//ppid
            printf("Parent process id is: \t %ld\n", (long) pid2);
            char cwd[128];//cwd
            if(getcwd(cwd,sizeof(cwd))!=NULL)
            {
                printf("Process current working directory is: \t %s\n", cwd);
            }
            char hostName[50];//hostname
            gethostname(hostName,sizeof(hostName));
            printf("Hostname is: \t %s\n", cwd);

            //part 1:
            close(fd[0]);//close read end of pipe.
            int f=open(inputF,O_RDONLY, 0);
            while ((n = read( f, buf, buffer)) > 0) //keep writing until file is empty
            {
                if (write(fd[1], buf, n) != n)
                {
                    write(STDOUT_FILENO,"Can't write file\n",17 );
                    close(f);
                    return 4;
                }
            }
            close(f);//close file reader
            char buff[128];
            struct timespec timeout = {0, 0};
            unsigned int prio;
            while(1) //receive message from mq until it's empty
            {
                ssize_t numRead = mq_timedreceive(mqd, buff, sizeof(buff), &prio, &timeout);
                if (numRead==0||numRead==-1)
                {
                    break;
                }
                else
                {
                    write(STDOUT_FILENO, buff,128);
                }
            }

            write(STDOUT_FILENO,"Parent terminating.\n",20);
            return 0;

        }
        //child process.
        else if(pid==0)
        {
            pid_t pid1=getpid();
            printf("process id is:\t %ld\n", (long) pid1);
            pid_t pid2=getppid();
            printf("Parent process id is: \t %ld\n", (long) pid2);
            char cwd[128];
            if(getcwd(cwd,sizeof(cwd))!=NULL)
            {
                printf("Process current working directory is: \t %s\n", cwd);
            }
            char hostName[50];
            gethostname(hostName,sizeof(hostName));
            printf("Hostname is: \t %s\n", cwd);
            close(fd[1]);//close write end of pipe
            char line[buffer];
            char ans[128];
            struct timespec timeout = {0, 0};
            int ret=0, count=0;
            switch(option)
            {
            case 0:
                int lines=0, words=0, chars=0, maxLength=0;
                while (1) //keep reading from pipe
                {
                    n=read(fd[0],line, buffer);
                    if(n==-1||n==0) //break if no character read or error occurs.
                    {
                        break;
                    }
                    else
                    {
                        lines+=countLine(line,n);//lines count
                        words+=countWords(line,n);//words count
                        chars+=countChars(line,n);//chars count
                        for(int i=0; i<n; i++) //max length
                        {
                            if(line[i]!='\n')
                            {
                                count++;
                            }
                            else
                            {
                                maxLength=max(maxLength, count);
                                count=0;
                            }
                        }
                    }
                }
                char ans1[128],ans2[128],ans3[128],ans4[128];//sending msg to queue
                snprintf(ans1,128,"%s:\t new line count is: \t %i \n",inputF, lines);
                if (mq_timedsend(mqd, ans1, sizeof(ans1), 1, &timeout) == -1)
                {
                    write(STDOUT_FILENO,"Child: mq_send error\n",22);
                    return 4;
                }
                snprintf(ans2,128,"%s:\t word count is: \t %i \n",inputF, words);

                if (mq_timedsend(mqd, ans2, sizeof(ans2), 1, &timeout) == -1)
                {
                    write(STDOUT_FILENO,"Child: mq_send error\n",22);
                    return 4;
                }
                snprintf(ans3,128,"%s:\t character count is: \t %i \n",inputF, chars);
                if (mq_timedsend(mqd, ans3, sizeof(ans3), 1, &timeout) == -1)
                {
                    write(STDOUT_FILENO,"Child: mq_send error\n",22);
                    return 4;
                }
                snprintf(ans4,128,"%s:\t maximum line length count is: \t %i \n",inputF, maxLength);
                if (mq_timedsend(mqd, ans4, sizeof(ans4), 1, &timeout) == -1)
                {
                    write(STDOUT_FILENO,"Child: mq_send error\n",22);
                    return 4;
                }
                break;
            case 1://line count
                while (1) //keep reading from pipe
                {
                    n=read(fd[0],line, buffer);
                    if(n==-1||n==0) //break if no character read or error occurs.
                    {
                        break;
                    }
                    else
                    {
                        ret+=countLine(line,n);
                    }
                }
                snprintf(ans,128,"%s:\t new line count is: \t %i \n",inputF, ret);
                if (mq_timedsend(mqd, ans, sizeof(ans), 1, &timeout) == -1)
                {
                    write(STDOUT_FILENO,"Child: mq_send error",21);
                    return 4;
                }
                break;
            case 2://word count
                while (1) //keep reading from pipe
                {
                    n=read(fd[0],line, buffer);
                    if(n==-1||n==0) //break if no character read or error occurs.
                    {
                        break;
                    }
                    else
                    {
                        ret+=countWords(line,n);
                    }
                }
                snprintf(ans,128,"%s:\t word count is: \t %i \n",inputF, ret);
                if (mq_timedsend(mqd, ans, sizeof(ans), 1, &timeout) == -1)
                {
                    write(STDOUT_FILENO,"Child: mq_send error",21);
                    return 4;
                }
                break;
            case 3://character count
                while (1) //keep reading from pipe
                {
                    n=read(fd[0],line, buffer);
                    if(n==-1||n==0) //break if no character read or error occurs.
                    {
                        break;
                    }
                    else
                    {
                        ret+=countChars(line,n);
                    }
                }
                snprintf(ans,128,"%s:\t character count is: \t %i \n",inputF, ret);
                if (mq_timedsend(mqd, ans, sizeof(ans), 1, &timeout) == -1)
                {
                    write(STDOUT_FILENO,"Child: mq_send error",21);
                    return 4;
                }
                break;
            case 4:// max line length
                while (1) //keep reading from pipe
                {
                    n=read(fd[0],line, buffer);
                    if(n==-1||n==0) //break if no character read or error occurs.
                    {
                        break;
                    }
                    else
                    {
                        for(int i=0; i<n; i++)
                        {
                            if(line[i]!='\n')
                            {
                                count++;
                            }
                            else
                            {
                                ret=max(ret, count);
                                count=0;
                            }
                        }

                    }
                }
                //sending
                snprintf(ans,128,"%s:\t maximum line length count is: \t %i \n",inputF, ret);
                if (mq_timedsend(mqd, ans, sizeof(ans), 1, &timeout) == -1)
                {
                    write(STDOUT_FILENO,"Child: mq_send error",21);
                    return 4;
                }
                break;
            }
            write(STDOUT_FILENO,"Child terminating.\n",19);
            exit(0);
            return 0;

        }
    }
}

