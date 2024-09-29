#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"
#include "syscall.h"

#define DENTRY_RES_SIZE 24

#define DATA_BLOCK_SIZE 1023

#define BOOT_RES_SIZE	52
#define BOOT_NUM_DIRS	63
#define FILENAME_LEN 	32

#define MAX_BRANCH		5

/* Structure to represent a directory entry */
typedef struct dentry_desc {
	int8_t	    filename[FILENAME_LEN];	
	int32_t     filetype; 	
	int32_t     inode_num;
	int8_t	    reserved[DENTRY_RES_SIZE];
} dentry_t;

/* Structure to represent a specific file */
typedef struct inode_desc {
	int32_t 	length;
	int32_t 	data_block_num[DATA_BLOCK_SIZE];
} inode_t;

/* Represents the parent block */
typedef struct boot_block_desc {
	int32_t 	dir_count;
	int32_t 	inode_count;
	int32_t 	data_count;
	int8_t 		reserved[BOOT_RES_SIZE];
	dentry_t 	direntries[BOOT_NUM_DIRS];
} boot_block_t;

/* Tree search structure for autocomplete */
typedef struct fs_tree_desc {
	uint8_t	 	label;
	struct 		fs_tree_t* 	child_one;
	struct 		fs_tree_t* 	child_two;
	struct 		fs_tree_t* 	child_three;
	struct 		fs_tree_t* 	child_four;
	struct 		fs_tree_t* 	child_five;
} fs_tree_t;


/* Returns the point to the global pcb in filesys */
void set_filesys_pcb(pcb_t* pcb);

/* Sets starting/ending address of file system - call in kernel.c */
void fs_set_start_end(unsigned int start_addr, unsigned int end_addr);

/* Initialized cur_dentry structure (closed filed) */
void init_cur_dentry();

/* Initialize the boot block */
void init_boot_block();

/* Get the filetype from a filename */
int32_t get_filetype(const uint8_t* fname);

/* File a matching file in the filesystem */
int fs_search(uint8_t* buf, int idx);

/* Fills the dentry with filename, filetype, and inode # of the file */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

/* Fills the dentry with filename, filetype, and inode # of the file */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/* Reads the data from the data blocks in memory */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* Opens a file and fills out temporary information */
int32_t file_open(const uint8_t* fname);

/* Reverses the open function */
int32_t file_close(int32_t fd);

/* Write to a specified file */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

/* Read from a specified file */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

/* Opens a directory and fills out temporary information */
int32_t dir_open(const uint8_t* fname);

/* Reverse the open function */
int32_t dir_close(int32_t fd);

/* Write to a specified directory */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

/* Read from a specified directory */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

/* Initialize the file system */
void fs_init();

/* Fills the bitmap at index bit */
void fill_bitmap(int index);

/* Frees the bitmap at index bit */
void free_bitmap(int index);

/* Checks if the bitmap is open at index bit */
int32_t bitmap_open(int index);


#endif 
