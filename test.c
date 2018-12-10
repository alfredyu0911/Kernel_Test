#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#define SIZE 10000

int main(int argc, char *argv[])
{
    if ( argc != 3 )
        return -1;

    unsigned long ary[SIZE];
    unsigned int i=0;
    for ( i=0 ; i < SIZE ; i++ )
        ary[i] = 0;

    int callId = atoi(argv[2]) + 359;
    unsigned long pid = atoi(argv[1]);
    long sys = syscall(callId, pid, ary, SIZE);
    printf("test of system call 360, return : %ld\n", sys);

    for ( i=0 ; i < SIZE ; i=i+5 )
    {
        if ( ary[i] != 0 && ary[i+1] != 0 )
            printf("vaddr[%8lX | %8lX]", ary[i], ary[i+1]);
        else
            continue;

        if ( ary[i+2] == -1 )
            printf("  paddr{ ------- |");
        else
            printf("  paddr{%8lX |", ary[i+2]);

        if ( ary[i+3] == -1 )
            printf(" ------- }");
        else
            printf(" %8lX}", ary[i+3]);

        unsigned long unit = 0x1000;
        unsigned long pageCount = (ary[i+1] - ary[i]) / unit;
        double ratio = ((double)ary[i+4] / (double)pageCount) * 100.0;
        printf("  ( %5.2f%% )\n", ratio);
    }

    return 0;
 }
