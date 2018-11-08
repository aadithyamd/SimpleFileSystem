#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <openssl/sha.h>

#define MAXDEG 102
#define MAXFREE 4082
#define MAXDISKS 100
#define INODEDATA 3960
#define DISKSIZE 256 * 1024

FILE *disk_p[MAXDISKS] = {NULL};
int disk_size[MAXDISKS] = {-1};
int num_disk = -1;
int inode_no = -1;

struct inode {
	int inode_no;
	long size;
	bool is_dir;
	char name[40];
	char data[INODEDATA];
	int point[12];
	int Single;
	int Double;
};

struct freeList {
	int n;
	bool free[MAXFREE];
};

struct bnode {
	int count;
	unsigned char key[MAXDEG - 1][SHA256_DIGEST_LENGTH];
	int record_ptr[MAXDEG - 1];
	int ptr[MAXDEG];
	int par;
	bool is_leaf;
};

union block_type {
	struct inode i;
	struct bnode b;
	struct freeList f;
};

struct block {
	int blockno;
	int type;
	union block_type blk;
};

int openDisk(char *filename, int nblocks);
int closeDisks();
int readBlock(int disk, int blockno, struct block *blk);
int writeBlock(int disk, int blockno, struct block *blk);
int AllocateBlock(int disk);
int FreeBlock(int disk, int blockno);
int getInodeNo();
/*void syncDisk();*/

int getInodeNo()
{
	++inode_no;
	return inode_no;
}

void initFile(char *filename, int nblocks)
{
	FILE *fp = NULL;
	int n;
	int k;
	int n_used;
	int i;
	struct block blk;

	/* Creates a file of n blocks */
	fp = fopen(filename, "wb+");
	fseek(fp, nblocks * 4096 , SEEK_SET);
	fwrite(&blk, sizeof(blk), 1, fp);

	fseek(fp, 0, SEEK_SET);

	/* Write btree node as 1st node */
	blk.blk.b.count = 0;
	blk.blk.b.is_leaf = 1;
	blk.type = 1;
	blk.blockno = 0;
	fwrite(&blk, sizeof(blk), 1, fp);

	/* write bit vector for free list on next n blocks */
	n = (int) ceil(nblocks * 1.0 / MAXFREE);

	/* n for bit vector 1 for b tree root */
	n_used = n + 1;

	k = 1;
	blk.type = 2;
	if (n_used > MAXFREE) {
		for (i = 0; i < MAXFREE; ++i) {
			blk.blk.f.free[i] = false;
		}
		while (n_used > k * MAXFREE) {
			blk.blockno = k;
			blk.blk.f.n = n - k;
			++k;
			fwrite(&blk, sizeof(blk), 1, fp);
		}
	}

	if (n_used % MAXFREE > 0) {
		blk.blockno = k;
		blk.blk.f.n = n - k;
		++k;
		for (i = n_used % MAXFREE; i < MAXFREE; ++i) {
			blk.blk.f.free[i] = true;
		}
		fwrite(&blk, sizeof(blk), 1, fp);
	}

	/* Remaining blocks are full free */
	for (i = 0; i < MAXFREE; ++i) {
		blk.blk.f.free[i] = true;
	}
	while (k <= n) {
		blk.blockno = k;
		blk.blk.f.n = n - k;
		++k;
		fwrite(&blk, sizeof(blk), 1, fp);
	}

	/* Inode number stored at end of file */
	fseek(fp, -1 * sizeof(int), SEEK_END);
	fwrite(&inode_no, sizeof(int), 1, fp);

	fclose(fp);
}

int openDisk(char *filename, int nblocks)
{
	FILE *fp;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		initFile(filename, nblocks);
	} else {
		fclose(fp);
	}

	num_disk++;
	disk_p[num_disk] = fopen(filename, "rb+");

	if (inode_no == -1 && num_disk == 0) {
		fseek(disk_p[num_disk], -1 * sizeof(int), SEEK_END);
		fread(&inode_no, sizeof(int), 1, disk_p[num_disk]);
	}

	disk_size[num_disk] = nblocks;

	return num_disk;
}

int closeDisk(int n)
{
	if (n == 0) {
		fseek(disk_p[n], -1 * sizeof(int), SEEK_END);
		fwrite(&inode_no, sizeof(int), 1, disk_p[n]);
	}
	fclose(disk_p[n]);
	
	return n;
}



int readBlock(int disk, int blockno, struct block *blk)
{
	FILE *fp;
	if (disk > num_disk) {
		return -1;
	}
	fp = disk_p[disk];

	fseek(fp, blockno * sizeof(struct block), SEEK_SET);
	fread(blk, sizeof(struct block), 1, fp);

	return 0;
}

int writeBlock(int disk, int blockno, struct block *blk)
{
	FILE *fp;
	if (disk > num_disk) {
		return -1;
	}
	fp = disk_p[disk];

	fseek(fp, blockno * sizeof(struct block), SEEK_SET);
	fwrite(blk, sizeof(struct block), 1, fp);

	return 0;
}

int AllocateBlock(int disk)
{
	struct block blk;
	int b;
	int n;
	int i;

	n = (int) ceil(disk_size[disk] * 1.0 / MAXFREE);
	for (b = 1; b <=n; ++b) {
		readBlock(disk, b, &blk);

		/* break if it is not a freeList block */
		if (blk.type != 2) {
			break;
		}

		for (i = 0; i < MAXFREE; ++i) {
			/* return the free block no */
			if(blk.blk.f.free[i] == true) {
				blk.blk.f.free[i] = false;
				writeBlock(disk, b, &blk);

				return (b - 1) * MAXFREE + i;	
			}
		}
	}

	/* Free blocks are not available */
	return -1;
}

int FreeBlock(int disk, int blockno)
{
	struct block blk;
	int b;
	int n;

	b = 1 + blockno / MAXFREE;
	n = blockno % MAXFREE;
	readBlock(disk, b, &blk);
	if(blk.blk.f.free[n] == true) {
		return 0;
	} else {
		/* Todo: Add protection here */
		blk.blk.f.free[n] = true;
	}
	writeBlock(disk, b, &blk);

	return 0;
}
