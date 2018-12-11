#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BYTES_PER_PAGE 0x1000
#define MEMORY_SIZE 1000000
#define SYSTEM_CALL_ID_PART_I 359
#define SYSTEM_CALL_ID_PART_II 360

unsigned long result_1[MEMORY_SIZE];
unsigned long result_2[MEMORY_SIZE];

void linux_survey_TT(unsigned long pid, unsigned long *ary);
void linux_survey_VV(unsigned long *ary);

void project_Part_I();
int project_Part_II();

/* this function is run by the second thread */
void *inc_x(void *x_void_ptr)
{
    /* increment input parameter to 100 */
    int *x_ptr = (int *)x_void_ptr; 
    while ( ++(*x_ptr) < 100 );

    printf("x increment finished\n");
    linux_survey_VV(result_1);
    
    return NULL;
}

void linux_survey_TT(unsigned long pid, unsigned long *ary)
{
    unsigned long ret = syscall(SYSTEM_CALL_ID_PART_I, pid, ary, MEMORY_SIZE);
    printf("calling system call %d, return : %lu", SYSTEM_CALL_ID_PART_I, ret);

    unsigned long totalPages = 0;
    unsigned long totalPresentedCount = 0;
    unsigned int i=0;
    for ( i=0 ; i < MEMORY_SIZE ; i=i+5 )
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

        unsigned long pageCount = (ary[i+1] - ary[i]) / BYTES_PER_PAGE;
        double ratio = ((double)ary[i+4] / (double)pageCount) * 100.0;
        printf("  ( %3lu, %3lu, %6.2f%% )\n", ary[i+4], pageCount, ratio);

        totalPages += pageCount;
        totalPresentedCount += ary[i+4];
    }
    double ratio = (double)totalPresentedCount / (double)totalPages * 100.0;
    printf("presented pages: %3lu    total pages: %3lu\n", totalPresentedCount, totalPages);
    printf("system allocate %6.2f%% of memory pages\n", ratio);
}

void linux_survey_VV(unsigned long *ary)
{
    unsigned long ret = syscall(SYSTEM_CALL_ID_PART_II, ary, MEMORY_SIZE);
    printf("calling system call %d, return : %lu", SYSTEM_CALL_ID_PART_II, ret);

    unsigned long totalPages = 0;
    unsigned long totalPresentedCount = 0;
    unsigned int i=0;
    for ( i=0 ; i < MEMORY_SIZE ; i=i+5 )
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

        unsigned long pageCount = (ary[i+1] - ary[i]) / BYTES_PER_PAGE;
        double ratio = ((double)ary[i+4] / (double)pageCount) * 100.0;
        printf("  ( %3lu, %3lu, %6.2f%% )\n", ary[i+4], pageCount, ratio);

        totalPages += pageCount;
        totalPresentedCount += ary[i+4];
    }
    double ratio = (double)totalPresentedCount / (double)totalPages * 100.0;
    printf("presented pages: %3lu    total pages: %3lu\n", totalPresentedCount, totalPages);
    printf("system allocate %6.2f%% of memory pages\n", ratio);
}

void project_Part_I()
{
    memset(result_1, 0x0, MEMORY_SIZE);
    memset(result_2, 0x0, MEMORY_SIZE);
    unsigned int i, j;

    unsigned long pid;
    printf("please input the id of process 1 : ");
    scanf("%lu", &pid);
    linux_survey_TT(pid, result_1);

    printf("please input the id of process 2 : ");
    scanf("%lu", &pid);
    linux_survey_TT(pid, result_2);

    printf(" ------------------------------------- \n");
    printf("\nshared memory result : \n");
    for ( i=0 ; i < MEMORY_SIZE ; i=i+5 )
    {
        if ( result_1[i] == 0 || result_1[i+1] == 0 || result_1[i] == -1 || result_1[i+1] == -1 )
            continue;
            
        for ( j=0 ; j < MEMORY_SIZE ; j=j+5 )
        {
            if ( result_2[j] == 0 || result_2[j+1] == 0 || result_2[j] == -1 || result_2[j+1] == -1 )
                continue;

            // found the shared virtual memory interval
            if ( result_1[i+2] == result_2[j+2] && result_1[i+3] == result_2[j+3] )
            {
                printf("[ %8lX | %8lX ]\n", result_1[i+2], result_1[i+3]);
                break;
            }
        }
    }
}

int project_Part_II()
{
    int x = 0, y = 0;
    printf("x: %d, y: %d\n", x, y);    /* show the initial values of x and y */

    /* this variable is our reference to the second thread */
    pthread_t inc_x_thread;

    /* create a second thread which executes inc_x(&x) */
    if ( pthread_create(&inc_x_thread, NULL, inc_x, &x) )
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    
    /* increment y to 100 in the first thread */
    while ( ++y < 100 );

    printf("y increment finished\n");
    linux_survey_VV(result_2);

    /* wait for the second thread to finish */
    if ( pthread_join(inc_x_thread, NULL) )
    {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    /* show the results - x is now 100 thanks to the second thread */
    printf("x: %d, y: %d\n", x, y);

    // Code to process and report the final results

    return 0;
}

int main(int argc, char *argv[])
{
    if ( argc != 2 )
        return -1;

    int projectPart = atoi(argv[1]);

    if ( projectPart == 1 )
        project_Part_I();
    else if ( projectPart == 2 )
        project_Part_II();
    
    return 0;
 }
