#include <stdio.h>
#include <stdlib.h>

#define MAXDEG 510

int openDisk(char *filename, int nbytes);
int closeDisks();
int readBlock(int disk, int blocknr, void *block);
int writeBlock(int disk, int blocknr, void *block);
/*void syncDisk();*/

FILE *disk_p[100] = {NULL};
int num_disk = -1;

struct inode {
	int inode_no;
	long size;
	char data[4016];
	int point[12];
	int Single;
	int Double;
};

struct bnode {
    int count;
    int key[MAXDEG - 1];
    int (ptr[MAXDEG]);
    int par;
};

union block_type {
	struct inode i;
	struct bnode b;
};

struct block {
	int blocknr;
	int type;
	union block_type blk;
};



int openDisk(char *filename, int nbytes)
{
	num_disk++;
	disk_p[num_disk] = fopen(filename, "rb+");

	return num_disk;
}

int closeDisks()
{
	int n;
	n = 0;
	while (num_disk >= 0) {
		fclose(disk_p[num_disk]);
		--num_disk;
		++n;
	}
	return n;
}



int readBlock(int disk, int blocknr, void *block)
{
	if (disk > num_disk) {
		return -1;
	}



	return 0;
}

int main()
{
	printf("%lu\n", sizeof(struct block));

	return 0;
}
