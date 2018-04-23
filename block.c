//
// Created by Сергей Ефимочкин on 22.04.2018.
//
#include <stdlib.h>
#include "block.h"

int get_number_of_address_blocks(struct superblock *sb, int number_of_adresses){
    int added_block = 0;
    if (number_of_adresses * sb->number_of_chars_in_index % sb->number_of_bytes_in_block > 0)
        added_block = 1;
    int number_of_storage_blocks = number_of_adresses * sb->number_of_chars_in_index / sb->number_of_bytes_in_block + added_block;
    return number_of_storage_blocks;
}


unsigned int put_data_in_blocks(struct superblock *sb, char* data, int size_of_data){
    int added_block = 0;
    if (size_of_data % sb->number_of_bytes_in_block > 0)
        added_block = 1;
    int number_of_storage_blocks = size_of_data / sb->number_of_bytes_in_block + added_block;

    int number_of_address_blocks = get_number_of_address_blocks(sb, number_of_storage_blocks);
    struct block* address_blocks = get_n_continuous_free_blocks(sb, number_of_address_blocks);
    for(int k = 0; k < number_of_storage_blocks; k++) {
        int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
        int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;
        struct block* address_block = &address_blocks[i];
        struct block* data_block = get_free_block(sb);
        put_index_in_address_block(sb, address_block->data + j, data_block->number_of_block);
    }

    for(int k = 0; k < size_of_data; k++) {
        //one address block holds links to (number_of_bytes_in_block /  sb->number_of_chars_in_index) blocks.
        //each block can store (having sizeof(char) = 1)  number_of_bytes_in_block chars;
        //so, with number_of_bytes_in_block = 32,  sb->number_of_chars_in_index = 4,
        //one address block can store links to 256 chars.
        //k is index of char in data
        //i is index for address block
        //j is index for link to jth datablock in ith address block
        //s is index of char in jth datablock
        int i = k * sb->number_of_chars_in_index / (sb->number_of_bytes_in_block * sb->number_of_bytes_in_block);
        int j = (k / sb->number_of_bytes_in_block) % (sb->number_of_bytes_in_block / sb->number_of_chars_in_index);
        int s = k % sb->number_of_bytes_in_block;
        struct block* address_block = &address_blocks[i];
        struct block* data_block = get_block_by_index_in_address_block(sb, address_block->data + j *  sb->number_of_chars_in_index);
        data_block->data[s] = data[k];
    }
    return address_blocks->number_of_block;
}


char* read_data_in_blocks(struct superblock *sb, struct block* address_blocks, int size_of_data){
    char* data = (char*) malloc(sizeof(char)*(size_of_data +1));
    for(int k = 0; k < size_of_data; k++) {
        //one address block holds links to (number_of_bytes_in_block / sizeof(char *)) blocks.
        //each block can store (having sizeof(char) = 1)  number_of_bytes_in_block chars;
        //so, with number_of_bytes_in_block = 32, sizeof(char *) = 8 (as in my system),
        //one address block can store links to 128 chars.
        //k is index of char in data
        //i is index for address block
        //j is index for link to jth datablock in ith address block
        //s is index of char in jth datablock
        int i = k * sb->number_of_chars_in_index / (sb->number_of_bytes_in_block * sb->number_of_bytes_in_block);
        int j = (k / sb->number_of_bytes_in_block) % (sb->number_of_bytes_in_block /  sb->number_of_chars_in_index);
        int s = k % sb->number_of_bytes_in_block;
        struct block* address_block = &address_blocks[i];
        struct block* data_block = get_block_by_index_in_address_block(sb, address_block->data + j * sb->number_of_chars_in_index);
        data[k] = data_block->data[s];
    }
    data[size_of_data] = '\0';
    return data;
}

//valgrind --leak-check=full ./my_prog


void free_data_in_blocks(struct superblock *sb, struct block* address_blocks, int size_of_data, _Bool is_inodes){
    int number_of_adress_blocks = get_number_of_address_blocks(sb, size_of_data);
    struct block** address_blocks_array = (struct block**) malloc(sizeof(void *) * number_of_adress_blocks);

    for(int k = 0; k < size_of_data; k++) {
        int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
        int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;
        struct block *address_block = &address_blocks[i];

        if(!is_inodes){  //if they are inodes, that means we are deleting a directory, and
            // directory's inodes are freed manually, because there's a need for recursively deleting all subdirectories
            // and such. But we still need to free the address blocks!
            struct block *data_block = get_block_by_index_in_address_block(sb, address_block->data + j);
            free_block(sb, data_block);}
        address_blocks_array[i] = address_block;
    }

    for(int i = 0; i < number_of_adress_blocks; i++)
        free_block(sb, address_blocks_array[i]);
    free(address_blocks_array);
}