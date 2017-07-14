#include <stdio.h>
#include <string.h>
#include "inode.h"
#include "const.h"
#include "superblk.h"
#include "block.h"
#include "dir.h"

extern SuperBlk	super_blk;//�ļ�ϵͳ�ĳ�����
extern FILE*	Disk;
extern int		user_num;

/*����inode*/
int apply_inode()
{
	int i;

	if (super_blk.inode_used >= InodeNum) {
		return -1;//inode�ڵ�����
	}

	super_blk.inode_used++;

	for (i = 0; i<InodeNum; ++i) {
		if (!super_blk.inode_map[i]) {//�ҵ�һ���յ�i�ڵ�
			super_blk.inode_map[i] = 1;
			return i;
		}
	}
	return 0;
}

int free_inode(int inode)
{
	Inode 	temp;
	int 	i;
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	for (i = 0; i<temp.blk_num; ++i) {
		free_blk(temp.blk_identifier[i]);
	}

	super_blk.inode_map[inode] = 0;
	super_blk.inode_used--;

	return 1;
}

/*��ʼ���½��ļ���indoe*/
int init_file_inode(int inode)
{
	Inode temp;
	/*��ȡ��Ӧ��Inode*/
	//fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	//fread(&temp, sizeof(Inode), 1, Disk);

	temp.blk_num = 0;
	temp.type = File;
	temp.file_size = 0;
	temp.access[0][user_num] = 1;//�ɶ�
	temp.access[1][user_num] = 1;//��д
	temp.access[2][user_num] = 0;//����ִ��
	temp.i_atime = time(NULL);
	temp.i_ctime = time(NULL);
	temp.i_mtime = time(NULL);
	//for (int i = 0; i < 3; i++) {
	//	for (int j = 0; j < 9; j++)
	//	{
	//		printf("%d", temp.access[i][j]);
	//	}
	//	printf("\n");
	//}

	/*���Ѿ���ʼ����Inodeд��*/
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fwrite(&temp, sizeof(Inode), 1, Disk);

	return 1;
}


/*��ʼ���½�Ŀ¼��inode*/
int init_dir_inode(int child, int father)
{
	Inode	temp;
	Dir 	dot[2];
	int		blk_pos;

	/*fseek(Disk, InodeBeg + sizeof(Inode)*child, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);*/

	blk_pos = get_blk();//��ȡ�´��̿�ı��

	temp.blk_num = 1;
	temp.blk_identifier[0] = blk_pos;
	temp.type = Directory;
	//printf("temp.type = %d\n", temp.type);
	temp.file_size = 2 * sizeof(Dir);
	temp.access[0][user_num] = 1;//�ɶ�
	temp.access[1][user_num] = 1;//��д
	temp.access[2][user_num] = 1;//��ִ��
	temp.i_atime = time(NULL);
	temp.i_ctime = time(NULL);
	temp.i_mtime = time(NULL);

	//for (int i = 0; i < 3; i++) {
	//	for (int j = 0; j < 9; j++)
	//	{
	//		printf("%d", temp.access[i][j]);
	//	}
	//	printf("\n");
	//}

	fseek(Disk, InodeBeg + sizeof(Inode)*child, SEEK_SET);
	fwrite(&temp, sizeof(Inode), 1, Disk);

	strcpy(dot[0].name, ".");//ָ��Ŀ¼����
	dot[0].inode_num = child;

	strcpy(dot[1].name, "..");
	dot[1].inode_num = father;

	/*����Ŀ¼������д�����ݿ�*/
	fseek(Disk, BlockBeg + BlkSize*blk_pos, SEEK_SET);
	fwrite(dot, sizeof(Dir), 2, Disk);

	return 1;
}