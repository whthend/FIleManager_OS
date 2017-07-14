#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <iostream> 
#include "login.h"
#include "dir.h"
#include "path.h"

extern char	path[40];
extern FILE*	Disk;
extern int	 	inode_num;//当前目录的inode编号
extern int		user_num;
int				login_suc=0;

char* username[] = { "user1", "user2" ,"user3" ,"user4", "user5" ,"user6" ,"user7" ,"user8" ,"root"};
char* password[] = { "user1", "user2" ,"user3" ,"user4", "user5" ,"user6" ,"user7" ,"user8","root"};
char name[6], pwd[6];

/*登陆函数，判断信息是否匹配，若匹配返回1，否则返回0*/
int logincheck(char name[6],char pwd[6])
{
	printf("用户名 : ");
	scanf("%s", name);
	printf("密  码 : ");
	scanf("%s", pwd);

	for (user_num=0; user_num < 9; user_num++)
	{
		if (strcmp(name, username[user_num]) == 0)
		{
			break;
		}
		if(user_num==8)
			return 0;
	}			
	
	if (strcmp(pwd, password[user_num]) == 0)
	{
		login_suc = 1;
		return 1;
	}
	else
		return 0;
	
}

void loginfill()
{
	puts("\n登录失败！系统5秒后终止！\n");
	_sleep(5000);//延时5秒
	exit(0);
}

int login()
{
	int count = 5; //可输入次数
	while (count)
	{
		if (logincheck(name,pwd)== 1)
			break;
		else
		{
			count--;
			printf("用户名密码错误，还有 %d 次机会\n", count);
		}
	}

	if (count != 0)
	{
		strcat(path, username[user_num]);
		if (user_num != 8) {
			if (check_name(inode_num, name) == -1)
			{
				make_file(inode_num, name, Directory);
			}
			if (enter_dir_first(name) == -1) {
				printf("'%s'用户根目录打开失败\n", name);
				return 0;
			}
			else
			{
				printf("\n成功登陆！\n");
			}
		}
		else
		{
			printf("欢迎超级管理员");
		}
		
	}
	else
	{
		loginfill();
	}	
	return 1;
}