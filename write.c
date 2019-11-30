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
int main(int argc,char *argv[])
{
	key_t key;
	int shmid;
	int fdr,retur;
	char *pload=NULL;
	key_t semkey;
	int semid=0;
	int value[]={1,0,0,0}; 
	/*
		位0：互斥锁
		位1：唤醒进程
		位2：计数
		位3：没申请
	*/
	
	/*信号量*/
	semkey=ftok("/root/Desktop/0828/2.c",'a'); //获取唯一键值
	semid=semget(semkey,3,IPC_CREAT);		   //申请信号量，获取ID
	initial_sem_array(semid,value,3);		   //初始化信号量
	/*共享内存*/
	key=ftok("/root/Desktop/0829/sem.c",'c');  //获取唯一键值
	shmid=shmget(key,BUFFSIZE,IPC_CREAT);	   //申请共享内存，获取ID
	pload=(char *)shmat(shmid,NULL,0);		   //开辟空间
	fdr=open(argv[1],O_RDONLY,0666);			   //读文件
	assert(pload!=NULL&& fdr>0);                 //确保共享内存分配成功以及文件读取成功

	while(1)
	{
		sem_wait(semid,0);	//进入共享内存（临界区） 互斥
		memset(pload,'\0',BUFFSIZE*sizeof(char)); //确保空间干净
		retur=read(fdr,pload,BUFFSIZE);	 	 	  //将文件写入共享空间，retur：成功写入的个数
		sem_post(semid,0);  //出临界区
		//set the number of share memory to be read
		set_sem_value(semid,2,retur);  //将成功写入的个数写入信号量结构体
		if(retur==0)  //如果文件读取到最后 跳出while
		{
			break;
		}
		else
		printf("write share memory  retur =%d\n",retur);
		sem_post(semid,3);		//唤醒 读进程 BUG(没申请，没定义)
		sem_wait(semid,1);		//阻塞 写进程 BUG(本身就已经阻塞)
	}
	close(fdr);		//关闭文件
	shmdt(pload);	//释放共享内存
	return 1;
}