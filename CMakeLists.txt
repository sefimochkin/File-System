cmake_minimum_required(VERSION 3.10)
project(FS C)

set(CMAKE_C_STANDARD 99)

add_executable(FS main.c superblock.h inode.h FileSystem.h FileSystem.c superblock.c inode.c block.h block.c)