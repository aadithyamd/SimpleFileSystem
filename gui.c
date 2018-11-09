#include "btree.c"

int readData(char *s1, int length) {
	char ch;
	int i;
	i = 0;
	ch = '\0';
	while (i < length && ch != '\n' && ch != EOF) {
		ch = getc(stdin);
		if(ch != '\n')
		{
			s1[i] = ch;
			++i;
		}
	}
	s1[i] = '\0';

	return strlen(s1);
}


int main(int argc, char const *argv[])
{
	int disk;
	char ch;
	char command[81];
	char filename[100];
	char line[100];

	disk = openDisk("hd1", DISKSIZE);

	while (strcmp(command, "exit") != 0) {
		printf("Learners@FileSystem$ ");
		readData(line, 100);	
		memset(command, 0, sizeof(command));
		sscanf(line, "%s %s", command, filename);

		/*printf("Command: %s\n", command); */
		if (command[0] == '\n' || strcmp(command, "") == 0) {
			continue;

		} else if (strcmp(command, "ls") == 0) {
			inorder(disk, 0);

		} else if (strcmp(command, "cat") == 0) {
		/*	sscanf(line, "%s", filename);
		*/	printf("%s %s\n", command, filename);

		} else if (strcmp(command, "vim") == 0) {
		/*	sscanf(line, "%s", filename);
		*/	printf("Filename: %s", filename);
			char input_buffer[3500];
			int flag = 0;
			int i = -1;

			while (flag != 3) {
				++i;
				scanf("%c", &ch);

				if (ch == ':') {
					flag = 1;
				} else if (ch == 'w' && flag == 1) {
					flag++;
				} else if (ch == 'q' && flag == 2) {
					flag++;
				} else {
					flag = 0;
				}
				input_buffer[i] = ch;
			}
			flag = 0;
			input_buffer[i - 2] = '\0';

			btree_insert(disk, filename, input_buffer);
			/* to flush out unwanted data in the input_buffer */
			memset(input_buffer, 0, sizeof(input_buffer));
			memset(filename, 0, sizeof(filename));
		} 

		memset(line, 0, sizeof(line));
	}
	closeDisk(disk);

	return 0;
}
