#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include "login.h"
#include "const.h"
#include "inode.h"
#include "superblk.h"
#include "dir.h"
#include "zip.h"
#include "fs.h"
#include "file.h"
#include "block.h"

extern Dir 	dir_table[MaxDirNum];//����ǰĿ¼�ļ������ݶ������ڴ�
extern int 	dir_num;//��Ӧ��ŵ�Ŀ¼����
extern int	 	inode_num;//��ǰĿ¼��inode���
extern FILE*	Disk;
extern Inode 	curr_inode;//��ǰĿ¼��inode�ṹ
extern SuperBlk	super_blk;//�ļ�ϵͳ�ĳ�����
extern FILETIME BuffModifyTimeBeforeEdit;
 FILETIME BuffModifyTimeAfterEdit;
char	path[40];




