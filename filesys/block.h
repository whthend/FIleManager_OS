#pragma once
#ifndef BLOCK_H
#define BLOCK_H

int free_blk(int);//�ͷ���Ӧ�Ĵ��̿�
int get_blk(void);//��ȡ���̿�
void show_disk_usage();//��ʾ����ʹ�����
void show_inode_usage();//��ʾinode�ڵ�ʹ�����

#endif