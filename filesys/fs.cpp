#include "fs.h"
#include <stdio.h>
#include <string.h>
#include "const.h"
#include "inode.h"
#include "superblk.h"
#include "dir.h"

extern Dir 	dir_table[MaxDirNum];//����ǰĿ¼�ļ������ݶ������ڴ�
extern int 	dir_num;//��Ӧ��ŵ�Ŀ¼����
extern int	 	inode_num;//��ǰĿ¼��inode���
extern Inode 	curr_inode;//��ǰĿ¼��inode�ṹ
extern SuperBlk	super_blk;//�ļ�ϵͳ�ĳ�����
extern FILE*	Disk;
extern char	path[40];

int init_fs(void)
{
	fseek(Disk, SuperBeg, SEEK_SET);
	fread(&super_blk, sizeof(SuperBlk), 1, Disk);//��ȡ������

	inode_num = 0;//��ǰ��Ŀ¼��inodeΪ0

	if (!open_dir(inode_num)) {
		printf("CANT'T OPEN ROOT DIRECTORY\n");
		return 0;
	}

	return 1;
}

int close_fs(void)
{
	fseek(Disk, SuperBeg, SEEK_SET);
	fwrite(&super_blk, sizeof(SuperBlk), 1, Disk);

	close_dir(inode_num);
	return 1;
}

int format_fs(void)
{
	/*��ʽ��inode_map,������Ŀ¼*/
	memset(super_blk.inode_map, 0, sizeof(super_blk.inode_map));
	super_blk.inode_map[0] = 1;
	super_blk.inode_used = 1;
	/*��ʽ��blk_map,������һ�����̿����Ŀ¼*/
	memset(super_blk.blk_map, 0, sizeof(super_blk.blk_map));
	super_blk.blk_map[0] = 1;
	super_blk.blk_used = 1;

	inode_num = 0;//����ǰĿ¼��Ϊ��Ŀ¼

				  /*��ȡ��Ŀ¼��i�ڵ�*/
	fseek(Disk, InodeBeg, SEEK_SET);
	fread(&curr_inode, sizeof(Inode), 1, Disk);
	//	printf("%d\n",curr_inode.file_size/sizeof(Dir));

	curr_inode.file_size = 2 * sizeof(Dir);
	curr_inode.blk_num = 1;
	curr_inode.blk_identifier[0] = 0;//��������һ���Ǹ�Ŀ¼��
	curr_inode.type = Directory;
	curr_inode.access[0] = 1;//�ɶ�
	curr_inode.access[1] = 1;//��д
	curr_inode.access[2] = 1;//��ִ��
	curr_inode.i_atime = time(NULL);
	curr_inode.i_ctime = time(NULL);
	curr_inode.i_mtime = time(NULL);

	fseek(Disk, InodeBeg, SEEK_SET);
	fwrite(&curr_inode, sizeof(Inode), 1, Disk);

	dir_num = 2;//��.��..Ŀ¼����Ч

	strcpy(dir_table[0].name, ".");
	dir_table[0].inode_num = 0;
	strcpy(dir_table[1].name, "..");
	dir_table[1].inode_num = 0;

	strcpy(path, "monster@root:");

	return 1;
}