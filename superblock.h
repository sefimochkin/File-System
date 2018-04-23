//
// Created by Сергей Ефимочкин on 18.04.2018.
//

#ifndef FS_SUPERBLOCK_H
#define FS_SUPERBLOCK_H

#include "inode.h"
#include "block.h"

struct superblock {
    int number_of_blocks;
    int number_of_inods;
    int number_of_bytes_in_block;
    int number_of_chars_in_index;
    int number_of_free_blocks;
    int number_of_free_inods;

    void* inods_bitmap;
    void* blocks_bitmap;

    struct inode* inods_array;
    struct block* blocks_array;
    char* blocks_data_array;
};

struct inode * get_free_inode(struct superblock *sb);
struct block* get_free_block(struct superblock *sb);

struct block* get_n_continuous_free_blocks(struct superblock *sb, int number_of_blocks);

void free_inode(struct superblock *sb, struct inode* inode);
void free_block(struct superblock *sb, struct block* block);

void put_index_in_address_block(struct superblock *sb, char* address_block, unsigned int index);
struct inode* get_inode_by_index_in_address_block(struct superblock *sb, char* address_block);
struct block* get_block_by_index_in_address_block(struct superblock *sb, char* address_block);


#endif //FS_SUPERBLOCK_H
