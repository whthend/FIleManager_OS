// filesys.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include "login.h"
#include "const.h"
#include "inode.h"
#include "superblk.h"
#include "dir.h"
#include "zip.h"
#include "fs.h"
#include "file.h"
#include "block.h"
#include "path.h"

Dir 	dir_table[MaxDirNum];//����ǰĿ¼�ļ������ݶ������ڴ�
int 	dir_num;//��Ӧ��ŵ�Ŀ¼����
int	 	inode_num;//��ǰĿ¼��inode���
FILE*	Disk;
Inode 	curr_inode;//��ǰĿ¼��inode�ṹ
SuperBlk	super_blk;//�ļ�ϵͳ�ĳ�����
FILETIME BuffModifyTimeBeforeEdit;
FILETIME BuffModifyTimeAfterEdit;

/*ָ���*/
char*	command[] = { "mkfs","q","mkdir","rmdir","cd","ls","touch","rm","vi",
"cp","mv", "stat", "chmod", "zip", "unzip", "man", "df", "ps" };
char	path[40] = "FS@";
//char* username[] = { "user1", "user2" ,"user3" ,"user4", "user5" ,"user6" ,"user7" ,"user8" };
//char* password[] = { "user1", "user2" ,"user3" ,"user4", "user5" ,"user6" ,"user7" ,"user8" };
//char name[6], pwd[6];

int commander();
//int logincheck(char name[6], char pwd[6]);
//void loginfill();
//int login();
int main()
{
	Disk = fopen(DISK, "rb+");
	if (!Disk) {
		printf("���̳�ʼ��ʧ�ܣ�\n");
		system("pause");
		exit(-1);
	}
	init_fs();
	if (login() == 1)
	{
		commander();
	}

	fclose(Disk);
	return 0;
}

int	commander()
{
	char comm[NameLength], name[NameLength],
		cp_name[NameLength], mv_name[NameLength],
		zip_name[NameLength];
	char parameter[10];
	int i, quit = 0, choice;

	while (1) {
		printf("%s# ", path);
		scanf("%s", comm);
		choice = -1;

		for (i = 0; i < CommanNum; ++i) {
			if (strcmp(comm, command[i]) == 0) {
				choice = i;
				break;
			}
		}

		switch (choice) {
			/*��ʽ���ļ�ϵͳ*/
		case 0:
			format_fs();
			break;

			/*�˳��ļ�ϵͳ*/
		case 1:
			quit = 1;
			break;

			/*������Ŀ¼*/
		case 2:
			scanf("%s", name);
			make_file(inode_num, name, Directory);
			break;

			/*ɾ����Ŀ¼*/
		case 3:
			scanf("%s", name);
			remove_file(inode_num, name, 0, Directory);
			break;

			/*����Ŀ¼*/
		case 4:
			scanf("%s", name);
			if (enter_dir(name) == -1) {
				printf("cd: '%s': No such file or directory\n", name);
			}
			break;

			/*��ʾĿ¼����*/
		case 5: show_dir(inode_num);
			break;

			/*�����ļ�*/
		case 6:
			scanf("%s", name);
			make_file(inode_num, name, File);
			break;

			/*ɾ���ļ�*/
		case 7:
			scanf("%s", name);
			remove_file(inode_num, name, 0, File);
			break;

			/*���ļ����б༭*/
		case 8:
			scanf("%s", name);
			file_edit(name);
			break;

			/*�����ļ�*/
		case 9:
			scanf("%s %s", name, cp_name);
			file_copy(name, cp_name);
			break;

			/*�ƶ��ļ�*/
		case 10:
			scanf("%s %s", name, mv_name);
			file_move(name, mv_name);
			break;

			/*�鿴�ļ���Ϣ*/
		case 11:
			scanf("%s", name);
			show_file_info(name);
			break;

			/*�ı��ļ�Ȩ��*/
		case 12:
			scanf("%s %s", parameter, name);
			change_mode(parameter, name);
			break;

			/*�ļ�ѹ��*/
		case 13:
			scanf("%s %s", zip_name, name);
			zip(zip_name, name);
			break;

			/*�ļ���ѹ*/
		case 14:
			scanf("%s", name);
			unzip(name);
			break;

			/*ָ��˵���ֲ�*/
		case 15:
			show_manual();
			break;

			/*��ʾ���̿ռ�*/
		case 16:
			show_disk_usage();//��ʾ����ʹ�����
			printf("\n");
			show_inode_usage();//��ʾinode�ڵ�ʹ�����
			break;

			/*���̹���*/
		case 17:

			break;

		default:
			printf("'%s' command not found\n", comm);
			char ch;
			while ((ch = getchar()) != '\n' && ch != EOF);//ָ�����������뻺����
		}
		close_dir(inode_num);
		close_fs();//ÿִ����һ��ָ��ͱ���һ������
		if (quit) break;
	}
	return 0;
}


