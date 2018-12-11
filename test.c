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

#define ARRAY_EMPTY 0
#define PAGE_NOT_PRESENTED -1

unsigned long result_1[MEMORY_SIZE];
unsigned long result_2[MEMORY_SIZE];

void show_linux_survey_result(unsigned long *ary);
void search_and_show_SharedInterval();

void linux_survey_TT(unsigned long pid, unsigned long *ary);
void linux_survey_VV(unsigned long *ary);

void project_Part_I();
int project_Part_II();

/* this function is run by the second thread */
void *inc_x(void *x_void_ptr)
{
    /* increment input parameter to 100 */
    int *x_ptr = (int *)x_void_ptr;
    while (++(*x_ptr) < 100)
        ;

    printf("x increment finished\n");
    linux_survey_VV(result_1);

    return NULL;
}

/**
 * memory layout
 * 將長度為 N 的陣列切為兩份
 * 前四分之一 0 ~ (N/4 - 1) : 儲存 vma 提供的位址區間、以及其對應的相關資訊
 * 後四分之三 N/4 ~ N-1 : 儲存以 page size 為單位的所有區間、以及其對應的相關資訊
*/
void show_linux_survey_result(unsigned long *ary)
{
    unsigned long totalPages = 0;
    unsigned long totalPresentedCount = 0;
    unsigned int i = 0;

    // only looking for first 1/4 datas
    for ( i = 0 ; i < MEMORY_SIZE/4 ; i = i + 5 )
    {
        if ( ary[i] != 0 && ary[i+1] != 0 )
            printf("vaddr[ 0x%08lX | 0x%08lX ]", ary[i], ary[i+1]);
        else
            continue;

        if ( ary[i+2] == PAGE_NOT_PRESENTED )
            printf("  paddr{ ---------- |");
        else
            printf("  paddr{ 0x%08lX |", ary[i+2]);

        if ( ary[i+3] == PAGE_NOT_PRESENTED )
            printf(" ---------- }");
        else
            printf(" 0x%08lX }", ary[i+3]);

        unsigned long pageCount = (ary[i+1] - ary[i]) / BYTES_PER_PAGE;
        double ratio = ((double)ary[i+4] / (double)pageCount) * 100.0;
        printf("  ( %4lu, %4lu, %6.2f%% )\n", ary[i+4], pageCount, ratio);

        totalPages += pageCount;
        totalPresentedCount += ary[i+4];
    }
    double ratio = (double)totalPresentedCount / (double)totalPages * 100.0;
    printf("presented pages: %3lu    total pages: %3lu\n", totalPresentedCount, totalPages);
    printf("system allocate %6.2f%% of memory pages\n", ratio);
}

void linux_survey_TT(unsigned long pid, unsigned long *ary)
{
    unsigned long ret = syscall(SYSTEM_CALL_ID_PART_I, pid, ary, MEMORY_SIZE);
    printf("calling system call %d, return : %lu\n", SYSTEM_CALL_ID_PART_I, ret);

    show_linux_survey_result(ary);
}

void linux_survey_VV(unsigned long *ary)
{
    unsigned long ret = syscall(SYSTEM_CALL_ID_PART_II, ary, MEMORY_SIZE);
    printf("calling system call %d, return : %lu\n", SYSTEM_CALL_ID_PART_II, ret);
}

/**
 * memory layout
 * 將長度為 N 的陣列切為兩份
 * 前四分之一 0 ~ (N/4 - 1) : 儲存 vma 提供的位址區間、以及其對應的相關資訊
 * 後四分之三 N/4 ~ N-1 : 儲存以 page size 為單位的所有區間、以及其對應的相關資訊
*/
void search_and_show_SharedInterval()
{
    // 因輸出結果資料量較大，這邊以寫檔的方式輸出.
    FILE *fout;
    fout=fopen("res.txt","w+t");
    if ( !fout )
    {
        printf("fopen fail\n");
        return;
    }

    int count = 0;

    fprintf(fout, " ------------------------------------- \n");
    fprintf(fout, "\nshared memory result : \n");
    int i, j;
    for ( i = (MEMORY_SIZE/4) ; i < MEMORY_SIZE ; i = i+5 )
    {
        unsigned long v_addr_1_start = result_1[i];
        unsigned long p_addr_1_start = result_1[i+2];
        if ( v_addr_1_start == ARRAY_EMPTY || p_addr_1_start == PAGE_NOT_PRESENTED )
            continue;

        for ( j = (MEMORY_SIZE/4) ; j < MEMORY_SIZE ; j = j+5 )
        {
            unsigned long v_addr_2_start = result_2[j];
            unsigned long p_addr_2_start = result_2[j+2];
            if ( v_addr_2_start == ARRAY_EMPTY || p_addr_2_start == PAGE_NOT_PRESENTED )
                continue;

            // search if the physical address is match
            if ( p_addr_1_start == p_addr_2_start )
            {
                fprintf(fout, "[ 0x%08lX ] & ", v_addr_1_start);
                fprintf(fout, "[ 0x%08lX ] share page ", v_addr_2_start);
                fprintf(fout, "[ 0x%08lX ]\n", p_addr_1_start);
                count++;
                break;
            }
        }
    }

    int totalCount1=0, totalCount2=0;
    int totalPresentedCount1=0, totalPresentedCount2=0;
    for ( i = 0 ; i < (MEMORY_SIZE/4) ; i = i+5 )
    {
        if ( result_1[i] != ARRAY_EMPTY && result_1[i+1] != ARRAY_EMPTY )
        {
            totalCount1 += ((result_1[i+1] - result_1[i]) / BYTES_PER_PAGE);
            totalPresentedCount1 += result_1[i+4];
        }
        if ( result_2[i] != ARRAY_EMPTY && result_2[i+1] != ARRAY_EMPTY )
        {
            totalCount2 += ((result_2[i+1] - result_2[i]) / BYTES_PER_PAGE);
            totalPresentedCount2 += result_2[i+4];
        }
    }

    double per1 = (double)count / (double)totalCount1 * 100.0;
    double per2 = (double)count / (double)totalCount2 * 100.0;
    printf("total %d of pages are shared.\n(%6.2f%%/%6.2f%% of all pages)\n", count, per1, per2);
    per1 = (double)count / (double)totalPresentedCount1 * 100.0;
    per2 = (double)count / (double)totalPresentedCount1 * 100.0;
    printf("(%6.2f%%/%6.2f%% of presented pages)\n", per1, per2);
}

void project_Part_I()
{
    unsigned int i, j;

    unsigned long pid;
    printf("please input the id of process 1 : ");
    scanf("%lu", &pid);
    linux_survey_TT(pid, result_1);

    printf("please input the id of process 2 : ");
    scanf("%lu", &pid);
    linux_survey_TT(pid, result_2);

    search_and_show_SharedInterval();
}

int project_Part_II()
{
    int x = 0, y = 0;
    printf("x: %d, y: %d\n", x, y); /* show the initial values of x and y */

    /* this variable is our reference to the second thread */
    pthread_t inc_x_thread;

    /* create a second thread which executes inc_x(&x) */
    if ( pthread_create(&inc_x_thread, NULL, inc_x, &x) )
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    /* increment y to 100 in the first thread */
    while (++y < 100)
        ;

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
    // Part II doesn't have to show the shared physical address
    printf(" Part II result of created thread : \n");
    show_linux_survey_result(result_1);
    printf(" +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- \n");
    printf(" Part II result of original thread : \n");
    show_linux_survey_result(result_2);
    //search_and_show_SharedInterval();

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        return -1;

    memset(result_1, 0x0, MEMORY_SIZE);
    memset(result_2, 0x0, MEMORY_SIZE);
    int projectPart = atoi(argv[1]);

    if (projectPart == 1)
        project_Part_I();
    else if (projectPart == 2)
        project_Part_II();

    return 0;
}
