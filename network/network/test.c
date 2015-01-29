#include <stdio.h>
#include "anet.h"
#include "ae.h"

void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask) 
{
    printf("got a connect..........\n");
    int cport, cfd;
    char cip[128];
    char neterr[ANET_ERR_LEN];
    cfd = anetTcpAccept(neterr, fd, cip, sizeof(cip), &cport);
    if (cfd == AE_ERR) { return; }
}

int main()
{
    aeEventLoop *loop = aeCreateEventLoop(1024);

    char neterr[128] = {0};
    int fd = anetTcpServer(neterr, 8088, "127.0.0.1");
    if(fd < 0)
    {
        return -1;
    }
    aeCreateFileEvent(loop, fd, AE_READABLE, acceptTcpHandler, NULL);
    aeMain(loop);
    return 0;
}
