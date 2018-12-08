#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>

unsigned long ayu_pte_2_addr(pte_t *pte, unsigned int level, unsigned long v_addr);
unsigned long ayu_virt_2_phys(struct mm_struct *mm, unsigned long v_addr, int testMode);

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

unsigned long ayu_virt_2_phys(struct mm_struct *mm, unsigned long v_addr, int testMode)
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

    printk("{[(ayumsg)]} check 1 pgd_val, pgd_index = [0x%lx, %lu]\n", pgd_val(*pgd), pgd_index(v_addr));

	pud = pud_offset(pgd, v_addr);
	if ( !pud || pud_none(*pud) )
		return -1;

    printk("{[(ayumsg)]} check 2\n");

	level = PG_LEVEL_1G;
	if ( pud_large(*pud) || !pud_present(*pud) )
        return ayu_pte_2_addr((pte_t *)pud, level, v_addr);

    printk("{[(ayumsg)]} check 3 pud_val = 0x%lx\n", pud_val(*pud));

	pmd = pmd_offset(pud, v_addr);
	if ( pmd_none(*pmd) )
		return -1;

    printk("{[(ayumsg)]} check 4\n");

	level = PG_LEVEL_2M;
	if ( pmd_large(*pmd) || !pmd_present(*pmd) )
		return ayu_pte_2_addr((pte_t *)pmd, level, v_addr);

    printk("{[(ayumsg)]} check 5 pmd_val, pmd_index = [0x%lx, %lu]\n", pmd_val(*pmd), pmd_index(v_addr));
    
    level = PG_LEVEL_4K;
    if ( testMode == 1 )
    {
        printk("{[(ayumsg)]} check 6\n");
        pte = pte_offset_kernel(pmd, v_addr);
        p_addr = ayu_pte_2_addr(pte, level, v_addr);
    }
    else
    {
        printk("{[(ayumsg)]} check 7\n");
        pte = pte_offset_map(pmd, v_addr);
        p_addr = ayu_pte_2_addr(pte, level, v_addr);
        pte_unmap(pte);
    }

    return p_addr;
}

unsigned long vaddr2paddr1(struct mm_struct *mm, unsigned long vaddr)
{
        pgd_t *pgd;
        pud_t *pud;
        pmd_t *pmd;
        pte_t *pte;
        unsigned long paddr = 0;
            unsigned long page_addr = 0;
        unsigned long page_offset = 0;

        pgd = pgd_offset(current->mm, vaddr);
        printk("pgd_val = 0x%lx\n", pgd_val(*pgd));
        printk("pgd_index = %lu\n", pgd_index(vaddr));
        if (pgd_none(*pgd)) {
            printk("not mapped in pgd\n");
            return -1;
        }

        pud = pud_offset(pgd, vaddr);
        printk("pud_val = 0x%lx\n", pud_val(*pud));
        if (pud_none(*pud)) {
            printk("not mapped in pud\n");
            return -1;
        }

        pmd = pmd_offset(pud, vaddr);
        printk("pmd_val = 0x%lx\n", pmd_val(*pmd));
        printk("pmd_index = %lu\n", pmd_index(vaddr));
        if (pmd_none(*pmd)) {
            printk("not mapped in pmd\n");
            return -1;
        }

        pte = pte_offset_kernel(pmd, vaddr);
        printk("pte_val = 0x%lx\n", pte_val(*pte));
        printk("pte_index = %lu\n", pte_index(vaddr));
        if (pte_none(*pte)) {
            printk("not mapped in pte\n");
            return -1;
        }

        /* Page frame physical address mechanism | offset */
        page_addr = pte_val(*pte) & PAGE_MASK;
        page_offset = vaddr & ~PAGE_MASK;
        paddr = page_addr | page_offset;
        printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
            printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);

        return paddr;
    }

unsigned long vaddr2paddr2(struct mm_struct *mm, unsigned long vaddr)
{
        pgd_t *pgd;
        pud_t *pud;
        pmd_t *pmd;
        pte_t *pte;
        unsigned long paddr = 0;
            unsigned long page_addr = 0;
        unsigned long page_offset = 0;

        pgd = pgd_offset(mm, vaddr);
        printk("pgd_val = 0x%lx\n", pgd_val(*pgd));
        printk("pgd_index = %lu\n", pgd_index(vaddr));
        if (pgd_none(*pgd)) {
            printk("not mapped in pgd\n");
            return -1;
        }

        pud = pud_offset(pgd, vaddr);
        printk("pud_val = 0x%lx\n", pud_val(*pud));
        if (pud_none(*pud)) {
            printk("not mapped in pud\n");
            return -1;
        }

        pmd = pmd_offset(pud, vaddr);
        printk("pmd_val = 0x%lx\n", pmd_val(*pmd));
        printk("pmd_index = %lu\n", pmd_index(vaddr));
        if (pmd_none(*pmd)) {
            printk("not mapped in pmd\n");
            return -1;
        }

        pte = pte_offset_kernel(pmd, vaddr);
        printk("pte_val = 0x%lx\n", pte_val(*pte));
        printk("pte_index = %lu\n", pte_index(vaddr));
        if (pte_none(*pte)) {
            printk("not mapped in pte\n");
            return -1;
        }
        
        /* Page frame physical address mechanism | offset */
        page_addr = pte_val(*pte) & PAGE_MASK;
        page_offset = vaddr & ~PAGE_MASK;
        paddr = page_addr | page_offset;
        printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
            printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);

        return paddr;
    }

void showinfo0(struct mm_struct *mm,
               unsigned long vstart,
               unsigned long vend,
               unsigned long *ary,
               unsigned long *idx, 
               int testMode)
{
    unsigned long ps = ayu_virt_2_phys(mm, vstart, testMode);
    unsigned long pe = ayu_virt_2_phys(mm, vend, testMode);

    ary[(*idx)++] = vstart;
    ary[(*idx)++] = vend;
    ary[(*idx)++] = ps;
    ary[(*idx)++] = pe;
    printk("ayu kernel test msg 0 : v[0x%lX 0x%lX] p{0x%lX 0x%lX}\n", vstart, vend, ps, pe);
}

void showinfo1(struct mm_struct *mm,
               unsigned long vstart,
               unsigned long vend,
               unsigned long *ary,
               unsigned long *idx)
{
    unsigned long ps = vaddr2paddr1(mm, vstart);
    unsigned long pe = vaddr2paddr1(mm, vend);

    ary[(*idx)++] = vstart;
    ary[(*idx)++] = vend;
    ary[(*idx)++] = ps;
    ary[(*idx)++] = pe;
    printk("ayu kernel test msg 1 : v[0x%lX 0x%lX] p{0x%lX 0x%lX}\n", vstart, vend, ps, pe);
}

void showinfo2(struct mm_struct *mm,
               unsigned long vstart,
               unsigned long vend,
               unsigned long *ary,
               unsigned long *idx)
{
    unsigned long ps = vaddr2paddr2(mm, vstart);
    unsigned long pe = vaddr2paddr2(mm, vend);

    ary[(*idx)++] = vstart;
    ary[(*idx)++] = vend;
    ary[(*idx)++] = ps;
    ary[(*idx)++] = pe;
    printk("ayu kernel test msg 2 : v[0x%lX 0x%lX] p{0x%lX 0x%lX}\n", vstart, vend, ps, pe);
}

asmlinkage long sys_ayutest0(int pid, unsigned long *addr, unsigned long arySize, int testMode)
{
    struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);
    struct vm_area_struct *vma = 0;

    unsigned long idxAry = 0;

    if (task->mm && task->mm->mmap)
    {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next)
        {
            if ( idxAry+3 < arySize )
                showinfo0(task->mm, vma->vm_start, vma->vm_end, addr, &idxAry, testMode);
        }
    }
    return task->pid;
}

asmlinkage long sys_ayutest1(int pid, unsigned long *addr, unsigned long arySize, int testMode)
{
    struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);
    struct vm_area_struct *vma = 0;

    unsigned long idxAry = 0;

    if (task->mm && task->mm->mmap)
    {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next)
        {
            if ( idxAry+3 < arySize )
                showinfo1(task->mm, vma->vm_start, vma->vm_end, addr, &idxAry);
        }
    }
    return task->pid;
}

asmlinkage long sys_ayutest2(int pid, unsigned long *addr, unsigned long arySize, int testMode)
{
    struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);
    struct vm_area_struct *vma = 0;

    unsigned long idxAry = 0;

    if (task->mm && task->mm->mmap)
    {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next)
        {
            if ( idxAry+3 < arySize )
                showinfo2(task->mm, vma->vm_start, vma->vm_end, addr, &idxAry);
        }
    }
    return task->pid;
}
