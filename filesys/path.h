#pragma once
#ifndef PATH_H
#define PATH_H
#include "const.h"

void change_path(char*);
int eat_path(char* name);//�����ļ�·����,���뵽����Ŀ¼�£�����name�޸�Ϊ���һ����ļ���
int split(char dst[][NameLength], char* str, const char* spl);//·�����ִ�
int check_name(int, char*);//���������,����-1��ʾ���ֲ����ڣ����򷵻���Ӧinode
int type_check(char*);//ȷ���ļ�������

#endif