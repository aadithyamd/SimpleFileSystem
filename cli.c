#include "btree.c"

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

void intro_message()
{
	printf("Simple File system v2 (Nov 09 2018, 14:31:34)\n" 
		"[GCC 7.3.0] on linux \n"
		"Type help for more information.\n");
}
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
	if (s1[0] == EOF) {
		printf("exit\n");
		strcpy(s1,"exit");
		
	}

	return strlen(s1);
}


int main(int argc, char const *argv[])
{
	int disk;
	int i;
	char ch;
	char command[81];
	char filename[100];
	char line[100];

	disk = openDisk("hd1", DISKSIZE);
	intro_message();
	while (strcmp(command, "exit") != 0) {
		printf(GREEN "Learners@FileSystem$ " RESET);
		readData(line, 100);	
		memset(command, 0, sizeof(command));
		sscanf(line, "%s %s", command, filename);

		/*printf("Command: %s\n", command); */
		if (command[0] == '\n' || strcmp(command, "") == 0) {
			continue;

		} else if (strcmp(command, "clear") == 0) {
			system("clear");
		} else if (strcmp(command, "ls") == 0) {
			inorder(disk, 0);
		} else if (strcmp(command, "cat") == 0) {
		/*	sscanf(line, "%s", filename);
			printf("%s %s\n", command, filename);
		*/	i = btree_search(disk, filename);
			if (i == -1) {
				printf("cat: %s: No such file or directory\n", 
						filename);
			} else {
				cat_file(disk, i);
			}
		} else if (strcmp(command, "vim") == 0) {
		/*	sscanf(line, "%s", filename);
			printf("(Filename: %s)", filename); */
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
			
			while ((getchar()) != '\n');
			
			flag = 0;
			input_buffer[i - 2] = '\0';
			i = btree_search(disk, filename);
			if (i == -1) {
				btree_insert(disk, filename, input_buffer);
			} else {
				modify_file(disk, i, input_buffer);
			}

			/* to flush out unwanted data in the input_buffer */
			memset(input_buffer, 0, sizeof(input_buffer));
		} else if (strcmp(command, "help") == 0) {
			printf("ls             : List all awailable files.\n");
			printf("cat <filename> : Show the content of given "
					"fileif exist else show error.\n");
			printf("vim <filename> : Opens a given file if it"
				" exists else creates new one to modify.\n");
			printf("clear          : Clear the screen.\n");
			printf("exit           : To exit from the terminal."
					" Alternate : press (CTRL + d) \n");
		}

		memset(line, 0, sizeof(line));
		memset(filename, 0, sizeof(filename));
	}
	printf("logout\n");
	closeDisk(disk);

	return 0;
}
