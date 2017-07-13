#include <stdio.h>
#include <string.h>
#include "const.h"
#include "inode.h"
#include "superblk.h"
#include "dir.h"
#include "path.h"

extern char	path[40];
extern int	inode_num;//��ǰĿ¼��inode���
extern int 	dir_num;//��Ӧ��ŵ�Ŀ¼����
extern Dir 	dir_table[MaxDirNum];//����ǰĿ¼�ļ������ݶ������ڴ�
extern FILE*	Disk;

void change_path(char *name)
{
	int pos;
	if (strcmp(name, ".") == 0) {//���뱾Ŀ¼��·������
		return;
	}
	else if (strcmp(name, "..") == 0) {//�����ϲ�Ŀ¼�������һ��'/'�������ȥ��
		pos = strlen(path) - 1;
		for (; pos >= 0; --pos) {
			if (path[pos] == '/') {
				path[pos] = '\0';
				break;
			}
		}
	}
	else {//������·��ĩβ�����Ŀ¼
		strcat(path, "/");
		strcat(path, name);
	}

	return;
}

int eat_path(char* name)
{
	int tmp = inode_num;//��¼ԭʼinode�ڵ�
	char dst[30][NameLength];
	int cnt = split(dst, name, "/");
	if (name[0] == '/') {//�Ӹ�Ŀ¼��ʼ
						 //printf("1111111111\n");
		close_dir(inode_num);
		inode_num = 0;
		open_dir(inode_num);
	}
	for (int i = 0; i < cnt - 1; i++) {
		//printf("%d\n", i);
		int res = enter_child_dir(inode_num, dst[i]);
		if (res == -1) {
			inode_num = tmp;
			open_dir(inode_num);
			return -1;
		}
	}
	if (cnt == 0)
		strcpy(name, ".");
	else
		strcpy(name, dst[cnt - 1]);
	return 0;
}

int split(char dst[][NameLength], char* str, const char* spl)
{
	int n = 0;
	char *result = NULL;
	result = strtok(str, spl);
	while (result != NULL)
	{
		strcpy(dst[n++], result);
		result = strtok(NULL, spl);
	}
	return n;
}


/*���������
����inode_num
*/
int check_name(int inode, char* name)
{
	int i;
	for (i = 0; i<dir_num; ++i) {
		/*����������*/
		if (strcmp(name, dir_table[i].name) == 0) {
			return dir_table[i].inode_num;
		}
	}

	return -1;
}

int type_check(char* name)
{
	int 	i, inode;
	Inode 	temp;
	for (i = 0; i<dir_num; ++i) {
		if (strcmp(name, dir_table[i].name) == 0) {
			inode = dir_table[i].inode_num;
			fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);

			fread(&temp, sizeof(Inode), 1, Disk);
			return temp.type;
		}
	}
	return -1;//���ļ���Ŀ¼������
}
