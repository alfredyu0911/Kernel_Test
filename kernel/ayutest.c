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

    // printk("{[(ayumsg)]} check 1 pgd_val, pgd_index = [0x%lx, %lu]\n", pgd_val(*pgd), pgd_index(v_addr));

	pud = pud_offset(pgd, v_addr);
	if ( !pud || pud_none(*pud) )
		return -1;

    // printk("{[(ayumsg)]} check 2\n");

	level = PG_LEVEL_1G;
	if ( pud_large(*pud) || !pud_present(*pud) )
        return ayu_pte_2_addr((pte_t *)pud, level, v_addr);

    // printk("{[(ayumsg)]} check 3 pud_val = 0x%lx\n", pud_val(*pud));

	pmd = pmd_offset(pud, v_addr);
	if ( pmd_none(*pmd) )
		return -1;

    // printk("{[(ayumsg)]} check 4\n");

	level = PG_LEVEL_2M;
	if ( pmd_large(*pmd) || !pmd_present(*pmd) )
		return ayu_pte_2_addr((pte_t *)pmd, level, v_addr);

    // printk("{[(ayumsg)]} check 5 pmd_val, pmd_index = [0x%lx, %lu]\n", pmd_val(*pmd), pmd_index(v_addr));
    
    level = PG_LEVEL_4K;
    // printk("{[(ayumsg)]} check 7\n");
    pte = pte_offset_map(pmd, v_addr);
    p_addr = ayu_pte_2_addr(pte, level, v_addr);
    pte_unmap(pte);

    return p_addr;
}

void showinfoI(struct mm_struct *mm,
               unsigned long vstart,
               unsigned long vend,
               unsigned long *ary,
               unsigned long *idx)
{
    unsigned long start = vstart;

    unsigned long pageCount = (vend - vstart) / BYTES_PER_PAGE;
    unsigned long presentCount = 0;

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

asmlinkage unsigned long sys_linux_project_partI(int pid, unsigned long *addr, unsigned long arySize)
{
    struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);
    struct vm_area_struct *vma = 0;

    unsigned long idxAry = 0;

    if (task->mm && task->mm->mmap)
    {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next)
        {
            if ( idxAry+4 < arySize )
                showinfoI(task->mm, vma->vm_start, vma->vm_end, addr, &idxAry);
        }
    }
    return task->pid;
}

asmlinkage unsigned long sys_linux_project_partII(unsigned long *addr, unsigned long arySize)
{
    struct task_struct *task = current;
    struct vm_area_struct *vma = 0;
    pid_t pid;

    unsigned long idxAry = 0;

    if (task->mm && task->mm->mmap)
    {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next)
        {
            rcu_read_lock();

            struct task_struct *thread = current;
            while_each_thread(current, thread) 
            {
                if ( current != thread && thread -> state == TASK_RUNNING )
                {
                    pid = thread->pid;
                    if ( idxAry+4 < arySize )
                        showinfoI(thread, vma->vm_start, vma->vm_end, addr, &idxAry);
                }
            };

            rcu_read_unlock();
        }
    }

    return pid;
}
