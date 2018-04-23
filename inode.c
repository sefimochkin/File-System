//
// Created by Сергей Ефимочкин on 18.04.2018.
//
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "inode.h"


struct inode * create_file(struct superblock *sb, char* name, char* file, int size_of_name, int size_of_file, struct inode* owner){
    struct inode *inode = get_free_inode(sb);

    inode->is_directory = 0;
    inode->index_of_blocks_array_of_name = put_data_in_blocks(sb, name, size_of_name);
    inode->index_of_blocks_array_of_file = put_data_in_blocks(sb, file, size_of_file);
    inode->size_of_name_in_chars = size_of_name;
    inode->size_of_file_in_chars = size_of_file;
    inode->number_of_files_in_directory = 0;

    if(owner != NULL) {
        inode->index_of_owner_inode = owner->number_of_inode;
        add_file_to_directory(sb, owner, inode);
    }

    return inode;
}


struct inode * create_directory(struct superblock *sb, char* name, int size_of_name, struct inode* owner){
    struct inode *inode = get_free_inode(sb);

    inode->is_directory = 1;
    inode->index_of_blocks_array_of_name = put_data_in_blocks(sb, name, size_of_name);
    inode->number_of_files_in_directory = 0;
    inode->size_of_name_in_chars = size_of_name;

    if(owner != NULL){
        inode->index_of_owner_inode = owner->number_of_inode;
        add_file_to_directory(sb, owner, inode);
    }

    return inode;
}


void add_file_to_directory(struct superblock *sb, struct inode* directory, struct inode* added_inode){
    int number_of_inodes = directory->number_of_files_in_directory;
    if(number_of_inodes * sizeof(unsigned int) % sb->number_of_bytes_in_block == 0) {
        // in case all space in given blocks is gone

        struct inode** inodes_addresses = malloc(sizeof(void*) * (number_of_inodes + 1));

        int number_of_address_blocks = number_of_inodes * sizeof(unsigned int) / sb->number_of_bytes_in_block;

        struct block** previous_inodes_address_blocks = malloc(sizeof(void *) * number_of_address_blocks);


        for (int k = 0; k < number_of_inodes; k++) {
            int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
            int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;

            struct block *address_block = &((&sb->blocks_array[directory->index_of_blocks_array_of_inodes])[i]);
            struct inode* inode = get_inode_by_index_in_address_block(sb, address_block->data + j);

            inodes_addresses[k] = inode;
            previous_inodes_address_blocks[i] = address_block;
        }

        inodes_addresses[number_of_inodes] = added_inode;

        //cleaning up
        for (int i = 0; i < number_of_address_blocks; i++)
            free_block(sb, previous_inodes_address_blocks[i]);


        //putting all inodes into new blocks
        struct block *new_address_blocks = get_n_continuous_free_blocks(sb, number_of_address_blocks + 1);
        for (int k = 0; k < number_of_inodes + 1; k++) {
            int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
            int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;

            struct block *address_block = &new_address_blocks[i];

            put_index_in_address_block(sb, address_block->data + j, inodes_addresses[k]->number_of_inode);
        }
        directory->index_of_blocks_array_of_inodes = new_address_blocks->number_of_block;

        //more cleaning up
        free(inodes_addresses);
        free(previous_inodes_address_blocks);
    }
    else{
        int k = directory->number_of_files_in_directory;
        int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
        int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;

        struct block *address_block = &((&sb->blocks_array[directory->index_of_blocks_array_of_inodes])[i]);

        put_index_in_address_block(sb, address_block->data + j, added_inode->number_of_inode);

    }

    directory->number_of_files_in_directory++;
}


void delete_file(struct superblock *sb, struct inode* inode){
    if(inode->is_directory)
        delete_directory(sb, inode);
    else
        delete_file_from_directory(sb, inode);
}


void delete_directory(struct superblock *sb, struct inode* inode){
    struct inode **inodes_addresses = malloc(sizeof(struct inode *) * inode->number_of_files_in_directory);

    int number_of_adress_blocks = get_number_of_address_blocks(sb, inode->number_of_files_in_directory);
    struct block **inodes_address_blocks = malloc(sizeof(struct block *) * number_of_adress_blocks);

    for (int k = 0; k < inode->number_of_files_in_directory; k++) {
        int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
        int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;

        struct block *address_block = &((&sb->blocks_array[inode->index_of_blocks_array_of_inodes])[i]);
        struct inode* inode_for_adding = get_inode_by_index_in_address_block(sb, address_block->data + j);

        inodes_addresses[k] = inode_for_adding;
        inodes_address_blocks[i] = address_block;
    }

    //cleaning up
    for (int i = 0; i < number_of_adress_blocks; i++)
        free_block(sb, inodes_address_blocks[i]);

    for (int i = 0; i < inode->number_of_files_in_directory; i++) {
        struct inode* inode_for_deleting = inodes_addresses[i];
        if (inode_for_deleting->is_directory)
            delete_directory(sb, inode_for_deleting);
        else
            delete_file_from_directory(sb, inode_for_deleting);
    }

    //more cleaning up
    free(inodes_addresses);
    free(inodes_address_blocks);

    delete_file_from_directory(sb, inode);
}


void delete_file_from_directory(struct superblock *sb, struct inode* deleted_inode){
    struct inode* directory = &(sb->inods_array[deleted_inode->index_of_owner_inode]);
    int number_of_inodes = directory->number_of_files_in_directory;
    struct inode** inodes_addresses = malloc(sizeof(void*) * (number_of_inodes - 1));

    int number_of_adress_blocks = get_number_of_address_blocks(sb, number_of_inodes);
    struct block** previous_inodes_address_blocks = malloc(sizeof(void *) * number_of_adress_blocks);
    int s = 0;
    for (int k = 0; k < number_of_inodes; k++) {
        int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
        int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;

        struct block *address_block = &((&sb->blocks_array[directory->index_of_blocks_array_of_inodes])[i]);
        struct inode* inode_for_saving = get_inode_by_index_in_address_block(sb, address_block->data + j);

        if(inode_for_saving->number_of_inode != deleted_inode->number_of_inode){
            //these ones we save!
            inodes_addresses[s] = inode_for_saving;
            s++;
        }
        previous_inodes_address_blocks[i] = address_block;
    }

    //cleaning up
    for (int i = 0; i < number_of_adress_blocks; i++)
        free_block(sb, previous_inodes_address_blocks[i]);

    //deleting the file
    free_inode(sb, deleted_inode);

    //putting all inodes into new blocks
    int number_of_new_address_blocks = get_number_of_address_blocks(sb, number_of_inodes-1);

    struct block *new_address_blocks = get_n_continuous_free_blocks(sb, number_of_new_address_blocks);
    for (int k = 0; k < number_of_inodes - 1; k++) {
        int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
        int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;

        struct block *address_block = &new_address_blocks[i];

        put_index_in_address_block(sb, address_block->data + j, inodes_addresses[k]->number_of_inode);
    }
    if(number_of_new_address_blocks > 0)
        directory->index_of_blocks_array_of_inodes = new_address_blocks->number_of_block;

    directory->number_of_files_in_directory--;

    //more cleaning up
    free(inodes_addresses);
    free(previous_inodes_address_blocks);
}


char* get_file_name(struct superblock *sb, struct inode* inode){
    return read_data_in_blocks(sb, &sb->blocks_array[inode->index_of_blocks_array_of_name], inode->size_of_name_in_chars);
}


char* open_file(struct superblock *sb, struct inode* inode){
    return read_data_in_blocks(sb, &sb->blocks_array[inode->index_of_blocks_array_of_file], inode->size_of_file_in_chars);
}


char* get_file_names_from_directory(struct superblock *sb, struct inode* directory){
    if(directory->number_of_files_in_directory > 0){
        int number_of_inodes = directory->number_of_files_in_directory;
        struct inode** inodes_addresses = malloc(sizeof(void*) * number_of_inodes);
        for (int k = 0; k < number_of_inodes; k++) {
            int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
            int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;

            struct block *address_block = &((&sb->blocks_array[directory->index_of_blocks_array_of_inodes])[i]);
            struct inode* inode = get_inode_by_index_in_address_block(sb, address_block->data + j);

            inodes_addresses[k] = inode;
        }

        int output_length = 0;
        for (int i = 0; i < number_of_inodes; i++)
            output_length += inodes_addresses[i]->size_of_name_in_chars;

        output_length += directory->number_of_files_in_directory - 1;

        char* output_str = malloc(sizeof(char) * output_length + 1);
        int i = 0;
        for (int k = 0; k < directory->number_of_files_in_directory; k++){
            char* inode_name = get_file_name(sb, inodes_addresses[k]);
            for(int j = 0; j < inodes_addresses[k]->size_of_name_in_chars; j++){
                output_str[i] = inode_name[j];
                i++;
            }
            if(i == output_length)
                output_str[i]='\0';
            else if(i != output_length)
                output_str[i] = '\n';
            i++;
            //cleaning up
            free(inode_name);
        }

        free(inodes_addresses);

        return output_str;
    }
    else {
        return "No files in directory!";
    }
}


struct inode* get_inode_by_name(struct superblock *sb, char*name, struct inode* directory){
    if(directory->number_of_files_in_directory > 0){
        int number_of_inodes = directory->number_of_files_in_directory;

        for (int k = 0; k < number_of_inodes; k++) {
            int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
            int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;

            struct block *address_block = &((&sb->blocks_array[directory->index_of_blocks_array_of_inodes])[i]);
            struct inode* inode = get_inode_by_index_in_address_block(sb, address_block->data + j);
            char* inode_name = get_file_name(sb, inode);

            if(strncmp(inode_name, name, (size_t)inode->size_of_name_in_chars) == 0){
                free(inode_name);
                return inode;
            }
            free(inode_name);
        }
        printf("No files with this name!\n");
        return NULL;
    }
    else {
        printf("No files in directory!\n");
        return NULL;
    }

}

_Bool check_doubling_name(struct superblock *sb, char*name, struct inode* directory){
    if(directory->number_of_files_in_directory > 0) {
        int number_of_inodes = directory->number_of_files_in_directory;

        for (int k = 0; k < number_of_inodes; k++) {
            int i = k * sb->number_of_chars_in_index / sb->number_of_bytes_in_block;
            int j = k * sb->number_of_chars_in_index % sb->number_of_bytes_in_block;

            struct block *address_block = &((&sb->blocks_array[directory->index_of_blocks_array_of_inodes])[i]);
            struct inode *inode = get_inode_by_index_in_address_block(sb, address_block->data + j);
            char *inode_name = get_file_name(sb, inode);

            if (strncmp(inode_name, name, (size_t) inode->size_of_name_in_chars) == 0) {
                free(inode_name);
                return 0;
            }
            free(inode_name);
        }
    }
    return 1;
}


int get_size_of_data_in_blocks(struct superblock *sb, int size_of_data){
    int size = 0;
    size += get_number_of_address_blocks(sb, size_of_data);

    int added_block = 0;
    if (size_of_data % sb->number_of_bytes_in_block > 0)
        added_block = 1;

    size += size_of_data / sb->number_of_bytes_in_block + added_block;

    return size;
}



