#pragma once
#ifndef INODE_H
#define INODE_H
#include <time.h>
#include "const.h"

typedef struct {
	int blk_identifier[BlkPerNode];//ռ�õĴ��̿���
	int blk_num;//ռ�õĴ��̿���Ŀ
	int file_size;//�ļ��Ĵ�С
	int type;//�ļ�������
	int access[3];//Ȩ��
	time_t i_atime;//������ʱ��
	time_t i_mtime;//����޸�(modify)ʱ��
	time_t i_ctime;//���ı�(change)ʱ��
}Inode;


int free_inode(int);//�ͷ���Ӧ��inode
int apply_inode();//����inode,������Ӧ��inode�ţ�����-1��INODE����
int init_dir_inode(int, int);//��ʼ���½�Ŀ¼��inode
int init_file_inode(int);//��ʼ���½��ļ���inode

#endif
