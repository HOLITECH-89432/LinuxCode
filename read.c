#include<stdio.h>
#include<assert.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include"sem.h"
#define BUFFSIZE 4*1024

/*分时操作系统*/
int main(int argc,char *argv[])
{
	key_t key;	
	int shmid;
	int fdr,retur,count=0;
	char *pload=NULL;
	key_t semkey;
	int semid=0;
	
	/*信号量*/
	semkey=ftok("/root/Desktop/0828/2.c",'a');  //获取唯一键值
	semid=semget(semkey,3,IPC_CREAT);			//申请信号量，获取ID
	/*共享内存*/
	key=ftok("/root/Desktop/0829/sem.c",'c');   //获取唯一键值
	shmid=shmget(key,BUFFSIZE,IPC_CREAT);		//申请共享内存，获取ID
	pload=(char *)shmat(shmid,NULL,0);			//开辟空间
	fdr=open(argv[1],O_WRONLY|O_CREAT,0666);	//打开待写入的文件
	assert(pload!=NULL&& fdr>0);				//确保共享内存分配成功以及文件读取成功
	
	while(1)
	{
		// let write process run first	
		sem_wait(semid,3);  //BUG 
		sem_wait(semid,0);  //互斥
		count=get_sem_value(semid,2); //从信号量结构体中读取成功写入的个数
		if(count>0)  //有信息可读，直接写入文件 
		{
			retur=write(fdr,pload,count);
		}
		else if(count==0) //共享空间内文件读完 跳出while
		{
			break;
		}
		sem_post(semid,0); //出临界区
		sem_post(semid,1); //唤醒 写进程(唤醒)
	}
	close(fdr);
	shmdt(pload);
	return 1;
}