#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>	   // Kmalloc/Kfree
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/tty.h>
#include <asm/uaccess.h>
#include <linux/cred.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>

static int Monitor_Thrashing(void *data);
//extern int pmd_huge(pmd_t pmd);

//Declare and initialize kernel thread to detect thrashing
static struct task_struct * KernelDetectThread = NULL;

int pmd_huge(pmd_t pmd)
{
        return (!pmd_none(pmd) && (pmd_val(pmd) & (_PAGE_PRESENT|_PAGE_PSE)) != _PAGE_PRESENT);
}

int virt2phys(struct mm_struct* mm, unsigned long vpage)
{
	int ret = 0;
	pte_t *ptep;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	

	spin_lock(&mm->page_table_lock);

	pgd = pgd_offset(mm, vpage);
	if (pgd_present(*pgd))
	{
		pud = pud_offset(pgd, vpage);
		if (pud_present(*pud))		
		{
			pmd = pmd_offset(pud, vpage);
			if (pmd_present(*pmd))
			{	
				if(!pmd_huge(*pmd))
				{
				    	ptep = pte_offset_map(pmd, vpage);
				    	if( pte_present(*ptep) )
					{
						if(pte_young(*ptep))
						{
							set_pte(ptep, pte_mkold(*ptep));
							ret = 1;
						}	
					} 
					else 
					{
						ret = 0;	
					}
					pte_unmap(ptep);
				}
				else
				{
					if(pmd_young(*pmd)){
						set_pmd(pmd, pmd_mkold(*pmd));
						ret = (PMD_PAGE_SIZE/PAGE_SIZE);
					}
					else
					{
						ret = 0;
					}
				}
			}	
		}
	}

	
	spin_unlock(&mm->page_table_lock);
	return ret;
}


/*
 * Runs through each task and calls virt2phys() function to check whether page was accessed or not
 */
void Thrashing_detect(void)
{		
	struct task_struct *task;
	struct vm_area_struct *vma;
	int totalMemory = 1999960000 * 0.9;
	unsigned long long vpage = 0;
	unsigned long long wss = 0;
	unsigned long long twss = 0;
	unsigned long long temp = 0;

	for_each_process(task)
	{
		vma = 0;
		wss = 0;
		if (task->mm && task->mm->mmap)
		{
			for (vma = task->mm->mmap; vma != NULL; vma = vma->vm_next)
		    	{
				for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE)
				{
					temp = virt2phys(task->mm, vpage);
					twss+=temp;
					wss+=temp;
				}
		    	}
			printk(KERN_ALERT "PID : %04d	WSS : %lld\n", task->pid, wss);
		}
	}
	printk(KERN_ALERT "\nTWSS : %lld\n", twss);
	printk(KERN_ALERT "\nTWSS Size in kB: %lld kB\n", twss * 4096);

	if( (totalMemory) <= (twss * 4096))
	{
		printk(KERN_ALERT "\n\nKernel Alert!\n\n");
	}
}
		

/*
 * Kernel Thread Handler to monitor Thrashing
 */
static int Monitor_Thrashing(void *data)
{
	while(!kthread_should_stop())
	{
		printk("kernel thread running \n");
		Thrashing_detect();
		msleep(1000);
	}

	return 0;
}


/*
 * Thrashing Detection Kernel Module Init Function
 */
static int __init detect_init(void)
{
	KernelDetectThread = kthread_run(Monitor_Thrashing, NULL,"Kernel_Thread_Detect_thrashing");
	return 0;
}


/*
 * Thrashing Detection Kernel Module Exit Function
 */
static void __exit detect_exit(void)
{
	//Stop the kernel thread as exiting from kernel module
	if(KernelDetectThread)
	{
		kthread_stop(KernelDetectThread);
	}
}


module_init(detect_init);
module_exit(detect_exit);
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("SAILI BAKARE, SIVAKUMAR VENKATARAMAN, OMKAR SALVI");
MODULE_DESCRIPTION("Project 3 Step 2 for CSE430 OPERATING SYSTEMS");
