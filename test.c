#include "disc.c"

int readData(char *s1, int length) {
	strcpy(s1, "testing inode working$ ok!"); 
	return 27;
}

void test(int f1, int nblocks)
{
	struct block b;
	struct block b1;
	int n;
	int i;
	int k;
	int r;

	printf("Enter K :");
	scanf("%d", &k);
	r = k / 2;
	b1.type = 0;
	for (i = 0; i < k; ++i) {
		n = AllocateBlock(f1);

		b1.blockno = n;
		printf("Allocated block no : %d\n", n);
		b1.blk.i.inode_no = getInodeNo();
		b1.blk.i.size = readData(b1.blk.i.data, INODEDATA);

		writeBlock(f1, n, &b1);	
	}

	n = 1 + nblocks / sizeof(struct block) ;
	for (i = 1; i <= n; ++i) {
		readBlock(f1, i, &b1);
		for (k = 0; k < MAXFREE; ++k) {
			if (b1.blk.f.free[k] == true) {
				continue;
			}
			readBlock(f1, (i - 1) * MAXFREE + k, &b);
			printf("Block no: %d Type: %d ", b.blockno, b.type);
			if (b.type == 2) {
/*				printf("N : %d [%d %d ..]\n", b.blk.f.n,
						b.blk.f.free[0], 
						b.blk.f.free[1]);
*/			} else if (b.type == 1) {
				printf("count : %d isleaf: %d\n", 
						b.blk.b.count, 
						b.blk.b.is_leaf);
			} else if (b.type == 0) {
				printf("Inode no: %d Size: %ld data: %26s\n", 
						b.blk.i.inode_no, 
						b.blk.i.size, 
						b.blk.i.data);
				if (r > 0) {
					--r;
					FreeBlock(f1, (i - 1) * MAXFREE + k);
				}
			} else {
				printf("Invalid type!\n");
			}
		}
	}
}

int main()
{
	int f1;

	printf("Block Size :%lu\n", sizeof(struct block));
	f1 = openDisk("hd1", DISKSIZE);

	test(f1, DISKSIZE);

	closeDisk(f1);
	return 0;
}
