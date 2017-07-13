#pragma once
#ifndef CONST_H
#define CONST_H

#define InodeNum		1024//i�ڵ���Ŀ
#define BlkNum			(80*1024)//���̿����Ŀ
#define BlkSize			1024//���̿��СΪ1K
#define BlkPerNode		1024//ÿ���ļ����������Ĵ��̿���Ŀ
#define DISK 			"disk.txt"
#define BUFF			"buff.txt"//��д�ļ�ʱ�Ļ����ļ�
#define SuperBeg		0//���������ʼ��ַ
#define InodeBeg		sizeof(SuperBlk)//i�ڵ�����ʼ��ַ
#define BlockBeg		(InodeBeg+InodeNum*sizeof(Inode))//��������ʼ��ַ
#define MaxDirNum		(BlkPerNode*(BlkSize/sizeof(Dir)))//ÿ��Ŀ¼�����ļ���
#define DirPerBlk		(BlkSize/sizeof(Dir))//ÿ�����̿���������Ŀ¼��
#define Directory		0
#define File			1
#define CommanNum		(sizeof(command)/sizeof(char*))//ָ����Ŀ
#define NameLength		30 //�ļ�������
#define ValueSize		256 //Ȩֵͳ���У�һ���ֽ���0~255�ֿ���ȡֵ
#define HaffCodeLen		128//�����ַ��Ĺ���������������볤
#define ZipMagicNumber	0xA8EF1314//ħ��
#define MagicSize		sizeof(ZipMagicNumber)//ħ������

#endif