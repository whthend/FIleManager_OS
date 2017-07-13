#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "zip.h"
#include "path.h"
#include "dir.h"
#include "file.h"
#include "block.h"
#include "const.h"
#include "superblk.h"

maptr map;//�����������
int len;//�����������ı�
int count = 0;
int weight[ValueSize] = { 0 };//Ȩֵͳ��
int supple;//01������λ��
char FILENAME[NameLength];//��ѹ�����ļ����ļ���
Inode unzip_file_inode;//��ѹ�����ļ���inode��Ϣ

extern int inode_num;//��ǰĿ¼��inode���
extern FILE* Disk;
extern Dir 	dir_table[MaxDirNum];//����ǰĿ¼�ļ������ݶ������ڴ�
extern int 	dir_num;//��Ӧ��ŵ�Ŀ¼����
extern SuperBlk	super_blk;//�ļ�ϵͳ�ĳ�����
extern Inode curr_inode;//��ǰĿ¼��inode�ṹ

int zip(char* zip_name, char* name)
{
	char original_name_path[NameLength];
	char original_zip_name_path[NameLength];
	int original_inode = inode_num;//��¼��ǰ��inode
	int inode_source_file, inode_zip_file;
	
	strcpy(original_name_path, name); //��¼name
	strcpy(original_zip_name_path, zip_name);//��¼zip_name

	if (eat_path(name) == -1) {//·����������
		printf("zip warning: name not matched: %s\n", original_name_path);
		printf("zip error: Nothing to do!(%s)\n", original_zip_name_path);
		return -1;
	}

	if (type_check(name) == Directory) {//ѹ�����ļ�����Ŀ¼��������
		printf("zip warning: cannot zip '%s': Not a file\n", name);
		close_dir(inode_num);
	    inode_num = original_inode;
	    open_dir(inode_num);
		return -1;
	}

	if (type_check(name) == -1) {//Ҫѹ�����ļ�������
		printf("zip warning: name not matched: %s\n", original_name_path);
		printf("zip error: Nothing to do!(%s)\n", original_zip_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	if (file_read(name) == -1) {//�ļ���ȡʧ�ܣ����ɶ�
		printf("adding: %s\n", original_name_path);
		printf("zip warning: Permission denied\n");
		printf("zip warning: could not open for reading: %s\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	inode_source_file = check_name(inode_num, name);//��¼ԭ�ļ���inode

	close_dir(inode_num);
	inode_num = original_inode;
	open_dir(inode_num);//���ز���ǰ��Ŀ¼λ��

	if (eat_path(zip_name) == -1) {//·����������
		printf("zip I/O error: No such file or directory\n");
		printf("zip error: Could not create output file (%s.zip)\n", original_zip_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	link root = NULL;
	char s[HaffCodeLen] = { 0 };

	map = (maptr)malloc(sizeof(mapnode)*ValueSize);//�����յĹ�ϣ�����

	strcpy(FILENAME, BUFF);//Ҫѹ�����ļ�������

	StaFrequency();//ͳ��Ƶ��

	root = CreatHuff();//���ɹ�������

	HuffCode(root, s);//���ɹ����������
	len = count;//��ñ�

	Compression(name);//ѹ���ļ�

	strcat(zip_name, ".zip");
	if (check_name(inode_num, zip_name) == -1) { //Ŀ���ļ������ڣ��򴴽�
		make_file(inode_num, zip_name, File);//�����յ�ѹ���ļ�
		zip_write(zip_name);//��ѹ���������д��ѹ���ļ�
		inode_zip_file = check_name(inode_num, zip_name);//��¼ѹ���ļ���inode

		if (root == NULL) {//ԭ�ļ�Ϊ��
			printf("adding: %s (stored 0%%)\n", original_name_path);
		}
		else {//ԭ�ļ���Ϊ��
			printf("adding: %s (deflated %d%%)\n", original_name_path,
			getCompressionRatio(inode_source_file, inode_zip_file));
		}
	}
	else {//Ŀ���ļ����ڣ������
		zip_write(zip_name);//��ѹ���������д��ѹ���ļ�
		inode_zip_file = check_name(inode_num, zip_name);//��¼ѹ���ļ���inode

		if (root == NULL) {//ԭ�ļ�Ϊ��
			printf("updating: %s (stored 0%%)\n", original_name_path);
		}
		else {//ԭ�ļ���Ϊ��
			printf("updating:  %s (deflated %d%%)\n", original_name_path,
			getCompressionRatio(inode_source_file, inode_zip_file));
		}
	}

	close_dir(inode_num);
	inode_num = original_inode;
	open_dir(inode_num);
	return 0;
}

int unzip(char* name)
{
	char original_name_path[30];
	int original_inode = inode_num;//��¼��ǰ��inode
	strcpy(original_name_path, name);
	if (eat_path(name) == -1) {
		printf("unzip: cannot find or open��%s'.\n", original_name_path);
		return -1;
	}

	if (type_check(name) == -1) {
		printf("unzip: cannot find or open��%s'.\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	if (file_read(name) == -1) {//�ļ���ȡʧ��
		printf("error: cannot open zipfile [ %s ], permission denied\n", original_name_path);
		printf("unzip: cannot find or open��%s'.\n", original_name_path);
		close_dir(inode_num);
		inode_num = original_inode;
		open_dir(inode_num);
		return -1;
	}

	char s[HaffCodeLen] = { 0 };

	map = (maptr)malloc(sizeof(mapnode)*ValueSize);

	int res = DeCompression();//��ѹ�ļ�

	if (res == -1) {
		printf("unzip:  cannot find zipfile directory in %s, it is not a zipfile\n", original_name_path);
		return -1;
	}

	if (check_name(inode_num, FILENAME) == -1) {
		make_file(inode_num, FILENAME, File);//�����յĽ�ѹ�ļ�
		unzip_write(FILENAME);//����ѹ�������д���ѹ�ļ�
	}
	else {
		printf("Archive: %s\n", original_name_path);

		char choice;
		char new_name[NameLength];
		int flag = 1;

		char ch;
		while ((ch = getchar()) != '\n' && ch != EOF);//������뻺����

		while (flag) {
			printf("replace %s? [y]es, [n]o, [r]ename: ", FILENAME);
			scanf("%c", &choice);

			switch (choice) {
				case 'y':
					unzip_write(FILENAME);//����ѹ�������д���ѹ�ļ�
					flag = 0;
					break;

				case 'n':
					flag = 0;
					break;

				case 'r':
					printf("new name: ");
					scanf("%s", new_name);
					make_file(inode_num, new_name, File);//�����յĽ�ѹ�ļ�
					unzip_write(new_name);//����ѹ�������д���ѹ�ļ�
					flag = 0;
					break;

				default:
					printf("error: invalid response\n");
					while ((ch = getchar()) != '\n' && ch != EOF);//ָ�����������뻺����
					break;
			}
		}
	}

	close_dir(inode_num);
	inode_num = original_inode;
	open_dir(inode_num);
	return 0;
}

void StaFrequency()
{
	FILE *fp;
	unsigned char c;

	memset(weight, 0, sizeof(int) * ValueSize);
	if ((fp = fopen(FILENAME, "rb")) == NULL) {
		printf("�ļ���ʧ��!\n");
		exit(0);
	}

	while (!feof(fp)) {
		if (fread(&c, 1, 1, fp) == 1)
			weight[int(c)]++;
	}
	fclose(fp);
}

link CreatFreQueue()
{
	int i, length = 0;
	link q, front, rear;

	front = rear = (link)malloc(sizeof(node));
	front->next = NULL;

	for (i = 0; i <= 255; i++)
		if (weight[i] != 0) {
			q = (link)malloc(sizeof(node));

			if (q) {
				q->weight = weight[i];
				q->lch = NULL;
				q->rch = NULL;
				q->date = i;
				q->next = NULL;
				length++;

				rear->next = q;
				rear = rear->next;
			}
		}
	front->weight = length;//ͷ����weight��¼���г���
	return front;
}

link CreatHuff()
{
	link front, minnode1, minnode2, q, r;//minnode1��С�ڵ㣬minnode2��С�ڵ�
	link parent, temp;
	int length, i;

	front = CreatFreQueue();
	length = front->weight;

	for (i = 1; i <= length - 1; i++) {
		minnode1 = front->next;
		minnode2 = minnode1->next;

		if (minnode1->weight > minnode2->weight) {
			temp = minnode1;
			minnode1 = minnode2;
			minnode2 = temp;
		}

		for (q = front->next->next->next; q != NULL; q = q->next) {
			if (minnode1->weight > q->weight) {
				minnode2 = minnode1;
				minnode1 = q;
			}
			else
				if (minnode2->weight > q->weight)
					minnode2 = q;
		}

		parent = (link)malloc(sizeof(node));
		parent->date = 0;
		parent->lch = minnode1;
		parent->rch = minnode2;
		parent->weight = minnode1->weight + minnode2->weight;

		for (r = front, q = front->next; q != NULL; q = q->next, r = r->next) {//����С�ڵ�ɾ��
			if (q == minnode1) {
				r->next = q->next;
				break;
			}
		}
		for (r = front, q = front->next; q != NULL; q = q->next, r = r->next) {//����С�ڵ�ɾ��
			if (q == minnode2) {
				r->next = q->next;
				break;
			}
		}

		parent->next = front->next;//�ϳɵĸ��׽ڵ�������
		front->next = parent;
	}
	return front->next;
}

void HuffCode(link root, char s[])
{
	char s1[HaffCodeLen] = { 0 },
		s2[HaffCodeLen] = { 0 };

	if (root == NULL)
		return;

	if (root->lch) {
		strcpy(s1, s);
		strcat(s1, "0");
		HuffCode(root->lch, s1);
	}

	if (root->rch) {
		strcpy(s2, s);
		strcat(s2, "1");
		HuffCode(root->rch, s2);
	}

	if (root->lch == NULL && root->rch == NULL) {
		map[count].ch = root->date;
		strcpy(map[count].s, s);
		//	printf("%d  %s\n", root->date,map[count].s);
		count++;
	}
}

void Compression(char* name)
{
	int i, j, k;
	__int64 length = 0;
	unsigned char ch;
	int key;
	char s1[9] = { 0 };
	link root;
	FILE *fp, *fout, *fout1, *fp1;

	if ((fp = fopen(FILENAME, "rb")) == NULL) {
		printf("ԭʼ�ļ���ʧ��!\n");
		exit(0);
	}

	if ((fout = fopen("Compress.txt", "wb")) == NULL) {
		printf("ѹ���ļ���ʧ��!\n");
		exit(0);
	}

	if ((fout1 = fopen("code.txt", "wb")) == NULL) {
		printf("�����ļ���ʧ��!\n");
		exit(0);
	}

	root = CreatHuff();
	fprintf(fout, "%x ", ZipMagicNumber);//�ļ�ͷΪzip��ʽ�ļ���ħ��
	fprintf(fout, "%s ", name);//��ѹ�����ļ����ļ���
	PrintfTree(root, fout);

	while (!feof(fp)) {
		fread(&ch, 1, 1, fp);
		for (i = 0; i <= len; i++)
			if (int(ch) == map[i].ch) {
				fprintf(fout1, "%s", map[i].s);
				length = length + strlen(map[i].s);
			}
	}

	fclose(fp);

	if (length % 8 != 0) {//����8�ı���
		for (i = 1; i <= (8 - length % 8); i++)
			fprintf(fout1, "0");
		supple = 8 - length % 8;
	}
	fclose(fout1);
	fp1 = fopen("code.txt", "rb");

	if (length % 8 != 0)
		length = length + (8 - length % 8);

	for (i = 1; i <= length / 8; i++) {
		key = 0;

		for (k = 0; k<8; k++) {
			ch = fgetc(fp1);
			s1[k] = ch;
		}

		for (j = 0; j<8; j++)
			key = key + (s1[j] - 48)*(int)pow(2, 8 - j - 1);
		fprintf(fout, "%c", key);
	}
	fclose(fout);
}

int DeCompression()
{
	char ch, s1[9] = { 0 };
	unsigned char key;
	__int64 i, length = 0;
	link root, q;
	FILE *fp, *fout, *fout1, *fp1;

	fp = fopen(BUFF, "rb");
	fout = fopen("DeCompress.txt", "wb");
	fout1 = fopen("decode.txt", "wb");

	int test_magic_number;
	fscanf(fp, "%x", &test_magic_number);
	//printf("%x\n", test_magic_number);

	if (test_magic_number != ZipMagicNumber) {
		return -1;
	}

	fscanf(fp, "%s", &FILENAME);//ԭ�ļ����ļ���
	//printf("%s\n", FILENAME);
	root = InputTree(fp);

	fgetc(fp);
	while (!feof(fp)) {
		fread(&key, 1, 1, fp);

		if (feof(fp)) break;

		for (i = 7; i >= 0; i--) {
			s1[i] = ((int)key) % 2 + 48;
			key /= 2;
		}

		s1[8] = '\0';
		fprintf(fout1, "%s", s1);
		length = length + 8;
	}
	fclose(fp);
	fclose(fout1);

	fp1 = fopen("decode.txt", "rb");

	q = root;
	i = 0;
	while (i<(length - supple)) {
		ch = fgetc(fp1);

		if (!q->lch && !q->rch) {
			if (q->date > 127)
				fprintf(fout, "%c", q->date - 256);
			else
				fprintf(fout, "%c", q->date);
			q = root;
		}

		if (ch == '0' && q->lch != NULL)
			q = q->lch;
		else if (ch && q->rch != NULL)
			q = q->rch;

		i++;
	}
	fclose(fp1);
	fclose(fout);

	return 0;
}

void PrintfTree(link root, FILE *fp)
{
	if (root) {
		fprintf(fp, "%d ", root->date);

		PrintfTree(root->lch, fp);
		PrintfTree(root->rch, fp);
	}
	else
		fprintf(fp, "-1 ");

}

link InputTree(FILE *fp)
{
	int date;
	link p = NULL;

	fscanf(fp, "%d", &date);

	if (date != -1) {
		p = (link)malloc(sizeof(node));
		p->date = date;

		p->lch = InputTree(fp);
		p->rch = InputTree(fp);
	}

	return p;
}

/*ѹ���������д���Ӧ��ѹ���ļ���*/
int zip_write(char* name)
{
	int 	inode;
	int		num, blk_num;
	FILE* 	fp = fopen("Compress.txt", "rb");
	Inode	temp;
	char	buff[BlkSize];

	inode = check_name(inode_num, name);

	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	if (temp.access[1] == 0) { //�ļ�����д
		printf("%d\n", temp.access[1]);
		return -1;
	}

	//��ԭ�ļ����ݶ�Ӧ�ش��̿��ͷţ��ļ���С��Ϊ0��
	//Ȼ������������̿����޸ĺ���ļ����ݣ������ļ��Ĵ�С���޸�ʱ�䣬���Ըı�ʱ��
	temp.blk_num = 0;
	temp.file_size = 0;

	while (num = fread(buff, sizeof(char), BlkSize, fp)) {
		printf("num:%d\n", num);
		if ((blk_num = get_blk()) == -1) {
			printf("error:	block has been used up\n");
			break;
		}
		/*�ı�Inode�ṹ����Ӧ״̬*/
		temp.blk_identifier[temp.blk_num++] = blk_num;
		temp.file_size += num;

		/*������д�ش��̿�*/
		fseek(Disk, BlockBeg + BlkSize*blk_num, SEEK_SET);
		fwrite(buff, sizeof(char), num, Disk);
	}
	temp.i_mtime = time(NULL);
	temp.i_ctime = time(NULL);

	/*���޸ĺ��Inodeд��*/
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fwrite(&temp, sizeof(Inode), 1, Disk);

	fclose(fp);
	return 1;
}

int getCompressionRatio(int inodeBefore, int inodeAfter)
{
	Inode temp1, temp2;

	fseek(Disk, InodeBeg + sizeof(Inode)*inodeBefore, SEEK_SET);
	fread(&temp1, sizeof(Inode), 1, Disk);

	fseek(Disk, InodeBeg + sizeof(Inode)*inodeAfter, SEEK_SET);
	fread(&temp2, sizeof(Inode), 1, Disk);

	return temp2.file_size * 100 / temp1.file_size;
}

int unzip_write(char* name)//����ѹ�������д���ѹ�ļ�
{
	int 	inode;
	int		num, blk_num;
	FILE* 	fp = fopen("Decompress.txt", "rb");
	Inode	temp;
	char	buff[BlkSize];

	inode = check_name(inode_num, name);

	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fread(&temp, sizeof(Inode), 1, Disk);

	if (temp.access[1] == 0) { //�ļ�����д
		printf("%d\n", temp.access[1]);
		return -1;
	}

	//��ԭ�ļ����ݶ�Ӧ�ش��̿��ͷţ��ļ���С��Ϊ0��
	//Ȼ������������̿����޸ĺ���ļ����ݣ������ļ��Ĵ�С���޸�ʱ�䣬���Ըı�ʱ��
	temp.blk_num = 0;
	temp.file_size = 0;

	while (num = fread(buff, sizeof(char), BlkSize, fp)) {
		printf("num:%d\n", num);
		if ((blk_num = get_blk()) == -1) {
			printf("error:	block has been used up\n");
			break;
		}
		//�ı�Inode�ṹ����Ӧ״̬
		temp.blk_identifier[temp.blk_num++] = blk_num;
		temp.file_size += num;

		//������д�ش��̿�
		fseek(Disk, BlockBeg + BlkSize*blk_num, SEEK_SET);
		fwrite(buff, sizeof(char), num, Disk);
	}

	temp.i_mtime = time(NULL);
	temp.i_ctime = time(NULL);
	//���޸ĺ��Inodeд��
	fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
	fwrite(&temp, sizeof(Inode), 1, Disk);

	fclose(fp);
	return 1;
}
