#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    char ch;
    char command[20];
    char filename[100];

    while (strcmp(command, "exit") != 0) {
        printf("Learners@FileSystem$ ");
        scanf("%s", command);

        if (command[0] == '\n' || strcmp(command, "") == 0) {
            printf("Learners@FileSystem$ ");
            scanf("%s", command);
        
        } else if (strcmp(command, "ls") == 0) {
            printf("Learners@FileSystem$ ");
            printf("%s\n", command);
            
        } else if (strcmp(command, "cat") == 0) {
            scanf("%s", filename);
            printf("%s %s\n", command, filename);
        
        } else if (strcmp(command, "vim") == 0) {
            scanf("%s", filename);
            printf("%s %s\n", command, filename);
            
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
                    input_buffer[i] = ch;
                }
            }
            flag = 0;

            printf("%s\n", input_buffer);
            /* to flush out unwanted data in the input_buffer */
            memset(input_buffer,0,sizeof(input_buffer));

        } 

    }

    return 0;
}
