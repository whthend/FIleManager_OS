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

extern Dir 	dir_table[MaxDirNum];//将当前目录文件的内容都载入内存
extern int 	dir_num;//相应编号的目录项数
extern int	 	inode_num;//当前目录的inode编号
extern FILE*	Disk;
extern Inode 	curr_inode;//当前目录的inode结构
extern SuperBlk	super_blk;//文件系统的超级块
extern FILETIME BuffModifyTimeBeforeEdit;
 FILETIME BuffModifyTimeAfterEdit;
char	path[40];




