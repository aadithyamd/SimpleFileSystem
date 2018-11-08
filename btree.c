#include "disc.c"

#undef MAXDEG
#define MAXDEG 5

int print_hash(unsigned char hash[])
{
	int i; 

	for (i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		printf("%02x", hash[i]);
	}
	printf("\n");

	return 0;
}

void hashcpy(unsigned char h1[], unsigned char h2[])
{
	int i;
	for (i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		h1[i] = h2[i];
	}
}

int binary_search(unsigned char key[][SHA256_DIGEST_LENGTH], 
		unsigned char hash[], int n)
{
	int mid;
	int beg;
	int end;
	int cmp;
	beg = 0;
	end = n - 1;
	
	if (n <= 0) {
		return 0;
	}

	while (beg <= end) {
		mid = (beg + end) / 2;
		cmp = strncmp(key[mid], hash, SHA256_DIGEST_LENGTH);
		
		if (cmp == 0) {
			return mid;
		} else if (cmp < 0) {
			beg = mid + 1;
		} else {
			end = mid - 1;
		}
	}
	if (cmp > 0) {
		return mid;
	}

	return mid + 1;
}

int bnode_copy(struct bnode *dest, struct bnode *src, int beg, int end)
{
	int n;
	int i;

	n = end - beg + 1;
	dest->count = n;

	for (i = 0; i < n; ++i) {
		hashcpy(dest->key[i], src->key[beg + i]);
		dest->record_ptr[i] = src->record_ptr[beg + i];
		dest->ptr[i] = src->ptr[beg + i];
	}
	dest->ptr[i] = src->ptr[beg + i];
	
	return 0;
}


int insert_sorted(struct bnode *b, unsigned char hash[], int n)
{
	int i;
	int j;

	for (i = 0; i < n; ++i) {
		if (strncmp(b->key[i], hash, SHA256_DIGEST_LENGTH) > 0) {
			break;
		}
	}

	for (j = n; j > i; --j) {
		hashcpy(b->key[j], b->key[j - 1]);
		b->record_ptr[j] = b->record_ptr[j - 1];
		b->ptr[j] = b->ptr[j - 1];
	}
	hashcpy(b->key[i], hash);

	return i;
}

int bBlock_split(int disk, struct block *blk, struct block *parent)
{
	int med;
	int n;
	int i;
	int pos;
	struct block *right = NULL;
	struct block *left = NULL;

	if (blk == NULL) {
		return -1;
	}

	n = blk->blk.b.count - 1;
	med = n / 2;

	left = (struct block *) malloc(sizeof(struct block));
	left->blk.b.is_leaf = blk->blk.b.is_leaf;
	left->blk.b.par = blk->blockno;
	left->type = 1;
	
	if (blk->blockno == 0) {
		
		printf("Splitting Root : ");

		i = AllocateBlock(disk);
		left->blockno = i;
		bnode_copy(&(left->blk.b), &(blk->blk.b), 0, med - 1);
		writeBlock(disk, i, left);
		printf("(Left : %d) ", i);

		bnode_copy(&(blk->blk.b), &(blk->blk.b), med, med);
		blk->blk.b.ptr[0] = i;

		right = left;
		i = AllocateBlock(disk);
		right->blockno = i;
		bnode_copy(&(right->blk.b), &(blk->blk.b), med + 1, n);
		writeBlock(disk, i, right);

		printf("(Right : %d)\n", i);
		
		blk->blk.b.ptr[1] = i;
		blk->blk.b.is_leaf = false;
		writeBlock(disk, 0, blk);
	} else {
		if (parent == NULL) {
			printf("Error non root passed without parent!!\n");
			return -1;
		}
		pos = insert_sorted(&(parent->blk.b), 
				blk->blk.b.key[med], parent->blk.b.count);
		/* Left pointer to same node */
		parent->blk.b.ptr[pos] = blk->blockno;	
		parent->blk.b.count++;

		blk->blk.b.count = med;	
		writeBlock(disk, blk->blockno, blk);

		right = left;
		i = AllocateBlock(disk);
		right->blockno = i;
		bnode_copy(&(right->blk.b), &(blk->blk.b), med + 1, n);
		writeBlock(disk, i, right);

		/* Right pointer to newly allocated node  */
		parent->blk.b.ptr[pos + 1] = i;
		
		/* Write parent to memory */
		i = parent->blockno;
		writeBlock(disk, i, parent);
		
	}
	free(left);

	return 0;
}

void inorder(int disk, int blockno)
{
	struct block blk;
	int i;
	int n;

	readBlock(disk, blockno, &blk);
	n = blk.blk.b.count;
	
	for (i = 0; i < n; ++i) {
		if (blk.blk.b.is_leaf == false) {
			inorder(disk, blk.blk.b.ptr[i]);
		}
		print_hash(blk.blk.b.key[i]);
	}
	if (blk.blk.b.is_leaf == false) {
		inorder(disk, blk.blk.b.ptr[i]);
	}
}

int leaf_insert(struct bnode *b, char name[])
{
	int i;
	int n;	
	unsigned char hash[SHA256_DIGEST_LENGTH]; 

	SHA256(name, strlen(name), hash);
	n = b->count;
	(b->count)++;

	return insert_sorted(b, hash, n);
}

int btree_insert(int disk, char name[])
{
	struct block rblock;
	struct block cblock;
	struct block *blk = NULL;
	struct block *parent = NULL;
	struct block *temp;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	int i;

	SHA256(name, strlen(name), hash);
	
	readBlock(disk, 0, &rblock);
	blk = &rblock;
	
	/* Traverse till the leaf Splitting all full nodes on path */
	while (blk->blk.b.is_leaf == false || 
		(blk->blk.b.count >= (MAXDEG - 1))) {

		if (blk->blk.b.count >= (MAXDEG - 1)) {
			bBlock_split(disk, blk, parent);

			if (parent != NULL) {
				temp = parent;
				parent = blk;
				blk = temp;
			}
			
			/* Todo Go to appropriate side */
		}
		
		i = binary_search(blk->blk.b.key, hash, blk->blk.b.count);
		i = blk->blk.b.ptr[i];

		if (parent == NULL) {
			parent = &cblock;
		}
		temp = parent;
		parent = blk;
		blk = temp;
		readBlock(disk, i, blk);
	}

	leaf_insert(&(blk->blk.b), name);
	i = blk->blockno;
	writeBlock(disk, i, blk);
	
	return 0;
}

int main()
{
	int f1;
	int n;
	char name[40];

	f1 = openDisk("hd1", DISKSIZE);
	printf("N: ");	
	scanf("%d", &n);
	while (n--) {
		printf("Name: ");
		scanf("%s", name);
		btree_insert(f1, name);
	}

	inorder(f1, 0);

	closeDisk(f1);

	return 0;
}
