#pragma once
#ifndef FILE_H
#define FILE_H

int file_write(char*);//д�ļ�
int file_read(char*);//���ļ�
int file_edit(char*);//�༭�ļ�
int file_copy(char* name, char* cpname);//�����ļ�
int file_move(char* name, char* mvname);//�ƶ��ļ�
int show_file_info(char* name);//��ʾ�ļ���Ϣ
int change_mode(char* parameter, char* name);//�ı��ļ�Ȩ��
int temp_file_read(char* name);//��ȡ�ļ���Ϣ��������ʱ�ļ���������ִ��ʹ��
int exec(char* name);//ִ�� ��ִ�е��ļ�
void show_manual();//��ӡ�����ֲ�
int get_file_size(char *name);//��ȡ�ļ���С

#endif