#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#define SIZE 10000
#define SYSTEM_CALL_ID 359

void linux_survyTT(unsigned long pid, unsigned long *ary)
{
    syscall(SYSTEM_CALL_ID, pid, ary, SIZE);

    unsigned long totalPages = 0;
    unsigned long totalPresentedCount = 0;
    unsigned int i=0;
    for ( i=0 ; i < SIZE ; i=i+5 )
    {
        if ( ary[i] != 0 && ary[i+1] != 0 )
            printf("vaddr[ %8lX | %8lX ]", ary[i], ary[i+1]);
        else
            continue;

        if ( ary[i+2] == -1 )
            printf("  paddr{ -------- |");
        else
            printf("  paddr{ %8lX |", ary[i+2]);

        if ( ary[i+3] == -1 )
            printf(" -------- }");
        else
            printf(" %8lX }", ary[i+3]);

        unsigned long unit = 0x1000;
        unsigned long pageCount = (ary[i+1] - ary[i]) / unit;
        double ratio = ((double)ary[i+4] / (double)pageCount) * 100.0;
        printf("  ( %3lu, %3lu, %6.2f%% )\n", ary[i+4], pageCount, ratio);

        totalPages += pageCount;
        totalPresentedCount += ary[i+4];
    }
    double ratio = (double)totalPresentedCount / (double)totalPages * 100.0;
    printf("presented pages: %3lu    total pages: %3lu\n", totalPresentedCount, totalPages);
    printf("system allocate %6.2f%% of memory pages\n", ratio);
}

int main(int argc, char *argv[])
{
    if ( argc != 2 )
        return -1;

    unsigned long ary1[SIZE];
    unsigned long ary2[SIZE];
    unsigned int i, j;
    for ( i=0 ; i < SIZE ; i++ )
    {
        ary1[i] = 0;
        ary2[i] = 0;
    }

    unsigned long pid;
    printf("please input the id of process 1 : ");
    scanf("%lu", &pid);
    linux_survyTT(pid, ary1);

    printf("please input the id of process 2 : ");
    scanf("%lu", &pid);
    linux_survyTT(pid, ary2);

    printf("\n\nshare memory result : \n");
    for ( i=0 ; i < SIZE ; i=i+5 )
    {
        if ( ary1[i] == 0 || ary1[i+1] == 0 || ary1[i] == -1 || ary1[i+1] == -1 )
            continue;
            
        for ( j=0 ; j < SIZE ; j=j+5 )
        {
            if ( ary2[j] == 0 || ary2[j+1] == 0 || ary2[j] == -1 || ary2[j+1] == -1 )
            continue;

            // found the shared virtual memory interval
            if ( ary1[i] == ary2[j] && ary1[i+1] == ary2[j+1] )
            {
                printf("[ %8lX | %8lX ]", ary1[i], ary1[i+1]);
                break;
            }
        }
    }

    return 0;
 }
