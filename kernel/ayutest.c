#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>

#define BYTES_PER_PAGE 0x1000

unsigned long ayu_pte_2_addr(pte_t *pte, unsigned int level, unsigned long v_addr);
unsigned long ayu_virt_2_phys(struct mm_struct *mm, unsigned long v_addr);

unsigned long ayu_pte_2_addr(pte_t *pte, unsigned int level, unsigned long v_addr)
{
    if ( !pte || pte_none(*pte) )
    {
        if ( !pte )
            printk("{[(ayumsg)]} pte NULL !!!\n");
        else
            printk("{[(ayumsg)]} pte ERROR !!!\n");
        return -1;
    }

    unsigned long psize = page_level_size(level);
    unsigned long pmask = page_level_mask(level);
    unsigned long offset = v_addr & ~pmask;
    unsigned long phys_addr = (phys_addr_t)pte_pfn(*pte) << PAGE_SHIFT;
    return (phys_addr | offset);
}

unsigned long ayu_virt_2_phys(struct mm_struct *mm, unsigned long v_addr)
{
    unsigned long p_addr;

    unsigned int level;
	unsigned long offset;
	unsigned long psize;
	unsigned long pmask;
	
    pgd_t *pgd = pgd_offset(mm, v_addr);
    pud_t *pud;
	pmd_t *pmd;
    pte_t *pte;

	level = PG_LEVEL_NONE;

	if ( !pgd || pgd_none(*pgd) )
		return -1;

	pud = pud_offset(pgd, v_addr);
	if ( !pud || pud_none(*pud) )
		return -1;

	level = PG_LEVEL_1G;
	if ( pud_large(*pud) || !pud_present(*pud) )
        return ayu_pte_2_addr((pte_t *)pud, level, v_addr);

	pmd = pmd_offset(pud, v_addr);
	if ( pmd_none(*pmd) )
		return -1;

	level = PG_LEVEL_2M;
	if ( pmd_large(*pmd) || !pmd_present(*pmd) )
		return ayu_pte_2_addr((pte_t *)pmd, level, v_addr);
    
    level = PG_LEVEL_4K;
    
    pte = pte_offset_map(pmd, v_addr);
    p_addr = ayu_pte_2_addr(pte, level, v_addr);
    pte_unmap(pte);

    return p_addr;
}

void storeInfo1(struct mm_struct *mm,
                unsigned long vstart,
                unsigned long vend,
                unsigned long *ary,
                unsigned long *idx)
{
    unsigned long start = vstart;

    unsigned long pageCount = (vend - vstart) / BYTES_PER_PAGE;
    unsigned long presentCount = 0;

    // check how many pages is presented within the given interval
    while ( start < vend )
    {
        if ( ayu_virt_2_phys(mm, start) != -1 )
            presentCount++;

        start += BYTES_PER_PAGE;
    }

    unsigned long ps = ayu_virt_2_phys(mm, vstart);
    unsigned long pe = ayu_virt_2_phys(mm, vend);

    ary[(*idx)++] = vstart;
    ary[(*idx)++] = vend;
    ary[(*idx)++] = ps;
    ary[(*idx)++] = pe;
    ary[(*idx)++] = presentCount;
}

void storeInfo2(struct mm_struct *mm,
                unsigned long vstart,
                unsigned long vend,
                unsigned long *ary,
                unsigned long *idx)
{
    unsigned long start = vstart;
    while ( start < vend )
    {
        unsigned long ps = ayu_virt_2_phys(mm, vstart);

        // the size of interval is one page
        // so the information of end is not require
        // and this page is presented or not is determinate by the virtual address is successfully tranform
        ary[(*idx)++] = vstart;
        ary[(*idx)++] = 0;
        ary[(*idx)++] = ps;
        ary[(*idx)++] = 0;
        ary[(*idx)++] = (ps != -1);

        start += BYTES_PER_PAGE;
    }
}

unsigned long sys_linux_project(int pid, unsigned long *addr, unsigned long arySize)
{
    struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if ( !task )
    {
        printk("{[(ayumsg)]} task not found !!!\n");
        return -1;
    }

    struct vm_area_struct *vma = 0;

    unsigned long idxAry1 = 0;
    unsigned long idxAry2 = arySize/4;

    if ( task->mm && task->mm->mmap )
    {
        for ( vma = task->mm->mmap ; vma ; vma = vma->vm_next )
        {
            // 前四分之一放 vma 提供的區間的對應資訊.
            if ( idxAry1+4 < (arySize/4) )
                showinfo1(task->mm, vma->vm_start, vma->vm_end, addr, &idxAry1);

            // 前四分之三放所有 page 對應的資訊
            if ( idxAry2+4 < (arySize) )
                showinfo2(task->mm, vma->vm_start, vma->vm_end, addr, &idxAry2);
        }
    }
    return task->pid;
}

asmlinkage unsigned long sys_linux_project_partI(int pid, unsigned long *addr, unsigned long arySize)
{
    return sys_linux_project(pid, addr, arySize);
}

asmlinkage unsigned long sys_linux_project_partII(unsigned long *addr, unsigned long arySize)
{
    return sys_linux_project(current->pid, addr, arySize);
}