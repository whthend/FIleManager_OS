#pragma once
#ifndef SUPERBLK_H
#define SUPERBLK_H
#include "const.h"

typedef struct {
	int inode_map[InodeNum];//i�ڵ�λͼ
	int blk_map[BlkNum];//���̿�λͼ
	int inode_used;//�ѱ�ʹ�õ�i�ڵ���Ŀ
	int blk_used;//�ѱ�ʹ�õĴ��̿���Ŀ
}SuperBlk;

#endif
