#include "btree.c"

int readData(char *s1, int length) {
	char ch;
	int i;
	i = 0;
	ch = '\0';
	while (i < length && ch != '\n') {
		ch = getc(stdin);
		s1[i] = ch;
		++i;
	}
	s1[i] = '\0';
 
	return strlen(s1);
}

void test(int f1, int nblocks)
{
	struct block b;
	struct block b1;
	char data[INODEDATA];
	char name[80];
	int n;
	int i;
	int k;

	printf("Enter K :");
	scanf("%d", &k);
	b1.type = 0;
	for (i = 0; i < k; ++i) {
		printf("Name: ");
		scanf("%s", name);
		readData(data, INODEDATA);
		btree_insert(f1, name, data);
	}
	
   	return; 
	
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
				printf("N : %d [%d %d ..]\n", b.blk.f.n,
						b.blk.f.free[0], 
						b.blk.f.free[1]);
			} else if (b.type == 1) {
				printf("count : %d isleaf: %d\n", 
						b.blk.b.count, 
						b.blk.b.is_leaf);
			} else if (b.type == 0) {
				printf("Inode no: %d Size: %ld data: %s\n", 
						b.blk.i.inode_no, 
						b.blk.i.size, 
						b.blk.i.data);
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
	inorder(f1, 0);
	closeDisk(f1);
	return 0;
}
