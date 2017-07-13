#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "const.h"
#include "inode.h"
#include "superblk.h"
#include "dir.h"
#include "path.h"
#include "block.h"

extern Dir 	dir_table[MaxDirNum];//����ǰĿ¼�ļ������ݶ������ڴ�
extern int 	dir_num;//��Ӧ��ŵ�Ŀ¼����
extern int	 	inode_num;//��ǰĿ¼��inode���
extern Inode 	curr_inode;//��ǰĿ¼��inode�ṹ
extern SuperBlk	super_blk;//�ļ�ϵͳ�ĳ�����
extern FILE*	Disk;
extern char	path[40];

int open_dir(int inode)
{
	int		i;
	int 	pos = 0;
	int 	left;
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);

	/*������Ӧ��i�ڵ�*/
	fread(&curr_inode, sizeof(Inode), 1, Disk);
	//	printf("%d\n",curr_inode.file_size);

	for (i = 0; i<curr_inode.blk_num - 1; ++i) {
		fseek(Disk, BlockBeg + BlkSize*curr_inode.blk_identifier[i], SEEK_SET);
		fread(dir_table + pos, sizeof(Dir), DirPerBlk, Disk);
		pos += DirPerBlk;
	}

	/*leftΪ���һ�����̿��ڵ�Ŀ¼����*/
	left = curr_inode.file_size / sizeof(Dir) - DirPerBlk*(curr_inode.blk_num - 1);
	fseek(Disk, BlockBeg + BlkSize*curr_inode.blk_identifier[i], SEEK_SET);
	fread(dir_table + pos, sizeof(Dir), left, Disk);
	pos += left;

	dir_num = pos;

	return 1;
}

int open_dir_first(int inode)
{
	int		i;
	int 	pos = 0;
	int 	left;
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);

	/*������Ӧ��i�ڵ�*/
	fread(&curr_inode, sizeof(Inode), 1, Disk);
	//	printf("%d\n",curr_inode.file_size);

	for (i = 0; i<curr_inode.blk_num - 1; ++i) {
		fseek(Disk, BlockBeg + BlkSize*curr_inode.blk_identifier[i], SEEK_SET);
		fread(dir_table + pos, sizeof(Dir), DirPerBlk, Disk);
		pos += DirPerBlk;
	}

	/*leftΪ���һ�����̿��ڵ�Ŀ¼����*/
	left = curr_inode.file_size / sizeof(Dir) - DirPerBlk*(curr_inode.blk_num - 1);
	fseek(Disk, BlockBeg + BlkSize*curr_inode.blk_identifier[i], SEEK_SET);
	fread(dir_table + pos, sizeof(Dir), left, Disk);
	pos += left;

	dir_num = pos;

	return 1;
}

int close_dir(int inode)
{
	int i, pos = 0, left;

	/*������Ӧ��i�ڵ�*/
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&curr_inode, sizeof(Inode), 1, Disk);

	/*����д�ش��̿�*/
	for (i = 0; i<curr_inode.blk_num - 1; ++i) {
		fseek(Disk, BlockBeg + BlkSize*curr_inode.blk_identifier[i], SEEK_SET);
		fwrite(dir_table + pos, sizeof(Dir), DirPerBlk, Disk);
		pos += DirPerBlk;
	}

	left = dir_num - pos;
	//	printf("left:%d",left);
	fseek(Disk, BlockBeg + BlkSize*curr_inode.blk_identifier[i], SEEK_SET);
	fwrite(dir_table + pos, sizeof(Dir), left, Disk);

	/*inodeд��*/
	curr_inode.file_size = dir_num*sizeof(Dir);
	fseek(Disk, InodeBeg + inode*sizeof(Inode), SEEK_SET);
	fwrite(&curr_inode, sizeof(curr_inode), 1, Disk);

	return 1;

}


/*�����µ�Ŀ¼��*/
int make_file(int inode, char* name, int type)
{
	char original_name_path[30];
	int original_inode = inode_num;//��¼��ǰ��inode

	strcpy(original_name_path, name);
	if (eat_path(name) == -1) {
		if (type == File)
			printf("touch: cannot touch��%s��: No such file or directory\n", original_name_path);
		if (type == Directory)
			printf("mkdir: cannot create directory ��%s��: No such file or directory\n", original_name_path);
		return -1;
	}

	int new_node;
	int blk_need = 1;//��Ŀ¼��Ҫ���Ӵ��̿���blk_need=2
	int t;
	Inode temp;

	/*��ȡ��ǰĿ¼��Inode*/
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	if (temp.access[1] == 0) { //��ǰĿ¼������д
		if (type == Directory)
			printf("mkdir: cannot create directory ��%s��: Permission denied\n", original_name_path);
		if (type == File)
			printf("touch: cannot touch ��%s��: Permission denied\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}
	if (dir_num>MaxDirNum) {//������Ŀ¼�ļ��ܰ��������Ŀ¼��
		if (type == Directory)
			printf("mkdir: cannot create directory '%s' : Directory full\n", original_name_path);
		if (type == File)
			printf("touch: cannot create file '%s' : Directory full\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	if (check_name(inode, name) != -1) {//��ֹ������
		if (type == Directory)
			printf("mkdir: cannnot create directory '%s' : Directory exist\n", original_name_path);
		if (type == File)
			printf("touch: cannot create file '%s' : File exist\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	if (dir_num / DirPerBlk != (dir_num + 1) / DirPerBlk) {//��Ŀ¼ҲҪ���Ӵ��̿�
		blk_need = 2;
	}

	//	printf("blk_used:%d\n",super_blk.blk_used);
	if (super_blk.blk_used + blk_need>BlkNum) {
		if (type == Directory)
			printf("mkdir: cannot create directory '%s' :Block used up\n", original_name_path);
		if (type == File)
			printf("touch: cannot create file '%s' : Block used up\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	if (blk_need == 2) {//��Ŀ¼��Ҫ���Ӵ��̿�
		t = curr_inode.blk_num++;
		curr_inode.blk_identifier[t] = get_blk();
	}

	/*����inode*/
	new_node = apply_inode();

	if (new_node == -1) {
		if (type == Directory)
			printf("mkdir: cannot create directory '%s' :Inode used up\n", original_name_path);
		if (type == File)
			printf("touch: cannot create file '%s' : Inode used up\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	if (type == Directory) {
		/*��ʼ���½�Ŀ¼��inode*/
		init_dir_inode(new_node, inode);
	}
	else if (type == File) {
		/*��ʼ���½��ļ���inode*/
		init_file_inode(new_node);
	}

	strcpy(dir_table[dir_num].name, name);
	dir_table[dir_num++].inode_num = new_node;

	close_dir(inode_num);
	inode_num = original_inode;
	open_dir(inode_num);
	return 0;
}


/*��ʾĿ¼����*/
int show_dir(int inode)
{
	int i;
	Inode temp;
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	if (temp.access[0] == 0) { //Ŀ¼�޶�дȨ��
		printf("ls: cannot open directory .: Permission denied\n");
		return -1;
	}
	for (i = 0; i<dir_num; ++i) {
		char name_suffix[NameLength];
		strcpy(name_suffix, dir_table[i].name + strlen(dir_table[i].name) - 4);

		int inode_tmp = check_name(inode_num, dir_table[i].name);
		Inode tempInode;
		fseek(Disk, InodeBeg + sizeof(Inode)*inode_tmp, SEEK_SET);
		fread(&tempInode, sizeof(Inode), 1, Disk);

		if (type_check(dir_table[i].name) == Directory) {
			color(11);
			printf("%s\t", dir_table[i].name);
		}
		else if (strcmp(name_suffix, ".zip") == 0) {
			color(12);
			printf("%s\t", dir_table[i].name);
		}
		else if (tempInode.access[2] == 1){
			color(10);
			printf("%s\t", dir_table[i].name);
		}
		else{
			color(7);
			printf("%s\t", dir_table[i].name);
		}
		//printf("%d  ", type_check(dir_table[i].name));
		if (!((i+1) % 5)) printf("\n");//5��һ��
	}
	if (dir_num % 5 != 0) printf("\n");
	color(7);
	return 1;
}



/*������Ŀ¼*/
int enter_child_dir(int inode, char* name)
{
	if (type_check(name) != Directory) {
		return -1;
	}

	int child;
	child = check_name(inode, name);

	/*�رյ�ǰĿ¼,������һ��Ŀ¼*/
	close_dir(inode);
	inode_num = child;
	open_dir(child);

	return 1;
}

int adjust_dir(char* name)
{
	int pos;
	for (pos = 0; pos<dir_num; ++pos) {
		/*���ҵ���ɾ����Ŀ¼��λ��*/
		if (strcmp(dir_table[pos].name, name) == 0)
			break;
	}
	for (pos++; pos<dir_num; ++pos) {
		/*pos֮���Ԫ�ض���ǰ�ƶ�һλ*/
		dir_table[pos - 1] = dir_table[pos];
	}

	dir_num--;
	return 1;
}

/*�ݹ�ɾ���ļ���*/
int del_file(int inode, char* name, int deepth)
{
	int 	child, i, t;
	Inode	temp;

	if (!strcmp(name, ".") || !strcmp(name, "..")) {
		/*������ɾ��.��..*/
		printf("rmdir: failed to remove '%s': Invalid argument\n", name);
		return -1;
	}

	child = check_name(inode, name);

	/*��ȡ��ǰ��Ŀ¼��Inode�ṹ*/
	fseek(Disk, InodeBeg + sizeof(Inode)*child, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	if (temp.type == File) {
		/*������ļ����ͷ���ӦInode����*/
		free_inode(child);
		/*�������ϲ��ļ��������Ŀ¼*/
		if (deepth == 0) {
			adjust_dir(name);
		}
		return 1;
	}
	else {
		/*���������Ŀ¼*/
		enter_child_dir(inode, name);
	}

	for (i = 2; i<dir_num; ++i) {
		del_file(child, dir_table[i].name, deepth + 1);
	}

	enter_child_dir(child, "..");//�����ϲ�Ŀ¼
	free_inode(child);

	if (deepth == 0) {
		/*ɾ��������Ŀ¼�е�����*/
		if (dir_num / DirPerBlk != (dir_num - 1) / DirPerBlk) {
			/*�д��̿�����ͷ�*/
			curr_inode.blk_num--;
			t = curr_inode.blk_identifier[curr_inode.blk_num];
			free_blk(t);//�ͷ���Ӧ�Ĵ��̿�
		}
		adjust_dir(name);//��Ϊ�����ڷ�ĩβ��ɾ�������Ҫ�ƶ�dir_table������
	}/*�ǳ�ʼĿ¼ֱ���ͷ�Inode*/

	return 1;
}

int enter_dir(char* name)
{
	int tmp = inode_num;//��¼ԭʼinode�ڵ�
	char tmpPath[40], nameCopy[30];
	char dst[30][NameLength];

	strcpy(tmpPath, path);
	strcpy(nameCopy, name);
	int cnt = split(dst, nameCopy, "/");
	if (name[0] == '/') {//�Ӹ�Ŀ¼��ʼ
		 //printf("1111111111\n");
		close_dir(inode_num);
		inode_num = 0;
		open_dir(inode_num);
		strcpy(path, "monitor@root:");
	}
	for (int i = 0; i < cnt; i++) {
		//printf("%d\n", i);
		int res = enter_child_dir(inode_num, dst[i]);
		change_path(dst[i]);
		if (res == -1) {
			inode_num = tmp;
			open_dir(inode_num);
			strcpy(path, tmpPath);
			return -1;
		}
	}
	return 0;
}

int remove_file(int inode, char* name, int deepth, int type)
{
	char original_name_path[30];
	int original_inode = inode_num;//��¼��ǰ��inode

	strcpy(original_name_path, name);
	if (eat_path(name) == -1) {
		if (type == Directory)
			printf("rmdir: failed to remove��%s��: No such file or directory\n", original_name_path);
		if (type == File)
			printf("rm: cannot remove��%s��: No such file or directory\n", original_name_path);
		return -1;
	}

	int check_type_result = type_check(name);
	if (check_type_result == -1) {//Ҫɾ�����ļ�������
		if (type == Directory)
			printf("rmdir: failed to remove '%s': No such file or directory\n", original_name_path);
		if (type == File)
			printf("rm: cannot remove '%s': No such file or directory\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	Inode father_inode;
	/*��ȡҪɾ����Ŀ¼���ļ��ĸ�Ŀ¼��Inode*/
	fseek(Disk, InodeBeg + sizeof(Inode)*inode_num, SEEK_SET);
	fread(&father_inode, sizeof(father_inode), 1, Disk);

	if (father_inode.access[1] == 0) { //Ҫɾ����Ŀ¼���ļ��ĸ�Ŀ¼������д
		if (type == Directory)
			printf("rmdir: failed to remove ��%s��: Permission denied\n", original_name_path);
		if (type == File)
			printf("rm: cannot remove��%s��: Permission denied\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	if (check_type_result == Directory && type == File) {//ɾ�����ļ��������Ӧָ������磺������rmɾ��Ŀ¼
		printf("rm: cannot remove '%s': Not a file\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}
	if (check_type_result == File && type == Directory) {//ɾ�����ļ��������Ӧָ�������:��rmdirɾ���ļ�
		printf("rmdir: failed to remove '%s': Not a directory\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	del_file(inode_num, name, 0);

	close_dir(inode_num);
	inode_num = original_inode;
	open_dir(inode_num);
	return 0;
}

void color(const unsigned short color1)
{        /*���޸ı�0-15����ɫ;�����0-15��ôʵ��������ɫ   ��Ϊ�������15����ĸı�����ı�����ɫ��*/
	if (color1 >= 0 && color1 <= 15)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color1);
	/*�������0-15�ķ�Χ��ɫ����ô��ΪĬ�ϵ���ɫ��ɫ��*/
	else
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}