//
// Created by Сергей Ефимочкин on 18.04.2018.
//

#ifndef FS_BLOCK_H
#define FS_BLOCK_H

#include "superblock.h"

struct block{
    char *data;
    unsigned int number_of_block;
};

int get_number_of_address_blocks(struct superblock *sb, int number_of_adresses);
unsigned int put_data_in_blocks(struct superblock *sb, char* data, int size_of_data);
char* read_data_in_blocks(struct superblock *sb, struct block* address_blocks, int size_of_data);
void free_data_in_blocks(struct superblock *sb, struct block* address_blocks, int size_of_data, _Bool is_inodes);

#endif //FS_BLOCK_H
