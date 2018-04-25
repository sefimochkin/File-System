#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "FileSystem.h"

int main(int argc, char* argv[]) {

    if(argc < 2){
        fprintf(stderr, "\nNot enough arguments!\n");
        exit(1);
    }
    char *path = argv[1];

    char* name = malloc(sizeof(char) * (strlen(path)+3));
    strncpy(name, "./", 2);
    strncpy(&name[2], path, strlen(path) + 1);

    void* filesystem = get_memory_for_filesystem();
    unsigned int a = sizeof(struct inode);

    char command[32];
    char first_argument[128];
    char second_argument[1024];
    char input_line[1184];

    struct inode* root = open_filesystem(name, (char*)filesystem);

    struct superblock* sb = (struct superblock *) filesystem;
    struct inode** current_directory = &root;
    //mkdir(sb, "test", root);
    //char* name1 = get_file_name(sb, get_inode_by_name(sb, "test", root));
    //mkdir(sb, "test2", root);


    while(fgets(input_line, 1184, stdin) != 0){
        int number_of_arguments = sscanf(input_line, "%s %s %s", command, first_argument, second_argument);

        if(strcmp(command, "ls") == 0){
            char* output = ls(sb, *current_directory);
            printf("%s\n",output);
            if((*current_directory)->number_of_files_in_directory > 0)
                free(output);
        }

        else if (strcmp(command, "mkdir") == 0){
            if(number_of_arguments == 1)
                printf("Not sufficient arguments!\n");
            else
                mkdir(sb, first_argument, *current_directory);
        }

        else if (strcmp(command, "rmdir") == 0){
            if(number_of_arguments == 1)
                printf("Not sufficient arguments!\n");
            else
                rmdir(sb, first_argument, *current_directory);
        }

        else if (strcmp(command, "touch") == 0){
            if(number_of_arguments < 3)
                printf("Not sufficient arguments!\n");
            else
                touch(sb, first_argument, second_argument, *current_directory);
        }

        else if (strcmp(command, "rm") == 0){
            if(number_of_arguments == 1)
                printf("Not sufficient arguments!\n");
            else
                rm(sb, first_argument, *current_directory);
        }

        else if (strcmp(command, "read") == 0){
            if(number_of_arguments == 1)
                printf("Not sufficient arguments!\n");
            else {
                char *output = read_file(sb, first_argument, *current_directory);
                if(output != NULL) {
                    printf("%s\n", output);
                    free(output);
                }

            }
        }

        else if (strcmp(command, "cd") == 0){
            if(number_of_arguments == 1)
                printf("Not sufficient arguments!\n");
            else
                cd(sb, first_argument, current_directory);
        }

        else if (strcmp(command, "import") == 0){
            if(number_of_arguments < 3)
                printf("Not sufficient arguments!\n");
            else
                import_file(sb, first_argument, second_argument, *current_directory);
        }

        else if (strcmp(command, "export") == 0){
            if(number_of_arguments < 3)
                printf("Not sufficient arguments!\n");
            else
                export_file(sb, first_argument, second_argument, *current_directory);
        }

        else if (strcmp(command, "save") == 0){
            save_filesystem(name, filesystem);
        }

        else if (strcmp(command, "help") == 0){
            printf("ls to list files in current directory\ncd $name$ to go to directory named $name$\n"
                   "mkdir $name$ to create directory named $name$\ntouch $name$ $file text$ to create a file "
                   "named $name$ with text of file $file text$\nrmdir $name$ to delete directory named $name$ "
                   "and all files in it\nrm $name$ to delete file named $name$ from directory\n"
                   "read $name$ to print text of file named $name$\nimport $inner name$ $outer name$ to import "
                   "file named $outer name$ from computer's file system and save it as file named $inner name$ in "
                   "this file system\nexport $inner name$ $outer name$ to export file named $inner name$ into "
                   "computer's file system as a file named $outername$\nsave to save all changes made in "
                   "the filesystem\nexit to save and exit\n");
        }

        else if (strcmp(command, "exit") == 0)
            break;
    }

    close_filesystem(name, filesystem);
    free(name);
}