//
// Created by Сергей Ефимочкин on 18.04.2018.
//

#ifndef FS_FILESYSTEM_H
#define FS_FILESYSTEM_H
#include "inode.h"
#include "superblock.h"

void * get_memory_for_filesystem();

struct inode * create_filesystem(char* filesystem);
struct inode * open_filesystem(char* file_system_name, char* filesystem);
void save_filesystem(char* file_system_name, char* filesystem);
void close_filesystem(char* file_system_name, char* filesystem);

char* ls(struct superblock *sb, struct inode* directory);
void mkdir(struct superblock *sb, char* name, struct inode* directory);
void rmdir(struct superblock *sb, char* name, struct inode* directory);
void rm(struct superblock *sb, char* name, struct inode* directory);
void touch(struct superblock *sb, char* name, char* input, struct inode* directory);
char* read_file(struct superblock *sb, char* name, struct inode* directory);
void cd(struct superblock *sb, char*name, struct inode** current_directory);
void import_file(struct superblock *sb, char* inner_name, char* outer_name, struct inode* directory);
void export_file(struct superblock *sb, char* inner_name, char* outer_name, struct inode* directory);

#endif //FS_FILESYSTEM_H
