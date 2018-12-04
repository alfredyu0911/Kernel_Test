#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/mm.h>

unsigned long vaddr2paddr0(struct mm_struct *mm, unsigned long vaddr)
{
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long paddr = 0;
    unsigned long page_addr = 0;
    unsigned long page_offset = 0;

    pgd = pgd_offset(mm, vaddr);
    printk(KERN_INFO"pgd = 0x%p\n",pgd);
    printk(KERN_INFO"pgd_val(*pgd) = 0x%lx\n",pgd_val(*pgd));
    if ( pgd_none(*pgd) || pgd_bad(*pgd) )
    {
        printk(KERN_INFO"Not mapped in pgd.\n");
        return 0;
    }
    /*printk("{[(ayumsg)]} pgd_val = 0x%lx\n", pgd_val(*pgd));
    printk("{[(ayumsg)]} pgd_index = %lu\n", pgd_index(vaddr));
    if ( pgd_none(*pgd) ) 
    {
        printk("{[(ayumsg)]} not mapped in pgd\n");
        return -1;
    }*/

    pud = pud_offset(pgd, vaddr);
    printk(KERN_INFO"pud = 0x%p\n",pud);
    printk(KERN_INFO"pud_val(*pud) = 0x%lx\n",pud_val(*pud));
    if ( pud_none(*pud) || pud_bad(*pud) )
    {
        printk(KERN_INFO"Not mapped in pud.\n");
        return -1;
    }
    /*printk("{[(ayumsg)]} pud_val = 0x%lx\n", pud_val(*pud));
    if ( pud_none(*pud) ) 
    {
        printk("{[(ayumsg)]} not mapped in pud\n");
        return -1;
    }*/

    pmd = pmd_offset(pud, vaddr);
    printk(KERN_INFO"pmd = 0x%p\n",pmd);
    printk(KERN_INFO"pmd_val(*pmd) = 0x%lx\n",pmd_val(*pmd));
    if ( pmd_none(*pmd) || pmd_bad(*pmd) )
    {
        printk(KERN_INFO"Not mapped in pmd.\n");
        return -1;
    }
    /*printk("{[(ayumsg)]} pmd_val = 0x%lx\n", pmd_val(*pmd));
    printk("{[(ayumsg)]} pmd_index = %lu\n", pmd_index(vaddr));
    if ( pmd_none(*pmd) ) 
    {
        printk("{[(ayumsg)]} not mapped in pmd\n");
        return -1;
    }*/

    printk("{[(ayumsg)]} check 1-1\n");
    pte = pte_offset_kernel(pmd, vaddr);
    printk("{[(ayumsg)]} check 1-2\n");
    if(pte_none(*pte))
    {
        printk("{[(ayumsg)]} check 1-3\n");
        printk(KERN_INFO"Not mapped in pte.\n");
        return -1;
    }
    printk("{[(ayumsg)]} check 1-4\n");
    if(!pte_present(*pte))
    {
        printk(KERN_INFO"pte not in RAM.\n");
        return -1;
    }
    printk(KERN_INFO"pte = 0x%p\n",pte);
    printk(KERN_INFO"pte_val(*pte) = 0x%lx\n",pte_val(*pte));
    /*printk("{[(ayumsg)]} pte_val = 0x%lx\n", pte_val(*pte));
    printk("{[(ayumsg)]} pte_index = %lu\n", pte_index(vaddr));
    printk("{[(ayumsg)]} check 9\n");
    return -1;*/
    return (pte_val(*pte) & PAGE_MASK) | (vaddr & ~PAGE_MASK);
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
               unsigned long *idx)
{
    unsigned long ps = vaddr2paddr0(mm, vstart);
    unsigned long pe = vaddr2paddr0(mm, vend);

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

asmlinkage long sys_ayutest0(int pid, unsigned long *addr, unsigned long arySize)
{
    struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);
    struct vm_area_struct *vma = 0;

    unsigned long idxAry = 0;

    if (task->mm && task->mm->mmap)
    {
        for (vma = task->mm->mmap; vma; vma = vma->vm_next)
        {
            if ( idxAry+3 < arySize )
                showinfo0(task->mm, vma->vm_start, vma->vm_end, addr, &idxAry);
        }
    }
    return task->pid;
}

asmlinkage long sys_ayutest1(int pid, unsigned long *addr, unsigned long arySize)
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

asmlinkage long sys_ayutest2(int pid, unsigned long *addr, unsigned long arySize)
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
