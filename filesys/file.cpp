#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include "file.h"
#include "inode.h"
#include "path.h"
#include "dir.h"
#include "const.h"
#include "superblk.h"
#include "block.h"

extern Dir		dir_table[MaxDirNum];//����ǰĿ¼�ļ������ݶ������ڴ�
extern int	 	inode_num;//��ǰĿ¼��inode���
extern FILE*	Disk;
extern int 		dir_num;//��Ӧ��ŵ�Ŀ¼����
extern char		path[40];
extern FILETIME BuffModifyTimeBeforeEdit;
extern FILETIME BuffModifyTimeAfterEdit;
extern int		user_num;

FILETIME getBuffModifyTime();//��ȡ��ʱ�ļ����ļ��޸�ʱ��
/*���ļ�����*/
int file_read(char* name)
{
	int 	inode, i, blk_num;
	Inode	temp;
	FILE*	fp = fopen(BUFF, "wb+");
	char 	buff[BlkSize];
	//printf("read\n");

	inode = check_name(inode_num, name);

	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	int file_size = temp.file_size;
	if (temp.access[0][user_num] == 0) { //�ļ����ɶ�����ֱ���˳�
		return -1;
	}
	if (temp.blk_num == 0) {//���Դ�ļ�û������,��ֱ���˳�
		fclose(fp);
		return 0;
	}
	printf("read\n");
	for (i = 0; i<temp.blk_num - 1; ++i) {
		blk_num = temp.blk_identifier[i];
		/*�����ļ������Ĵ��̿�*/
		fseek(Disk, BlockBeg + BlkSize*blk_num, SEEK_SET);
		fread(buff, sizeof(char), BlkSize, Disk);
		/*д��BUFF*/
		fwrite(buff, sizeof(char), BlkSize, fp);
		file_size -= BlkSize;
	}

	/*���һ����̿����δ��*/
	blk_num = temp.blk_identifier[i];
	fseek(Disk, BlockBeg + BlkSize*blk_num, SEEK_SET);
	fread(buff, sizeof(char), file_size, Disk);
	fwrite(buff, sizeof(char), file_size, fp);

	/*�޸�inode��Ϣ*/
	temp.i_atime = time(NULL);

	/*���޸ĺ��Inodeд��*/
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fwrite(&temp, sizeof(Inode), 1, Disk);

	fclose(fp);
	return 0;
}


/*д�ļ�����*/
int file_write(char* name)
{
	int 	inode;
	int		num, blk_num;
	FILE* 	fp = fopen(BUFF, "rb");
	Inode	temp;
	char	buff[BlkSize];

	inode = check_name(inode_num, name);

	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	if (temp.access[1][user_num] == 0) { //�ļ�����д
		printf("%d\n", temp.access[1][user_num]);
		return -1;
	}

	//��ԭ�ļ����ݶ�Ӧ�ش��̿��ͷţ��ļ���С��Ϊ0��
	//Ȼ������������̿����޸ĺ���ļ����ݣ������ļ��Ĵ�С���޸�ʱ�䣬���Ըı�ʱ��
	temp.blk_num = 0;
	temp.file_size = 0;
	
	while (num = fread(buff, sizeof(char), BlkSize, fp)) {
		printf("num:%d\n", num);
		if ((blk_num = get_blk()) == -1) {
			printf("error:	block has been used up\n");
			break;
		}
		/*�ı�Inode�ṹ����Ӧ״̬*/
		temp.blk_identifier[temp.blk_num++] = blk_num;
		temp.file_size += num;	
		printf("file_size:%d\n blocks: %d\n", temp.file_size, temp.blk_num);
		/*������д�ش��̿�*/
		fseek(Disk, BlockBeg + BlkSize*blk_num, SEEK_SET);
		fwrite(buff, sizeof(char), num, Disk);
	}
	temp.i_mtime = time(NULL);
	temp.i_ctime = time(NULL);


	/*���޸ĺ��Inodeд��*/
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fwrite(&temp, sizeof(Inode), 1, Disk);

	/*
	Inode temp2;
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp2, sizeof(Inode), 1, Disk);*/

//	printf("file_size:%d\n blocks: %d\n", temp2.file_size, temp2.blk_num);

	fclose(fp);
	return 1;
}

int file_copy(char* name, char* cpname)
{
	int originalInode = inode_num;//��¼��ǰ��inode
	int source_inode_num, dest_inode_num;
	char originalNamePath[30];
	char originalCpNamePath[30];

	strcpy(originalNamePath, name);
	strcpy(originalCpNamePath, cpname);

	if (eat_path(name) == -1) {
		printf("cp: cannot stat ��%s��: No such file or directory\n", originalNamePath);
		return -1;
	}
	if (type_check(name) != File) {
		printf("cp: cannot copy '%s': Not a file\n", originalNamePath);
		close_dir(inode_num);
		inode_num = originalInode;
		open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
		return -1;
	}

	if (file_read(name) == -1) {//��ԭ�ļ����ɶ�
		printf("cp: cannot open '%s' for reading: Permission denied\n", originalNamePath);
		close_dir(inode_num);
		inode_num = originalInode;
		open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
		return -1;
	}
	source_inode_num = inode_num;//��¼ԭ�ļ��ĸ�Ŀ¼�ڵ�

	close_dir(inode_num);
	inode_num = originalInode;
	open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��

	if (eat_path(cpname) == -1) {
		printf("cp: cannot stat ��%s��: No such file or directory\n", originalCpNamePath);
		return -1;
	}
	dest_inode_num = inode_num;//��¼Ŀ��Ŀ¼�ĸ�Ŀ¼�ڵ�

	if (source_inode_num == dest_inode_num && strcmp(name, cpname) == 0) {
		printf("cp: '%s' and '%s' are the same file\n", originalNamePath, originalCpNamePath);
		close_dir(inode_num);
		inode_num = originalInode;
		open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
		return -1;
	}

	Inode temp;

	/*��ԭ�ļ���������һ�ļ���*/
	if (type_check(cpname) != Directory) {
		if (check_name(inode_num, cpname) != -1) {//Ŀ���ļ��Ѿ����ڣ������ݿ�����ȥ
			if (file_write(cpname) == -1) { //��Ŀ���ļ�����д
				printf("cp: cannot create regular file��%s��: Permission denied\n", originalCpNamePath);
				close_dir(inode_num);
				inode_num = originalInode;
				open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
				return -1;
			}
		}
		else {									 //Ŀ���ļ������ڣ������ļ����������ݿ�����ȥ
			/*��ȡĿ��Ŀ¼��inode�ڵ㣬�жϸ�Ŀ¼�Ƿ��д*/
			fseek(Disk, InodeBeg + sizeof(Inode)*dest_inode_num, SEEK_SET);
			fread(&temp, sizeof(Inode), 1, Disk);

			if (temp.access[1][user_num] == 0) { //Ŀ��Ŀ¼����д
				printf("cp: cannot create regular file ��%s��: Permission denied\n", originalCpNamePath);
				close_dir(inode_num);
				inode_num = originalInode;
				open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
				return -1;
			}
			make_file(inode_num, cpname, File);
			file_write(cpname);//�����ݴ�BUFFд���ļ�
		}		
	}
	/*��ԭ�ļ�������ĳ����Ŀ¼�£�������ǰĿ¼(.), ԭĿ¼�ĸ�Ŀ¼��..��*/
	else {
		if (source_inode_num == dest_inode_num && strcmp(cpname, ".") == 0) {//����ǰĿ¼�µ�ĳ���ļ����Ƶ���ǰĿ¼����ʾ�Ѿ�����
			printf("cp: '%s' and '%s/%s' are the same file\n", originalNamePath, originalCpNamePath, name);
			return -1;
		}

		enter_child_dir(inode_num, cpname);//������Ŀ¼��Ŀ¼

		/*��¼ԭĿ¼��*/
		int pos = strlen(path) - 1; 
		for (; pos >= 0; --pos) {
			if (path[pos] == '/') {
				break;
			}
		}
		char curDirName[30];
		int i = 0;
		for (pos = pos + 1; pos <= strlen(path); ++pos)
			curDirName[i++] = path[pos];
		//printf("%s\n", curDirName);

		if (check_name(inode_num, name) != -1) {//�Ƿ�����ͬ���ļ�	
			file_write(name);//�����ݴ�BUFFд���ļ�
		}
		else {
			/*��ȡĿ��Ŀ¼��inode�ڵ㣬�жϸ�Ŀ¼�Ƿ��д*/
			fseek(Disk, InodeBeg + sizeof(Inode)*inode_num, SEEK_SET);
			fread(&temp, sizeof(Inode), 1, Disk);

			if (temp.access[1][user_num] == 0) { //Ŀ��Ŀ¼����д
				printf("cp: cannot create regular file ��./%s��: Permission denied\n", name);
				close_dir(inode_num);
				inode_num = originalInode;
				open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
				return -1;
			}

			make_file(inode_num, name, File);
			file_write(name);//�����ݴ�BUFFд���ļ�
		}
		/*����ԭĿ¼*/
	
		if (strcmp(cpname, "..") == 0)
			enter_child_dir(inode_num, curDirName);//������Ƶ���Ŀ¼�������귵��ԭĿ¼
		else
			enter_child_dir(inode_num, "..");//������Ƶ���Ŀ¼�������귵�ص���Ŀ¼��".."Ŀ¼����ԭĿ¼
	}

	close_dir(inode_num);
	inode_num = originalInode;
	open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
	return 0;
}

int file_move(char* name, char* mvname)
{
	int originalInode = inode_num;//��¼��ǰ��inode
	int source_inode_num, dest_inode_num;
	int inode;//ԭ�ļ���inode
	char originalNamePath[30];
	char originalMvNamePath[30];

	strcpy(originalNamePath, name);
	strcpy(originalMvNamePath, mvname);

	if (eat_path(name) == -1) {
		printf("mv: cannot stat ��%s��: No such file or directory\n", originalNamePath);
		return -1;
	}
	if (type_check(name) != File) {
		printf("mv: cannot move '%s': Not a file or no exist\n", originalNamePath);
		close_dir(inode_num);
		inode_num = originalInode;
		open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
		return -1;
	}

	source_inode_num = inode_num;//��¼ԭ�ļ��ĸ�Ŀ¼�ڵ�

	close_dir(inode_num);
	inode_num = originalInode;
	open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��

	if (eat_path(mvname) == -1) {
		printf("mv: cannot stat ��%s��: No such file or directory\n", originalMvNamePath);
		return -1;
	}
	dest_inode_num = inode_num;//��¼Ŀ��Ŀ¼�ĸ�Ŀ¼�ڵ�

	if (source_inode_num == dest_inode_num && strcmp(name, mvname) == 0) {
		printf("mv: '%s' and '%s' are the same file\n", originalNamePath, originalMvNamePath);
		close_dir(inode_num);
		inode_num = originalInode;
		open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
		return -1;
	}

	Inode temp;
	/*��ȡԭ�ļ�Ŀ¼��inode�ڵ㣬�жϸ�Ŀ¼�Ƿ��д*/
	fseek(Disk, InodeBeg + sizeof(Inode)*source_inode_num, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	if (temp.access[1][user_num] == 0) { //ԭ�ļ�Ŀ¼����д
		if (type_check(mvname) == Directory)
			printf("mv: cannot move ��%s�� to ��%s/%s��: Permission denied\n", originalNamePath, originalMvNamePath, name);
		else 
			printf("mv: cannot move ��%s�� to ��%s��: Permission denied\n", originalNamePath, originalMvNamePath);
		close_dir(inode_num);
		inode_num = originalInode;
		open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
		return -1;
	}

	/*���磺mv a.txt b.txt*/
	if (type_check(mvname) == -1) {  //���b.txt������

		/*��ȡb.txt�ĸ�Ŀ¼��inode���ж��Ƿ��д*/
		fseek(Disk, InodeBeg + sizeof(Inode)*dest_inode_num, SEEK_SET);
		fread(&temp, sizeof(temp), 1, Disk);

		if (temp.access[1][user_num] == 0) { //b.txt�ĸ�Ŀ¼����д
			printf("mv: cannot move ��%s�� to ��%s��: Permission denied\n", originalNamePath, originalMvNamePath);
			close_dir(inode_num);
			inode_num = originalInode;
			open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
			return -1;
		}

		if (source_inode_num == dest_inode_num) {//���b.txt�����ڣ���a.txt �� b.txt ��ͬһ��Ŀ¼�£����൱��a.txt������
			for (int pos = 0; pos < dir_num; ++pos) {
				if (strcmp(dir_table[pos].name, name) == 0)
					strcpy(dir_table[pos].name, mvname);
			}
		}
		else {//���b.txt�����ڣ���a.txt �� b.txt ����ͬһ��Ŀ¼�£���a.txt��Ŀ¼��ɾ��������b.txt��Ŀ¼��ָ��a.txt��inode
			inode_num = source_inode_num;
			open_dir(source_inode_num);//����a.txt��Ŀ¼λ��
			inode = check_name(inode_num, name);
			adjust_dir(name);//ɾ��a.txt��Ŀ¼��

			close_dir(inode_num);
			inode_num = dest_inode_num;
			open_dir(dest_inode_num);//����b.txt��Ŀ¼λ��
			strcpy(dir_table[dir_num].name, mvname);
			dir_table[dir_num++].inode_num = inode;
		}
	}
	else if (type_check(mvname) == File) {//���b.txtΪ�ļ�
		if (source_inode_num == dest_inode_num) {//��b.txt �� a.txt��ͬһ��Ŀ¼�£���ɾ��b.txt��Ŀ¼�ͬʱ��a.txt��Ŀ¼��������
			adjust_dir(mvname);						//b.txt �� a.txt��ͬһ��Ŀ¼�£���b.txt�ĸ�Ŀ¼Ȩ����ǰ���Ѿ��жϣ��ǿ�д��
			for (int pos = 0; pos < dir_num; ++pos) {
				if (strcmp(dir_table[pos].name, name) == 0)
					strcpy(dir_table[pos].name, mvname);
			}
		}
		else {//b.txt �Ѿ����� �� a.txt��b.txt����ͬһĿ¼����a.txt��Ŀ¼��ɾ�����޸�b.txt��Ŀ¼��ָ��a.txt��inode
			inode_num = source_inode_num;
			open_dir(source_inode_num);//����a.txt��Ŀ¼λ��
			inode = check_name(inode_num, name);
			adjust_dir(name);//ɾ��a.txt��Ŀ¼��

			close_dir(inode_num);
			inode_num = dest_inode_num;
			open_dir(dest_inode_num);//����b.txt��Ŀ¼λ��
			for (int pos = 0; pos < dir_num; ++pos) {
				if (strcmp(dir_table[pos].name, mvname) == 0)
					dir_table[pos].inode_num = inode;
			}
		}
	}
	else {  //���b.txtΪĿ¼
		if (source_inode_num == dest_inode_num && strcmp(mvname, ".") == 0) {
			printf("mv: '%s' and '%s/%s' are the same file\n", originalNamePath, originalMvNamePath, name);
			return -1;//�ƶ�����Ŀ¼�£�������Ҫ�ƶ�	
		}
		//���b.txt��Ŀ¼�����b.txtĿ¼�У�����һ��Ŀ¼��ָ��a.txt, �ٽ�a.txt��Ŀ¼��ɾ����

		inode_num = source_inode_num;
		open_dir(source_inode_num);//����a.txt��Ŀ¼λ��
		inode = check_name(inode_num, name);//��¼a.txt��inode

		close_dir(inode_num);
		inode_num = dest_inode_num;
		open_dir(dest_inode_num);//����b.txt��Ŀ¼λ��

		enter_child_dir(inode_num, mvname);
		if (check_name(inode_num, mvname) == -1) { //b.txtĿ¼�²�������a.txt��������Ŀ���򴴽�һ��Ŀ¼��ָ��a.txt

            /*��ȡb.txt��inode���ж��Ƿ��д*/
			fseek(Disk, InodeBeg + sizeof(Inode)*inode_num, SEEK_SET);
			fread(&temp, sizeof(Inode), 1, Disk);

			if (temp.access[1][user_num] == 0) { //b.txt����д
				printf("mv: cannot move ��%s�� to ��%s/%s��: Permission denied\n", originalNamePath, originalMvNamePath, name);
				close_dir(inode_num);
				inode_num = originalInode;
				open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
				return -1;
			}

			strcpy(dir_table[dir_num].name, name);
			dir_table[dir_num++].inode_num = inode;
		}
		else { //b.txtĿ¼�´�����a.txt��������Ŀ���޸ĸ���Ŀ��Ŀ¼��ָ��a.txt��inode
			dir_table[check_name(inode, mvname)].inode_num = inode;
		}
		close_dir(inode_num);
		inode_num = source_inode_num;
		open_dir(source_inode_num);//����a.txt��Ŀ¼λ��
		inode = check_name(inode_num, name);
		adjust_dir(name);//ɾ��a.txt��Ŀ¼��
	}

	close_dir(inode_num);
	inode_num = originalInode;
	open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��
	return 0;
}

int show_file_info(char* name) 
{
	int inode;
	Inode temp;
	char original_name_path[30];
	int original_inode = inode_num;//��¼��ǰ��inode

	strcpy(original_name_path, name);
	if (eat_path(name) == -1) {
		printf("stat: cannot stat��%s��: No such file or directory\n", original_name_path);
		return -1;
	}

	inode = check_name(inode_num, name);
	if (inode == -1) {
		printf("stat: cannot stat '%s': No such file or directory\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}
	
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	printf("File: '%s'\n", original_name_path);

	printf("Size: %d\tBlocks: %d\t", temp.file_size, temp.blk_num);
	temp.type == Directory ? printf("type: directory\n") : printf("type: regular file\n");

	printf("Inode: %d\t", inode);
	printf("Access: ");
	temp.access[0][user_num] ? printf("r") : printf("-");
	temp.access[1][user_num] ? printf("w") : printf("-");
	temp.access[2][user_num] ? printf("x") : printf("-");
	printf("\n");
	printf("Access: %s", ctime(&temp.i_atime));
	printf("Modify: %s", ctime(&temp.i_mtime));
	printf("Change: %s", ctime(&temp.i_ctime));

	close_dir(inode_num);
	inode_num = original_inode;
	open_dir(inode_num);
	return 0;
}

/*�޸��ļ���дȨ��*/
int change_mode(char* parameter, char* name)
{
	int inode;
	Inode temp;
	char original_name_path[30];
	int original_inode = inode_num;//��¼��ǰ��inode

	strcpy(original_name_path, name);
	if (eat_path(name) == -1) {
		printf("chmod: cannot access��%s��: No such file or directory\n", original_name_path);
		return -1;
	}
	inode = check_name(inode_num, name);
	if (inode == -1) {
		printf("stat: cannot stat '%s': No such file or directory\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	if (strcmp(parameter, "+r") == 0) {
		temp.access[0][user_num] = 1;
		temp.i_ctime = time(NULL);
	}
	else if (strcmp(parameter, "-r") == 0) {
		temp.access[0][user_num] = 0;
		temp.i_ctime = time(NULL);
	}
	else if (strcmp(parameter, "+w") == 0) {
		temp.access[1][user_num] = 1;
		temp.i_ctime = time(NULL);
	}
	else if (strcmp(parameter, "-w") == 0) {
		temp.access[1][user_num] = 0;
		temp.i_ctime = time(NULL);
	}
	else if (strcmp(parameter, "+x") == 0) {
		temp.access[2][user_num] = 1;
		temp.i_ctime = time(NULL);
	}
	else if (strcmp(parameter, "-x") == 0) {
		temp.access[2][user_num] = 0;
		temp.i_ctime = time(NULL);
	}
	else {
		printf("chmod: invalid option -- '%s'\n", parameter);
	}

	/*���޸ĺ��Inodeд��*/
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fwrite(&temp, sizeof(Inode), 1, Disk);

	close_dir(inode_num);
	inode_num = original_inode;
	open_dir(inode_num);
	return 0;
}

int file_edit(char* name)
{
	char original_name_path[30];
	int original_inode = inode_num;//��¼��ǰ��inode
	strcpy(original_name_path, name);
	if (eat_path(name) == -1) {
		printf("�����ļ�·��������%s��\n", original_name_path);
		return -1;
	}
	if (type_check(name) == Directory) {
		printf("����'%s'�ļ������ڣ�\n", name);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}
	if (type_check(name) == -1) {
		printf("����'%s'�ļ������ڣ�\n", name);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	if (file_read(name) == -1) {//�ļ���ȡʧ��
		printf("�����޶�д��%s���ļ�Ȩ�ޣ�\n", name);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	BuffModifyTimeBeforeEdit = getBuffModifyTime();//�ļ���Ϣ���뵽buff.txt�󣬻�ȡbuff.txt���޸�ʱ�䣬�����ж�����������ڼ��±����Ƿ��޸�
	/*
	SYSTEMTIME *STime = new SYSTEMTIME;
	FileTimeToSystemTime(&BuffModifyTimeBeforeEdit, STime);
	printf("%d-%d-%d-%d-%d-%d\n", STime->wYear, STime->wMonth, STime->wDay, STime->wHour, STime->wMinute, STime->wSecond);
	*/
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LPTSTR szCmdline = _tcsdup(TEXT("notepad.exe buff.txt"));

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// ���ӽ���
	if (!CreateProcess(NULL,   // No module name (use command line)
		szCmdline,      // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return 0;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	Sleep(10);
	BuffModifyTimeAfterEdit = getBuffModifyTime();//buff.txt�رպ󣬻�ȡbuff.txt���޸�ʱ�䣬�����ж�����������ڼ��±����Ƿ��޸�
	/*
	SYSTEMTIME *STime = new SYSTEMTIME;
	FileTimeToSystemTime(&BuffModifyTimeAfterEdit, STime);
	printf("%d-%d-%d-%d-%d-%d\n", STime->wYear, STime->wMonth, STime->wDay, STime->wHour, STime->wMinute, STime->wSecond);
	*/
	if (BuffModifyTimeBeforeEdit.dwLowDateTime == BuffModifyTimeAfterEdit.dwLowDateTime
		&& BuffModifyTimeBeforeEdit.dwHighDateTime == BuffModifyTimeAfterEdit.dwHighDateTime) {
								//��buff.txt���޸�ʱ��û�з����仯����������±�������û�з����ı䣬�򲻱�����д��		
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}
	
	if (file_write(name) == -1) { //�����ݴ�BUFFд���ļ�
		printf("�����ļ�����Ϊֻ�����޷��޸ģ�\n");
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	close_dir(inode_num);
	inode_num = original_inode;
	open_dir(inode_num);
	return 0;
}

FILETIME getBuffModifyTime()
{
	HFILE hFile = _lopen("buff.txt", OF_READ);

	FILETIME fCreateTime, fAccessTime, fWriteTime;
	GetFileTime((HANDLE*)hFile, &fCreateTime, &fAccessTime, &fWriteTime);//��ȡ�ļ�ʱ��
	return fWriteTime;
}


int temp_file_read(char* name)//��ȡ�ļ���Ϣ��������ʱ�ļ���������ִ��ʹ��
{
	int 	inode, i, blk_num;
	Inode	temp;
	char 	buff[BlkSize];
	
	//printf("read\n");

	inode = check_name(inode_num, name);

	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	int file_size = temp.file_size;

	if (temp.access[0][user_num] == 0) { //�ļ����ɶ�����ֱ���˳�������-1
		return -1;
	}

	if (temp.access[2][user_num] == 0) { //�ļ�����ִ�У��򷵻�-2
		return -2;
	}

	if (temp.blk_num == 0) {//���Դ�ļ�û������,��ֱ���˳�
		return 0;
	}

	FILE*	fp = fopen(name, "wb+");
	//printf("read\n");

	for (i = 0; i<temp.blk_num - 1; ++i) {
		blk_num = temp.blk_identifier[i];
		/*�����ļ������Ĵ��̿�*/
		fseek(Disk, BlockBeg + BlkSize*blk_num, SEEK_SET);
		fread(buff, sizeof(char), BlkSize, Disk);
		/*д�뵽��ʱ�ļ�*/
		fwrite(buff, sizeof(char), BlkSize, fp);
		file_size -= BlkSize;
	}

	/*���һ����̿����δ��*/
	blk_num = temp.blk_identifier[i];
	fseek(Disk, BlockBeg + BlkSize*blk_num, SEEK_SET);
	fread(buff, sizeof(char), file_size, Disk);
	fwrite(buff, sizeof(char), file_size, fp);
	
	/*�޸�inode��Ϣ*/
	temp.i_atime = time(NULL);

	/*���޸ĺ��Inodeд��*/
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fwrite(&temp, sizeof(Inode), 1, Disk);

	fclose(fp);
	return 0;
}

int exec(char * name)
{
	return 0;
}

void show_manual()//��ӡ�����ֲ�
{
	FILE *fp = NULL; 
	fp = fopen("man.txt", "rb");

	if (NULL == fp) return;

	char ch;
	while (fscanf(fp, "%c", &ch) != EOF) 
		printf("%c", ch); //���ı��ж��벢�ڿ���̨��ӡ����
	printf("\n");
	fclose(fp);
}

int get_file_size(char *name)
{
	Inode temp;
	int inode = check_name(inode_num, name);

	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	return temp.file_size;
}