/*User space program to test the system call*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __NR_my_syscall 359

int main(int argc, char* argv[])
{	
	char buff[300];
	pid_t pid;
	unsigned long long vir_mem_addr = 0;
	int count=0;
	memset(buff,0,300);

	if(argc < 3)
	{
		printf("\nNot enough arguments. \nUsage: ./testuserspace <virtual_memory_address> <PID>\n");
		return 1;
	}
	else
	{	
		//printf("%s %s",argv[1],argv[2]);
		vir_mem_addr=strtoull(argv[1],NULL,16);
		pid = atoi(argv[2]);
		printf("Virt Addr: %lld, PID: %d \n",vir_mem_addr,pid); 
		count = syscall(__NR_my_syscall,0,vir_mem_addr,pid,buff);
		printf("%s\n",buff);
	}
	return 0;
}
