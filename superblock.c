//
// Created by Сергей Ефимочкин on 18.04.2018.
//
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "superblock.h"

_Bool get_nth_bit(char byte, int bit)
{
    return 1 == ( (byte >> bit) & 1);
}

void set_nth_bit(char *byte, unsigned int bit, _Bool value){
    *byte = (char) ((*byte & (~(1 << bit))) | (value << bit));
}

struct inode * get_free_inode(struct superblock *sb){
    for(unsigned int k = 0; k < sb->number_of_inods; k++){
        unsigned int i = k / 8;
        unsigned int j = k % 8;
        char byte = ((char*)sb->inods_bitmap)[i];
        if(!get_nth_bit(byte, j)){
            set_nth_bit(&((char*)sb->inods_bitmap)[i], j, 1);
            struct inode* inode = &sb->inods_array[k];

            sb->number_of_free_inods--;
            return inode;
        }
    }

    return 0;
}

struct block* get_free_block(struct superblock *sb){
    for(unsigned int k = 0; k < sb->number_of_blocks; k++){
        unsigned int i = k / 8;
        unsigned int j = k % 8;
        char byte = ((char*)sb->blocks_bitmap)[i];
        if(!get_nth_bit(byte, j)){
            set_nth_bit(&((char*)sb->blocks_bitmap)[i], j, 1);
            struct block* block = &sb->blocks_array[k];

            sb->number_of_free_blocks--;
            return block;
        }
    }
}

struct block* get_n_continuous_free_blocks(struct superblock *sb, int number_of_blocks) {
    if(number_of_blocks > 0){
    unsigned int* blocks_numbers = (unsigned int*) malloc(sizeof(unsigned int)*number_of_blocks);
    int n = 0;
    for(unsigned int k = 0; k < sb->number_of_blocks; k++) {
        unsigned int i = k / 8;
        unsigned int j = k % 8;
        char byte = ((char*)sb->blocks_bitmap)[i];
        if(!get_nth_bit(byte, j)) {
            blocks_numbers[n] = k;
            if(n == number_of_blocks-1) {
                struct block* first_block = &sb->blocks_array[blocks_numbers[0]];

                for(n = 0; n < number_of_blocks; n++) {
                    i = blocks_numbers[n] / 8;
                    j = blocks_numbers[n] % 8;
                    set_nth_bit(&((char*)sb->blocks_bitmap)[i], j, 1); //marking as used
                }

                free(blocks_numbers);
                sb->number_of_free_blocks -= number_of_blocks;
                return  first_block;
            }
            n++;
        }
        else
            n = 0; //start searching again
    }
    free(blocks_numbers);
    }
    else return NULL;
    return 0;
}


void free_inode(struct superblock *sb, struct inode* inode){
    free_data_in_blocks(sb, &(sb->blocks_array[inode->index_of_blocks_array_of_name]), inode->size_of_name_in_chars, inode->is_directory);
    if(inode->is_directory)
        free_data_in_blocks(sb, &(sb->blocks_array[inode->index_of_blocks_array_of_inodes]), inode->number_of_files_in_directory, inode->is_directory);
    else
        free_data_in_blocks(sb, &(sb->blocks_array[inode->index_of_blocks_array_of_file]), inode->size_of_file_in_chars, inode->is_directory);
    unsigned int i = inode->number_of_inode / 8;
    unsigned int j = inode->number_of_inode % 8;
    set_nth_bit(&((char*)sb->inods_bitmap)[i], j, 0);

    sb->number_of_free_inods++;
}

void free_block(struct superblock *sb, struct block* block){
    unsigned int i = block->number_of_block / 8;
    unsigned int j = block->number_of_block % 8;
    set_nth_bit(&((char*)sb->blocks_bitmap)[i], j, 0);
    sb->number_of_free_blocks++;
}

void put_index_in_address_block(struct superblock *sb, char* address_block, unsigned int index){
    char * index_str = malloc(sizeof(char) * sb->number_of_chars_in_index);
    memset(index_str, 0, sb->number_of_chars_in_index);
    sprintf(index_str, "%u", index);
    strncpy(address_block, index_str, sb->number_of_chars_in_index);
    free(index_str);
}

struct inode* get_inode_by_index_in_address_block(struct superblock *sb, char* address_block){
    char * index_str = malloc(sizeof(char) * sb->number_of_chars_in_index);
    memset(index_str, 0, sb->number_of_chars_in_index);
    strncpy(index_str, address_block, sb->number_of_chars_in_index);
    unsigned int index;
    sscanf(index_str, "%u", &index);
    struct inode* inode = &sb->inods_array[index];
    free(index_str);
    return inode;
}

struct block* get_block_by_index_in_address_block(struct superblock *sb, char* address_block){
    char * index_str = malloc(sizeof(char) * sb->number_of_chars_in_index);
    memset(index_str, 0, sb->number_of_chars_in_index);

    strncpy(index_str, address_block, sb->number_of_chars_in_index);
    unsigned int index;
    sscanf(index_str, "%u", &index);
    struct block* block = &sb->blocks_array[index];
    free(index_str);
    return block;
}