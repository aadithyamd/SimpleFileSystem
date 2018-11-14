#include "disc.c"

#undef MAXDEG
#define MAXDEG 5


int btree_search(int disk, char name[], int treeno);
int btree_insert(int disk, char name[], char data[], int treeno);

int print_hash(unsigned char hash[], int n)
{
	int i; 

	for (i = 0; i < n; ++i) {
		printf("%02x", hash[i]);
	}
	printf("\n");

	return 0;
}

void hashcpy(unsigned char h1[SHA256_DIGEST_LENGTH], 
		unsigned char h2[SHA256_DIGEST_LENGTH])
{
	int i;
	for (i = 0; i < SHA256_DIGEST_LENGTH - 2; ++i) {
		h1[i] = h2[i];
	}
	h1[i] = '\0';
	h2[i] = '\0';
}

int binary_search(char key[][NAMELENGTH], char name[], int n)
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
		cmp = strcmp(key[mid], name);

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

int modify_file(int disk, int blockno, char data[])
{
	struct block blk;
	unsigned char hash[SHA256_DIGEST_LENGTH];

	readBlock(disk, blockno, &blk);
	if (blk.type != 0) {
		printf("Error not a file\n");
		return -1;
	}

	SHA256((unsigned char *) data, strlen(data), hash);

	strcpy(blk.blk.i.data, data);
	blk.blk.i.size = (int) strlen(data);
	hashcpy(blk.blk.i.hash, hash);

	writeBlock(disk, blockno, &blk);

	return 0;
}

int cat_file(int disk, int blockno)
{
	struct block blk;

	readBlock(disk, blockno, &blk);
	if (blk.type == 3) {
		printf("Error not a file\n");
		return -1;
	} else if (blk.type == 1) {
		printf("cat: It is a directory\n");

		return 0;
	}
	printf("%s\n", blk.blk.i.data);

	return 0;
}

int ch_dir(int disk, char name[], int treeno)
{
	struct block blk;
	int p;

	p = btree_search(disk, name, treeno);

	if (p == -1) {
		printf("%s: not found! \n", name);

		return treeno;
	}

	readBlock(disk, p, &blk);

	if (blk.type != 1) {
		printf("Error: not a directory\n");

		return treeno;
	} 

	printf("%s\n", name);

	return p;

}

int make_dir(int disk, char name[], int treeno)
{
	struct block blk;
	int p;
	char temp[] = "..";

	btree_insert(disk, name, temp, treeno);
	p = btree_search(disk, name, treeno);
	readBlock(disk, p, &blk);
	blk.type = 1;
	blk.blk.b.par = -1;
	blk.blk.b.count = 2;
	blk.blk.b.is_leaf = 1;
	strcpy(blk.blk.b.key[0], ".");
	strcpy(blk.blk.b.key[1], "..");
	blk.blk.b.record_ptr[0] = p;
	blk.blk.b.record_ptr[1] = treeno;
	writeBlock(disk, p, &blk);

	return 0;
}

int bnode_search(int disk, char name[], int *pos, int *rptr, int treeno)
{
	struct block rblock;
	struct block *blk = NULL;
	int i;
	int cmp;
	int n;

	blk = &rblock;
	i = treeno;
	do {
		readBlock(disk, i, blk);
		n = blk->blk.b.count;
		cmp = -1;
		for (i = 0; i < n; ++i) {
			cmp = strcmp(blk->blk.b.key[i], name);
			if (cmp == 0) {
				*pos = i;
				*rptr = blk->blk.b.record_ptr[i];
				return blk->blockno;
			} else if (cmp > 0) {
				break;
			}
		}
		i = blk->blk.b.ptr[i];
	} while (blk->blk.b.is_leaf == false);

	return -1;
}

int btree_search(int disk, char name[], int treeno)
{
	int i;
	int rptr;
	int pos;

	i = bnode_search(disk, name, &pos, &rptr, treeno);

	if (i == -1) {
		return -1;
	}

	return rptr;
}

int bnode_copy(struct bnode *dest, struct bnode *src, int beg, int end)
{
	int n;
	int i;

	n = end - beg + 1;
	dest->count = n;

	for (i = 0; i < n; ++i) {
		strcpy(dest->key[i], src->key[beg + i]);
		dest->record_ptr[i] = src->record_ptr[beg + i];
		dest->ptr[i] = src->ptr[beg + i];
	}
	dest->ptr[i] = src->ptr[beg + i];

	return 0;
}


int insert_sorted(struct bnode *b, char name[], int n)
{
	int i;
	int j;

	for (i = 0; i < n; ++i) {
		if (strcmp(b->key[i], name) > 0) {
			break;
		}
	}

	for (j = n; j > i; --j) {
		strcpy(b->key[j], b->key[j - 1]);
		b->record_ptr[j] = b->record_ptr[j - 1];
		b->ptr[j + 1] = b->ptr[j];
	}
	b->ptr[j + 1] = b->ptr[j]; /* testin */
	strcpy(b->key[i], name);

	return i;
}

bool is_root(struct block *blk)
{
	if (blk->blockno == 0 || blk->blk.b.par == -1) {
		return true;
	}
	return false;
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

	if (is_root(blk) == true) {
		i = AllocateBlock(disk);
		left->blockno = i;
		bnode_copy(&(left->blk.b), &(blk->blk.b), 0, med - 1);
		writeBlock(disk, i, left);

		bnode_copy(&(blk->blk.b), &(blk->blk.b), med, med);
		blk->blk.b.ptr[0] = i;

		right = left;
		i = AllocateBlock(disk);
		right->blockno = i;
		bnode_copy(&(right->blk.b), &(blk->blk.b), med + 1, n);
		writeBlock(disk, i, right);

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
		parent->blk.b.record_ptr[pos] = blk->blk.b.record_ptr[med];

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
	struct block t;
	int i;
	int n;

	readBlock(disk, blockno, &blk);
	n = blk.blk.b.count;

	for (i = 0; i < n; ++i) {
		if (blk.blk.b.is_leaf == false) {
			inorder(disk, blk.blk.b.ptr[i]);
		}
		readBlock(disk, blk.blk.b.record_ptr[i], &t);
		if (t.type == 0) {
			printf("%-12s  %2ld bytes  Inode no: %2d  sha256: ", 
					blk.blk.b.key[i], t.blk.i.size, 
					t.blk.i.inode_no);
			print_hash(t.blk.i.hash, 3);
		} else if (t.type == 1) {
			printf("%-12s it is a directory\n", blk.blk.b.key[i]);
		}
	}
	if (blk.blk.b.is_leaf == false) {
		inorder(disk, blk.blk.b.ptr[i]);
	}
}

int leaf_insert(struct bnode *b, char name[], int rptr)
{
	int i;
	int n;	

	n = b->count;
	(b->count)++;

	i = insert_sorted(b, name, n);
	b->record_ptr[i] = rptr;

	return i;
}

int btree_insert(int disk, char name[], char data[], int treeno)
{
	struct block rblock;
	struct block cblock;
	struct block *blk = NULL;
	struct block *parent = NULL;
	struct block *temp;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	int i;
	int rptr;

	SHA256((unsigned char *) data, strlen(data), hash);

	rptr = AllocateBlock(disk);
	rblock.blockno = rptr;
	rblock.type = 0;
	rblock.blk.i.inode_no = getInodeNo();
	rblock.blk.i.size = (int) strlen(data);
	hashcpy(rblock.blk.i.hash, hash);
	strcpy(rblock.blk.i.data, data);
	writeBlock(disk, rptr, &rblock);

	readBlock(disk, treeno, &rblock);
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

		}

		i = binary_search(blk->blk.b.key, name, blk->blk.b.count);
		i = blk->blk.b.ptr[i];

		if (parent == NULL) {
			parent = &cblock;
		}
		temp = parent;
		parent = blk;
		blk = temp;
		readBlock(disk, i, blk);
	}

	leaf_insert(&(blk->blk.b), name, rptr);
	i = blk->blockno;
	writeBlock(disk, i, blk);

	return 0;
}

/*	It merges 2 nodes along with another key	*/
int bBlock_merge(struct block *left, struct block *right, 
		char *name, int rptr)
{
	int n1;
	int n2;
	int i;

	if (left == NULL || right == NULL) {
		return -1;
	}

	i = (left->blk.b.count + right->blk.b.count + 1); 
	if (i >= MAXDEG) 
	{
		return -1;
	}

	n1 = left->blk.b.count;
	n2 = right->blk.b.count;

	strcpy(left->blk.b.key[n1], name);
	left->blk.b.record_ptr[n1] = rptr;
	++n1;

	for (i = 0; i < n2; ++i) {
		strcpy(left->blk.b.key[n1], right->blk.b.key[i]);
		left->blk.b.record_ptr[n1] = right->blk.b.record_ptr[i];
		left->blk.b.ptr[n1] = right->blk.b.ptr[i];
		++n1;
	}
	left->blk.b.ptr[n1] = right->blk.b.ptr[i];
	left->blk.b.count = n1;
	right->blk.b.count = 0;

	return 0;
}

void leftShift(struct bnode *b)
{
	int n;
	int i;

	--(b->count);
	n = b->count;
	for (i = 0; i < n; ++i) {
		strcpy(b->key[i], b->key[i + 1]);
		b->record_ptr[i] = b->record_ptr[i + 1];
		b->ptr[i] = b->ptr[i + 1];
	}
	b->ptr[i] = b->ptr[i + 1];

}

void rightShift(struct bnode *b)
{
	int n;
	int i;

	n = b->count;
	b->ptr[n + 1] = b->ptr[n];
	for (i = n - 1; i >= 0; --i) {
		strcpy(b->key[i + 1], b->key[i]);
		b->record_ptr[i + 1] = b->record_ptr[i];
		b->ptr[i + 1] = b->ptr[i];
	}

}

int delFromNode(struct bnode *b, char name[])
{
	int n;
	int i;
	int cmp;

	n = b->count;
	for (i = 0; i < n; ++i) {
		cmp = strcmp(b->key[i], name);
		if (cmp == 0) {
			break;
		} else if (cmp > 0) {
			printf("File not found \n");
			return -1;
		}
	}
	if (i == n) {
		printf("File not found \n");
		return -1;
	}

	for (; i < n - 1; ++i) {
		strcpy(b->key[i], b->key[i + 1]);
		b->record_ptr[i] = b->record_ptr[i + 1];
		b->ptr[i] = b->ptr[i + 1];
	}
	b->ptr[i] = b->ptr[i + 1];
	--(b->count);

	return 0;
}

int balance(int disk, int blockno, char name[])
{
	struct block blk;
	struct block parent;
	struct block sibling;
	struct block *left = NULL;
	struct block *right = NULL;
	int pos;
	int i;

	while (true) {
		readBlock(disk, blockno, &blk);
		if (blk.blk.b.count >= MAXDEG - 1 || blk.blk.b.par == -1) {
			return 0;
		}
		readBlock(disk, blk.blk.b.par, &parent);
		pos = binary_search(blk.blk.b.key, name, blk.blk.b.count);

		if (pos == -1) {
			printf("Error occured in fn balance\n");

			return -1;
		}

		if (parent.blk.b.ptr[pos] == blk.blockno) {
			/* Left child */
			readBlock(disk, parent.blk.b.ptr[pos + 1], &sibling);
			left = &blk;
			right = &sibling;

		} else if (parent.blk.b.ptr[pos + 1] == blk.blockno) {
			/* Right Child */
			readBlock(disk, parent.blk.b.ptr[pos], &sibling);
			left = &sibling;
			right = &blk;
		} else {
			printf("Error: Wrong parent value\n");

			return -1;
		}

		if(left->blk.b.count + right->blk.b.count < MAXDEG - 1) {
			/* Merge */
			i = bBlock_merge(left, right, blk.blk.b.key[pos],
					blk.blk.b.record_ptr[pos]);
			if (i == -1) {
				printf("Error Merging\n");

				return -1;
			}
			FreeBlock(disk, left->blockno);

			left->blockno = right->blockno;	
			writeBlock(disk, left->blockno, left);

			delFromNode(&(parent.blk.b), parent.blk.b.key[pos]);
			writeBlock(disk, blk.blockno, &blk);
			
			blockno = parent.blockno;
		} else if (left->blk.b.count < right->blk.b.count) {
			/* Left shift */
			i = left->blk.b.count;
			strcpy(left->blk.b.key[i], parent.blk.b.key[pos]);
			left->blk.b.record_ptr[i + 1] = 
				parent.blk.b.record_ptr[pos];
			left->blk.b.ptr[i] = right->blk.b.ptr[0];
			++(left->blk.b.count);

			strcpy(parent.blk.b.key[pos], right->blk.b.key[0]);
			parent.blk.b.record_ptr[pos] = 
				right->blk.b.record_ptr[0];
	
			leftShift(&(right->blk.b));
			--(right->blk.b.count);
			
			writeBlock(disk, parent.blockno, &parent);
			writeBlock(disk, left->blockno, left);
			writeBlock(disk, right->blockno, right);

			return 0;
		} else {
			/* Right shift */
			i = left->blk.b.count;
			rightShift(&(right->blk.b));
			++(right->blk.b.count);
			strcpy(right->blk.b.key[0], parent.blk.b.key[pos]);
			right->blk.b.record_ptr[0] = 
				parent.blk.b.record_ptr[pos];
			right->blk.b.ptr[0] = left->blk.b.ptr[i];

			strcpy(parent.blk.b.key[pos], left->blk.b.key[i]);
			parent.blk.b.record_ptr[pos] = 
				left->blk.b.record_ptr[i];
			--(left->blk.b.count);
			
			writeBlock(disk, parent.blockno, &parent);
			writeBlock(disk, left->blockno, left);
			writeBlock(disk, right->blockno, right);

			return 0;
		}

		printf("Warning: Untested function called\n");
	} 

	return 0;
}

int btree_delete(int disk, char name[], int treeno)
{
	struct block blk;
	struct block left;
	struct block right;
	int rptr;
	int pos;
	int i;
	int temp;

	i = bnode_search(disk, name, &pos, &rptr, treeno);

	/* Check & free the memory allocated to file */
	readBlock(disk, rptr, &blk);
	if (blk.type == 1) {
		if (blk.blockno == 0) {
			printf("Can't delete root directory\n");
			return -1;
		} else if (blk.blk.b.count > 2) {
			printf("Directory not empty!\n");
			return -1;
		}
	} else if (blk.type != 0) {
		printf("Error can't delete\n");
		return -1;
	}

	FreeBlock(disk, rptr);

	readBlock(disk, i, &blk);

	if (blk.blk.b.is_leaf == true) {
		delFromNode(&(blk.blk.b), name);
		writeBlock(disk, blk.blockno, &blk);

		if (blk.blk.b.count < (MAXDEG / 2 - 1)) {
			balance(disk, blk.blockno, name);
		}
	} else {
		readBlock(disk, blk.blk.b.ptr[pos], &left);
		readBlock(disk, blk.blk.b.ptr[pos + 1], &right);

		if (left.blk.b.count >= MAXDEG / 2) {
			temp = left.blk.b.count - 1;
			strcpy(blk.blk.b.key[pos],  left.blk.b.key[temp]);
			blk.blk.b.record_ptr[pos] = 
				left.blk.b.record_ptr[temp];
			btree_delete(disk, left.blk.b.key[temp], left.blockno);
			writeBlock(disk, blk.blockno, &blk);
		} else if (right.blk.b.count >= MAXDEG / 2) {
			temp = right.blk.b.count - 1;
			strcpy(blk.blk.b.key[pos],  right.blk.b.key[temp]);
			blk.blk.b.record_ptr[pos] = 
				right.blk.b.record_ptr[temp];
			btree_delete(disk, right.blk.b.key[temp], 
					right.blockno);
			writeBlock(disk, blk.blockno, &blk);
		} else {
			/* Merge */
			i = bBlock_merge(&left, &right, blk.blk.b.key[pos],
					blk.blk.b.record_ptr[pos]);
			if (i == -1) {
				printf("Error Merging\n");

				return -1;
			}
			FreeBlock(disk, left.blockno);

			left.blockno = right.blockno;	
			writeBlock(disk, left.blockno, &left);

			delFromNode(&(blk.blk.b), name);
			writeBlock(disk, blk.blockno, &blk);

			btree_delete(disk, name, left.blockno);

			if (blk.blk.b.count == 0) {
				FreeBlock(disk, left.blockno);
				left.blockno = blk.blockno;
				writeBlock(disk, left.blockno, &left);
			} else {
				balance(disk, blk.blockno, name);
			}
		}

	}

	return 0;
}
