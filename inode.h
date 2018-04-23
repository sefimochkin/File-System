//
// Created by Сергей Ефимочкин on 18.04.2018.
//

#ifndef FS_INODE_H
#define FS_INODE_H

#include "superblock.h"
#include "block.h"

struct inode {
    int size_of_name_in_chars;
    int size_of_file_in_chars;

    unsigned int index_of_blocks_array_of_name;
    unsigned int index_of_blocks_array_of_file;

    _Bool is_directory;
    int number_of_files_in_directory;
    unsigned int index_of_blocks_array_of_inodes;

    unsigned int index_of_owner_inode;
    unsigned int number_of_inode;
};

struct inode * create_file(struct superblock *sb, char* name, char* file, int size_of_name, int size_of_file, struct inode* owner);
struct inode * create_directory(struct superblock *sb, char* name, int size_of_name, struct inode* owner);
void add_file_to_directory(struct superblock *sb, struct inode* directory, struct inode* added_inode);


void delete_file(struct superblock *sb, struct inode* inode);
void delete_directory(struct superblock *sb, struct inode* inode);
void delete_file_from_directory(struct superblock *sb, struct inode* deleted_inode);

char* get_file_name(struct superblock *sb, struct inode* inode);
char* open_file(struct superblock *sb, struct inode* inode);

char* get_file_names_from_directory(struct superblock *sb, struct inode* directory);

struct inode* get_inode_by_name(struct superblock *sb, char*name, struct inode* directory);
_Bool check_doubling_name(struct superblock *sb, char*name, struct inode* directory);

int get_size_of_data_in_blocks(struct superblock *sb, int size_of_data);

#endif //FS_INODE_H
