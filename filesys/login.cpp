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
extern int	 	inode_num;//��ǰĿ¼��inode���
extern int		user_num;
int				login_suc=0;

char* username[] = { "user1", "user2" ,"user3" ,"user4", "user5" ,"user6" ,"user7" ,"user8" ,"root"};
char* password[] = { "user1", "user2" ,"user3" ,"user4", "user5" ,"user6" ,"user7" ,"user8","root"};
char name[6], pwd[6];

/*��½�������ж���Ϣ�Ƿ�ƥ�䣬��ƥ�䷵��1�����򷵻�0*/
int logincheck(char name[6],char pwd[6])
{
	printf("�û��� : ");
	scanf("%s", name);
	printf("��  �� : ");
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
	puts("\n��¼ʧ�ܣ�ϵͳ5�����ֹ��\n");
	_sleep(5000);//��ʱ5��
	exit(0);
}

int login()
{
	int count = 5; //���������
	while (count)
	{
		if (logincheck(name,pwd)== 1)
			break;
		else
		{
			count--;
			printf("�û���������󣬻��� %d �λ���\n", count);
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
				printf("'%s'�û���Ŀ¼��ʧ��\n", name);
				return 0;
			}
			else
			{
				printf("\n�ɹ���½��\n");
			}
		}
		else
		{
			printf("��ӭ��������Ա");
		}
		
	}
	else
	{
		loginfill();
	}	
	return 1;
}