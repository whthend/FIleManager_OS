#include "const.h"
#include "superblk.h"
#include <stdio.h>

extern SuperBlk	super_blk;//�ļ�ϵͳ�ĳ�����
/*����δ��ʹ�õĴ��̿�*/
int get_blk()
{
	int i;
	super_blk.blk_used++;
	for (i = 0; i<BlkNum; ++i) {//�ҵ�δ��ʹ�õĿ�
		if (!super_blk.blk_map[i]) {
			super_blk.blk_map[i] = 1;
			return i;
		}
	}

	return -1;//û�ж���Ĵ��̿�
}


/*�ͷŴ��̿�*/
int free_blk(int blk_pos)
{
	super_blk.blk_used--;
	super_blk.blk_map[blk_pos] = 0;
	return 0;
}

void show_disk_usage()//��ʾ����ʹ�����
{
	printf("Filesystem\t1K-blocks\tUsed\tAvailable\tUse%%\tMounted on\n");
	printf("%s\t%d\t\t%d\t%d\t\t%d\t%s\n", "myFileSystem", BlkNum, super_blk.blk_used,
		BlkNum - super_blk.blk_used, super_blk.blk_used * 100 / BlkNum, "/");
}

void show_inode_usage()//��ʾinode�ڵ�ʹ�����
{
	printf("Filesystem\tInodes\tIUsed\tIFree\tIUse%%\tMounted on\n");
	printf("%s\t%d\t%d\t%d\t%d\t%s\n", "myFileSystem", InodeNum, super_blk.inode_used,
		InodeNum - super_blk.inode_used, super_blk.inode_used * 100 / InodeNum, "/");
}