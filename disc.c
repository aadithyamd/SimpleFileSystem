#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXDEG 510
#define MAXFREE 4082

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

struct freeList {
	int n;
	bool free[MAXFREE];
};

struct bnode {
    int count;
    int key[MAXDEG - 1];
    int (ptr[MAXDEG]);
    int par;
    int is_leaf;
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
/*void syncDisk();*/

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
	n = nblocks / MAXFREE;
	if (nblocks % MAXFREE > 0) {
		++n;
	}
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

	return num_disk;
}

int closeDisk(int n)
{
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
