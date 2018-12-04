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

    int mode = atoi(argv[2]);
    unsigned long pid = atoi(argv[1]);
    long sys = syscall(359+mode, pid, ary, SIZE);
    printf("test of system call 360, return : %ld\n", sys);

    for ( i=0 ; i < SIZE ; i=i+4 )
    {
        if ( ary[i] != 0 && ary[i+1] != 0 )
            printf("vaddr[%lX | %lX]", ary[i], ary[i+1]);
        else
            continue;

        if ( ary[i+2] == -1 )
            printf("  paddr{ --- |");
        else
            printf("  paddr{%lX |", ary[i+2]);

        if ( ary[i+3] == -1 )
            printf(" --- }\n");
        else
            printf(" %lX}\n", ary[i+3]);
    }

    return 0;
 }
