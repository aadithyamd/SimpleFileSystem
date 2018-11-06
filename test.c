#include "disc.c"

void test(int f1, int nblocks)
{
	struct block b;
	int n;
	int i;

	n = 1 + nblocks / sizeof(struct block) ;
	for (i = 0; i < n; ++i) {
		readBlock(f1, i, &b);
		printf("Block no: %d Type: %d ", b.blockno, b.type);
		if (b.type == 2) {
			printf("N : %d [%d %d ..]\n ", b.blk.f.n,
					b.blk.f.free[0], b.blk.f.free[1]);
		} else if (b.type == 1) {
			printf("count : %d isleaf: %d\n ", b.blk.b.count,
					b.blk.b.is_leaf);
		} else {

		}
	}
}

int main()
{
	int f1;

	printf("Block Size :%lu\n", sizeof(struct block));
	f1 = openDisk("hd1", 256 * 1024);

	test(f1, 256 * 1024);

	closeDisk(f1);
	return 0;
}
