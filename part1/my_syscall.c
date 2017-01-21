/* syscall source code */
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/cred.h>
#include <asm/pgtable.h>
#include <asm/page.h>
//#include <linux/page.h>
#include <linux/highmem.h>

asmlinkage long sys_my_syscall(int a, unsigned long long address, pid_t pid, char *c)
{
	char buf[300];
	struct task_struct *task;
	struct mm_struct * mm;
	//struct page *page = NULL;
 	pgd_t *pgd;
	pmd_t *pmd;
	pud_t *pud;
        pte_t *ptep, pte;
	swp_entry_t swap;
	unsigned long long pgfn,phyaddr,mask,offset,swapoffset,pteint;
	int out = 0,validpid=0, len=0, n=0;
	mask = (1 << 12) - 1;
	offset = address & mask;
	//printk("%llu %d\n", address,pid);
	for_each_process(task)
        {	
		if(task->pid==pid)
		{	validpid=1;
			mm=task->mm;
			pgd = pgd_offset(mm, address);
			if (pgd_none(*pgd) || pgd_bad(*pgd))
			{
				out = 1;
				break;
			}
			//printk("pgd = %llx\n", pgd_val(*pgd));
			
			pud = pud_offset(pgd, address);
			if (pud_none(*pud) || pud_bad(*pud))
		        {
				out = 1;
				break;
			}
			//printk("pud = %llx\n", pud_val(*pud));

			pmd = pmd_offset(pud, address);
			if (pmd_none(*pmd) || pmd_bad(*pmd))
		        {	
				out = 1;
				break;
			}
			//printk("pmd = %llx\n", pmd_val(*pmd));

			ptep = pte_offset_map(pmd, address);
      			if (!ptep)
               		{
				out = 1;
				break;
			}

			pte = *ptep;

			if(pte_present(pte))
			{
				//page = pte_page(pte);
				printk("Page is present in memory, PTE = %llu\n", pte);
				pgfn = pte_pfn(pte);
				printk("PFN : %lu\n",pgfn);
				phyaddr= (pgfn<<12) | offset;
				printk("Physical address (in base 10) : %llu\n", phyaddr);
				printk("Physical address (in hexadecimal) : %llx\n", phyaddr);
				sprintf(buf,"Page is in memory, Physical address : %llx\n", phyaddr);
				pte_unmap(ptep);	
				break;
			}
			else if(!pte_none(pte))
			{ 
				printk("Page is in swap, PTE = %llu\n", pte);
				pteint = pte_val(pte);
				//printk("Page is in swap, PTE = %llu\n", pteint);
				printk("Swap ID: %llu\n", (pteint>>32));
				swap=__pte_to_swp_entry(pte);
				swapoffset = __swp_offset(swap);
				printk("Swap offset: %llu\n",swapoffset);
				sprintf(buf,"Page is in swap, PTE: %llu, Swap ID: %llu, Swap Offset: %llu\n", pte,(pteint>>32),swapoffset);
				pte_unmap(ptep);
				break;
			}
			else if(pte_none(pte))
			{	printk("Page is not mapped..not in memory and not in swap\n");
				sprintf(buf,"Page is not mapped..not in memory and not in swap\n");
				pte_unmap(ptep);
			}
		}//End of if condition
	}//End of for_each_process
	
	if(out==1)
		{
			printk("Either PGD or PUD or PMD entries are invalid!\n");
			sprintf(buf,"Either PGD or PUD or PMD entries are invalid!\n");
			
		}	
	if(validpid==0)
		{
			printk("Given PID is invalid!\n");
			sprintf(buf,"Given PID is invalid!\n");
			//printk("%s",buf);
		}
	len=strlen(buf);
	n=copy_to_user(c,buf,len);
	return n;

}

