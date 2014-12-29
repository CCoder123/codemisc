#include<iostream>
#include "UtilPdu.h"
using namespace std;

int main()
{
    char buf[1024];
    memset(buf, 0, 1024);

    CByteStream bstream(buf, 1024);
    //bstream.WriteInt16();

    return 0;
}
