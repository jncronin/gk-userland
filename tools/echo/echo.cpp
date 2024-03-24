#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    auto fw = open("/dev/ttyUSB0", O_WRONLY);
    if(!fw)
    {
        printf("Unable to fopen /dev/ttyUSB0 for writing\n");
        return -1;
    }
    auto fr = open("/dev/ttyUSB0", O_RDONLY);
    if(!fr)
    {
        printf("Unable to fopen /dev/ttyUSB0 for reading\n");
        return -1;
    }

    while(true)
    {
        char buf[512];
        auto br = read(fr, buf, 512);
        if(br > 0)
        {
            write(fw, buf, br);
        }
        else if(br < 0)
        {
            printf("fread returned %d, errno %d\n", br, errno);
        }
    }

    close(fw);
    close(fr);

    return 0;
}