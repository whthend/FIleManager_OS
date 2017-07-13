#pragma once
#ifndef DIR_H
#define DIR_H

typedef struct {
	char name[30];//Ŀ¼��
	short  inode_num;//Ŀ¼��Ӧ��inode
}Dir;

int open_dir(int);//����Ӧinode��Ӧ��Ŀ¼
int close_dir(int);//������Ӧinode��Ŀ¼
int show_dir(int);//��ʾĿ¼
int make_file(int, char*, int);//�����µ�Ŀ¼���ļ�
int del_file(int, char*, int);//ɾ����Ŀ¼�����ļ�, remove_file���ӳ��򣬲���������
int remove_file(int inode, char* name, int deepth, int type);//ɾ����Ŀ¼�����ļ�
int enter_child_dir(int, char*);//������Ŀ¼
int enter_dir(char* name);//��������Ŀ¼
int enter_dir_first(char* name);
int adjust_dir(char*);//ɾ����Ŀ¼�󣬵���ԭĿ¼��ʹ�м��޿�϶
void color(const unsigned short color1);//��������ַ���ɫ

#endif