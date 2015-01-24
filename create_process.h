#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

int createProcess(int n)
{
    if(n > 0)
    {
        int bIsOK = 1;
        pid_t pid = -1;

        while(bIsOK == 1)
        {
            if(n > 0)
            {
                pid = fork();
                if(pid > 0)
                {
                    bIsOK = 1;
                    n--;
                }
                else if(pid == 0)
                {
                    bIsOK = 0;
                    fprintf(stderr, "create process %d %d ok.\n", getpid(), n);
                }
                else
                {
                    fprintf(stderr, "create process error.\n");
                    return -1;
                }
            }
            else  // n<=0
            {
                int status;
                if(wait(&status) == -1)
                {
                    fprintf(stderr, "wait error %s\n", strerror(errno));
                    n++;
                }
            }
        }
    }

    return 0;
}

#endif
