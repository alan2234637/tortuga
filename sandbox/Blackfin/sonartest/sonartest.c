#include <stdio.h>

int main(int argc, char ** argv)
{
    signed short val;
    unsigned long addr;

    if(argc != 2)
    {
        printf("sonartest addr\n");
        printf("all values are in hex. addr is 32bit.\n");
        return -1;
    }

    addr = strtol(argv[1], NULL, 16);

    unsigned short lastCount = 0;

    while(1)
    {
        val = *((signed short *) (addr));
	    printf("%06d\n", val);
	    fflush(stdout);
        while(lastCount == *((unsigned short *)0x202F0020));
        lastCount = *((unsigned short *)0x202F0020);
    }
    return 0; // hi joe
}
