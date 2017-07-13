#pragma once
#ifndef ZIP_H
#define ZIP_H
#include <stdio.h>
#include "const.h"
#include "inode.h"

typedef struct Hnode//�������ڵ�
{
	int weight;
	int date;
	struct Hnode *lch, *rch;
	struct Hnode * next;
}node, *link;

typedef struct Mnode//�����������ڵ�
{
	int ch;
	char s[HaffCodeLen];
}mapnode, *maptr;

int zip(char* zip_name, char* name);//zipָ��ʵ�֣��ļ�ѹ��
int unzip(char* name);//�ļ���ѹ
void StaFrequency();//ͳ��Ƶ��
link CreatFreQueue();//Ƶ�ʶ���
link CreatHuff();//������������
void HuffCode(link root, char s[]);//���й���������
void Compression(char* name);//ѹ���ļ������ļ��е��ַ�ת����01�ַ������ٽ��ַ���ת����int
int DeCompression();//��ѹ�ļ�
void PrintfTree(link root, FILE *fp);//��ӡ��������
link InputTree(FILE *fp);//��ȡ�ָ���������
int zip_write(char* name);//ѹ���������д���Ӧ��ѹ���ļ���
int getCompressionRatio(int inodeBefore, int inodeAfter);//���ѹ����
int unzip_write(char* name);//����ѹ�������д���ѹ�ļ�

#endif