#include "filesys.h"
#include "lib.h"
#include "syscall.h"

#define NUM_DIR_ENTRY_OFFSET 0
#define NUM_INODE_OFFSET 4
#define NUM_DATA_OFFSET 8
#define BLOCK_RSRVED_OFFSET 12
#define DIR_ENTRIES_OFFSET 64
#define UNIT_64_OFFSET 64

#define FILE_NAME_OFFSET 0
#define FILE_TYPE_OFFSET 32
#define INODE_ID_OFFSET 36
#define DENTRY_RSRVED_OFFSET 40
#define UNIT_1_KB 1024
#define MAX_FILES 8

#define BOOT_NUM_DIRS	63
#define FREE_BIT    0xFE
#define MAX_FILE_LEN    32

#define FOUR_BYTES          4
#define BYTE_SIZE           8
#define LOWER_BYTE          0x00FF
#define FOUR_KB             4096
#define INODE_START_ADDR    (FS_START_ADDR + FOUR_KB)
#define MAX_DATA_BLK_IDX    1022

#define CURR_DIR_IDX    0
#define FD_MAX_IDX      8

#define MAX_BUF_IDX     126

/* Current process requesting a system call */
static pcb_t* curr_pcb;

/* Main (global) boot block for the OS */
static boot_block_t *boot_block;

/* Dentry to current opened file if any */
dentry_t cur_dentry;

/* This will hold address of FileSystem which must be set at boot time */
unsigned int FS_START_ADDR;

/* This will hold address of FileSystem's end which must be set at boot time */
unsigned int FS_END_ADDR;

/* This will allow up to 8 opened files at a given time */
// dentry_t file_process[MAX_FILES];

/* Will work as a bitmap to mark opened files*/
// char file_bitmap;

/*  get_filesys_pcb
 *  
 *  INPUT:  none
 *  OUTPUT: lets other files access 
 *  RETURN: global pcb of the filesys
 *  DESCRIPTION: helper function to fill the bitmap for the processes
 *                  
 */
void set_filesys_pcb(pcb_t* pcb) {
    curr_pcb = pcb;
}

/*  fill_bitmap
 *  
 *  INPUT:  index - fd index we want to fill
 *  OUTPUT: new bitmap with respective index filled
 *  RETURN: none
 *  DESCRIPTION: helper function to fill the bitmap for the processes
 *                  
 */
void fill_bitmap(int index) {
    if(index >= FD_MAX_IDX || index < 0) {return;}
    // file_bitmap |= 0x01 << index;
    curr_pcb->fdt[index].flags = 1;
}

/*  free_bitmap
 *  
 *  INPUT:  index - fd index we want to free
 *  OUTPUT: new bitmap with respective index freed
 *  RETURN: none
 *  DESCRIPTION: helper function to free the bitmap for the processes
 *                  
 */
void free_bitmap(int index) {
    if(index >= FD_MAX_IDX || index < 0) {return;}
    // file_bitmap ^= 0x01 << index;    
    curr_pcb->fdt[index].flags = 0;
}

/*  bitmap_open
 *  
 *  INPUT:  index - fd index we want to check status of
 *  OUTPUT: none
 *  RETURN: 1 if the index has no running file (free process)
 *          0 if the index is occupied (full)
 *  DESCRIPTION: helper function to check the bitmap for the processes
 *                  
 */
int32_t bitmap_open(int index) {
    // return (file_bitmap & 0x01 << index) == 0;
    return curr_pcb->fdt[index].flags == 0;
}

/*  fs_set_start_end
 *  
 *  INPUT:  unsigned integers to addresses spanning the file system
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: Sets the starting and ending addresses of the file system
 *                  
 */
void fs_set_start_end(unsigned int start_addr, unsigned int end_addr){
    FS_START_ADDR = start_addr;
    FS_END_ADDR = end_addr;
}

/*  init_cur_dentry
 *  
 *  INPUT:  none
 *  OUTPUT: initializes the cur_dentry with sentinal values
 *  RETURN: none
 *  DESCRIPTION: Initializes a closed file dentry
 *                  
 */
void init_cur_dentry(){
    /* filetype will be used as a sentinel value which is by default -1 (closed file) */
    // cur_dentry = boot_block->direntries;

}



/*  init_boot_block
 *  
 *  INPUT:  none
 *  OUTPUT: initializes the boot block with information
 *  RETURN: none
 *  DESCRIPTION: fills the boot_block struct with intial information 
 *                  
 */
void init_boot_block() {
    boot_block = (boot_block_t*)FS_START_ADDR;
}

/*  get_filetype
 *  
 *  INPUT:  fname - name of the file that we want to check
 *  OUTPUT: the filetype for the queried name
 *  RETURN: 0 for rtc
 *          1 for directory
 *          2 for file
 *          -1 for failure/does not exist
 *  DESCRIPTION: get the filetype for the fname file 
 *                  
 */
int32_t get_filetype(const uint8_t* fname) {
    // Error checking
    if(!fname || (strlen((int8_t*) fname) > MAX_FILE_LEN) || (strlen((int8_t*) fname) == 0)) {return -1;}

    // Get the index from the fname
    int index = 0;  
    int char_idx;
    int found = 0;
    while(index < BOOT_NUM_DIRS) {
        // Check if the index/dentry is valid
        if(boot_block->direntries == NULL) {return -1;}

        // Check if the fname (filename) matches that of the dentry
        char_idx = 0;
        while(char_idx < FILE_TYPE_OFFSET) {
            // Filenames do not match
            if(fname[char_idx] != boot_block->direntries[index].filename[char_idx]) {
                break;
            }

            // Filenames matched fully
            // TODO - MAKE SURE VERYLONG...TXT != VERYLONG...TX
            if(fname[char_idx] == '\0' || (char_idx == FILE_TYPE_OFFSET-1 && (fname[char_idx+1] == '\0'))) {
                found = 1;
                break;
            }

            // Check next character
            char_idx++;
        }
        
        // Check if we found the file index
        if(found == 1) {break;}

        index++;
    }

    if(found == 0) {return -1;}
    else {return boot_block->direntries[index].filetype;}
}

/*  fs_search
 *  
 *  INPUT:  buf - keyboard buffer
 *          idx - current idx into the buffer (next to be filled)
 *  OUTPUT: fills the buffer, and prints the chars to the screen
 *  RETURN: none
 *  DESCRIPTION: tries to complete the buffer with a matching file
 *                  prints out matches (only if there are a couple (5))
 */
int fs_search(uint8_t* buf, int idx) {
    // CURRENTLY VERY BASIC IMPLEMENTATION FINDS AND COPIES FIRST MATCHING FILE
    int start_idx = idx;
    int file_count = boot_block->dir_count;
    int found = 0;
    int matched = 0;
    // uint8_t num_matches;
    uint8_t* compare;
    int i, char_idx;

    if(idx > 0 && buf[idx-1] == ' ') {return idx;}

    while(start_idx > 0) {
        start_idx--;

        // found the starting character
        if(buf[start_idx] == ' ') {
            found = 1;
            start_idx++;
            break;
        } else if(start_idx == 0) {
            found = 1;
        }
    }

    if(found == 1) {
        for(i=0;i<file_count;i++) {
            compare = (uint8_t*) boot_block->direntries[i].filename;
            // strlen((const char*)boot_block->direntries[offset].filename)
            for(char_idx=0;char_idx<MAX_FILE_LEN;char_idx++) {
                if((start_idx + char_idx) == idx && compare[char_idx] != '\0') {matched = 1;break;}
                if(compare[char_idx] != buf[start_idx + char_idx]) {break;}
            }

            if(matched == 1) {
                while(compare[char_idx] != '\0' && char_idx < MAX_FILE_LEN && idx <= MAX_BUF_IDX) {
                    buf[idx] = compare[char_idx];
                    putc(compare[char_idx]);
                    idx++;
                    char_idx++;
                }
                return idx;
            }
        }
    }

    return idx;

}

/*	read_dentry_by_name
 *	
 *	INPUT:  fname - name of the file that we want to read
 *			dentry - place (data structure) we want to put the file information in
 *	OUTPUT: fills the dentry with data from the queried filename
 *	RETURN: 0 on success
 *			-1 on failure
 *	DESCRIPTION: tries to read a file by name and fills the dentry struct with
 *					the retrieved information
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
	// Error checking
	if(!fname || (strlen((int8_t*) fname) > MAX_FILE_LEN)) {return -1;}

	// Get the index from the fname
	int index = 0;	
	int char_idx;
	int found = 0;
	while(index < BOOT_NUM_DIRS) {
		// Check if the index/dentry is valid
		if(boot_block->direntries == NULL) {return -1;}

		// Check if the fname (filename) matches that of the dentry
		char_idx = 0;
		while(char_idx < FILE_TYPE_OFFSET) {
			// Filenames do not match
			if(fname[char_idx] != boot_block->direntries[index].filename[char_idx]) {
				break;
			}

			// Filenames matched fully
            // TODO - MAKE SURE VERYLONG...TXT != VERYLONG...TX
			if(fname[char_idx] == '\0' || (char_idx == FILE_TYPE_OFFSET-1 && (fname[char_idx+1] == '\0'))) {
				found = 1;
				break;
			}

			// Check next character
			char_idx++;
		}
		
		// Check if we found the file index
		if(found == 1) {break;}

		index++;
	}

    if(found == 0) {return -1;}

	// Fill the dentry from the index (read_dentry function provides error checking)
	return read_dentry_by_index(index, dentry);
}


/*	read_dentry_by_index
 *	
 *	INPUT:  index - index in the boot block we need to access
 *			dentry - place (data structure) we want to put the file information in
 *	OUTPUT: fills the dentry with data from the queried index
 *	RETURN: 0 on success
 *			-1 on failure
 *	DESCRIPTION: tries to read a file by index and fills the dentry struct with
 *					the retrieved information
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
	// Error checking
	if(!dentry || index > BOOT_NUM_DIRS || index < 0) {return -1;}

	int i;		// loop counter

	/* Populate the detnry with filename, filetype, inode # */
	dentry_t fromBoot = boot_block->direntries[index];
	
	// Copy filename
	for(i=0;i<FILE_TYPE_OFFSET;i++) {
		dentry->filename[i] = fromBoot.filename[i];
	}
	// Copy filetype
	dentry->filetype = fromBoot.filetype;
	
	// Copy inode #
	dentry->inode_num = fromBoot.inode_num;

	return 0;
}


/*  read_data
 *  
 *  INPUT:  inode - inode index we are reading from
 *          offset - offset from first index to data block in inode
 *          buf - place to copy the data into
 *          length - length of data we are reading
 *  OUTPUT: none
 *  RETURN: 0 on success
 *          -1 on failure
 *  DESCRIPTION: tries to read the data at a certain inode 
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    // Check for valid (in-range) inode
    
    if(inode < 0 || inode > boot_block->inode_count) {return -1;}

    int i, j;               // loop counters
    
    int num_blocks;         // obtain the number of blocks needed
    int data_left = length; // track how much data we have already transfered
    int data_read = 0;      // bytes already read
    int offset_left = offset; // track how much data we have to skip
    
    int curr_block_idx;     // current block idx in memory
    uint8_t* curr_block_ptr;// cuurent pointer to block in memory

    // Get the inode
    inode_t* read_inode = (inode_t *) (INODE_START_ADDR + (inode * FOUR_KB));

	// (change by matt april 1) if length is greater than file just read the whole file
    if(offset + length > read_inode->length){ data_left = read_inode->length - offset /*return -1*/;}

    // Number of blocks needed
    num_blocks = (data_left+offset) % FOUR_KB == 0 ? ((data_left+offset)/FOUR_KB) : (((data_left+offset)/FOUR_KB)+1);

    // Index into the data block and copy into buffer
    for(i=0;i<num_blocks;i++) {
        // Invalid block check
        // if(offset < 0 || (offset+i) > MAX_DATA_BLK_IDX) {return -1;}

        // Read the current block index and get the block number in memory
        curr_block_idx = read_inode->data_block_num[i];

        if(curr_block_idx < 0 || curr_block_idx > MAX_DATA_BLK_IDX) {return -1;}

        // Get the actual data block
        curr_block_ptr = ((uint8_t *) boot_block);
        curr_block_ptr += ((boot_block->inode_count + 1) * FOUR_KB) + (curr_block_idx * FOUR_KB);

        if(offset_left > FOUR_KB) {
            offset_left -= FOUR_KB;
            continue;
        }

        // Copy the next block data
        for(j=0;j<FOUR_KB && data_left>0;j++) {
            // Check if there is offset to skip (this can be optimized)
            if(offset_left > 0) {
                offset_left--;
                curr_block_ptr++;
                continue;
            }

            // Copy a byte of data
            *buf = *curr_block_ptr;

            // Keep track of data_left
            data_left--;
            data_read++;
            // data_left-=BYTE_SIZE;

            // Progress
            buf++;
            curr_block_ptr++;
        } 

    }

    // if(data_left > 0) {return -1;}
    // printf(" Data read: %d \n", data_read);
    return data_read;
}


/*	file_open
 *	
 *	INPUT: fname - string of filename that we want to open
 *	OUTPUT: modifies the bitmap and process array for the filesystem and returns the
 *          opened (on success) file descriptor
 *	RETURN: fd index on success, -1 on failure
 *	DESCRIPTION: tries to open the file with filename==fname if it exists within the bootblock
 *					
 */
int32_t file_open(const uint8_t* fname){
    // Index of the file_process
    int index = 0;

    // Fill cur_dentry
    if( -1 == read_dentry_by_name(fname,&cur_dentry))
        return -1;

    // Trying to open a directory
    if(cur_dentry.filetype == 1)
        return -1;

    // Make sure the file isn't already open
    while(index < MAX_FILES){
        if((!bitmap_open(index) && curr_pcb->fdt[index].inode_num == cur_dentry.inode_num))
            return -1;
        index++;
    }

    // Fill the next open spot in the array
    for(index=0;index<MAX_FILES;index++) {
        if(bitmap_open(index)) {
            fill_bitmap(index);
            curr_pcb->fdt[index].inode_num = cur_dentry.inode_num;
            /* Added to zero file position */
            curr_pcb->fdt[index].file_pos = 0;
            return index;
        }
    }

    return -1;
}

/*	file_close
 *	
 *	INPUT: fd - the file descriptor for the file that we want to close
 *	OUTPUT: frees the bitmap spot if valid
 *	RETURN: 0 on success
 *          -1 on failure
 *	DESCRIPTION: Closes a file respective to the file descriptor passed into the function
 *					
 */
int32_t file_close(int32_t fd) {
    // Make sure the file was opened in the first place
    if(fd >= MAX_FILES || fd < 0 || bitmap_open(fd))
        return -1;

    // Make sure we are closing a file/rtc
    // if(file_process[fd].filetype == 1)
    //     return -1;
    /* Added to zero file position */
    curr_pcb->fdt[fd].file_pos = 0;
    // Free the bitmap
    free_bitmap(fd);

    return 0;
}

/*	file_write
 *	
 *	INPUT: fd - file descriptor we are trying to write to
 *          buf - stuff we are trying to write
 *          nbytes - amount of bytes we want to write
 *	OUTPUT: a updated file
 *	RETURN: -1 on failure (kinda)
 *	DESCRIPTION: tries to write to a file
 *					
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/*	file_read
 *	
 *	INPUT: fd - file descriptor of the file we are trying to read
 *          offset - from the start of the file we are trying to read
 *          nbytes - number of bytes we want to read
 *	OUTPUT: prints nbytes to the screen on success
 *	RETURN: 0 on success
 *          -1 on failure
 *	DESCRIPTION: looks up a file using the file descriptor and tries to read nbytes
 *                  after skipping offset bytes 
 *					
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    buf = (uint8_t*) buf;
    inode_t* read_inode;    // where we read the data from
    int size;               // size of the buffer
    int bytes_read;
    int32_t offset = curr_pcb->fdt[fd].file_pos;



    //printf("\nRead entered using fd: %d\n", fd);

    if(nbytes < 0) {return -1;}

    // Make sure the file is opened so we can read
    if(fd >= MAX_FILES || fd < 0 || bitmap_open(fd))
        return -1;

    // Make sure we are reading a file
    // if(file_process[fd].filetype == 1)
    //     return -1;

    // read_inode = (inode_t *) (INODE_START_ADDR + (file_process[fd].inode_num * FOUR_KB));
    read_inode = (inode_t *) (INODE_START_ADDR + (curr_pcb->fdt[fd].inode_num * FOUR_KB));

    // if(read_inode->length < nbytes) {return -1;}

    // Check how many bytes to read from the file
    if(nbytes >= read_inode->length) {size = read_inode->length;}
    else {size = nbytes;}

    // Read the data
    if(-1 == (bytes_read = read_data(curr_pcb->fdt[fd].inode_num, offset, (uint8_t*)buf, size)))
        return -1;

    // Update the file position
    curr_pcb->fdt[fd].file_pos += bytes_read;

    return bytes_read;
}


/*	dir_open
 *	
 *	INPUT: fname - directory name of the directory we want to read
 *	OUTPUT: changes the bitmap to account for the newly opened directory on success
 *	RETURN: fd index on success
 *          -1 on failure
 *	DESCRIPTION: opens the directory of the filesystem
 *					
 */
int32_t dir_open(const uint8_t* fname){
    // Make sure that 
    if(strncmp(boot_block->direntries[CURR_DIR_IDX].filename, (int8_t*) fname, MAX_FILE_LEN) != 0)
        return -1;

    int32_t index = 0;

	// To pass in point, pass by reference
    if(-1 == read_dentry_by_name(fname, &cur_dentry))
        return -1;

    // Only open directories
    if(cur_dentry.filetype != 1)
        return -1;

    // Make sure the file isn't already open
    while(index < MAX_FILES){
        // if(curr_pcb->fdt[0].inode == cur_dentry.inode_num)
        // if((!bitmap_open(index) && 0 == strncmp(file_process[index].filename, (int8_t*) fname, MAX_FILE_LEN)))
        // if((!bitmap_open(index) && curr_pcb->fdt[index].inode_num == cur_dentry.inode_num))
            // return -1;
        index++;
    }

    // Fill the next open spot in the array
    for(index=0;index<MAX_FILES;index++) {
        if(bitmap_open(index)) {
            fill_bitmap(index);
            curr_pcb->fdt[index].inode_num = cur_dentry.inode_num;
            return index;
        }
    }

    return -1;
}

/*	dir_close
 *	
 *	INPUT: fd - file descriptor of the file we want to open
 *	OUTPUT: updated bitmap on success
 *	RETURN: 0 on success
 *          -1 on failure
 *	DESCRIPTION: tries to close the directory with the file descriptor==fd
 *					
 */
int32_t dir_close(int32_t fd){
    // Make sure the dir was opened in the first place
    if(fd >= MAX_FILES || fd < 0 || bitmap_open(fd))
        return -1;

    // Make sure we are closing a directory
    // if(file_process[fd].filetype != 1)
    //     return -1;

    // Free the bitmap
    free_bitmap(fd);

    return 0;
}

/*	dir_write
 *	
 *	INPUT: fd - file descriptor of the dire we want to write to
 *          buf - what we want to write to the directory
 *          nbytes - number of bytes that we want to write
 *	OUTPUT: a updated directory
 *	RETURN: -1 on failure (kinda)
 *	DESCRIPTION: tries to write to a directory
 *					
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/*	dir_read
 *	
 *	INPUT: fd - file descriptor of the directory we want to read from
 *          offset - offset from the beginning of the directory
 *          nbytes - number of bytes we want to read
 *	OUTPUT: prints the contents of the directory to the screen on succes
 *	RETURN: returns 0 on success
 *          -1 on failure
 *	DESCRIPTION: opens a directory and reads its filename min(nbytes, filename length)
 *					
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){

    buf = (uint8_t*) buf;
    int file_count = boot_block->dir_count;
    uint8_t* ret_buf;
    uint32_t fname_len;
    int32_t offset = curr_pcb->fdt[fd].file_pos;

    // Make sure the directory is opened so we can read
    if(fd >= MAX_FILES || fd < 0 || bitmap_open(fd))
        return -1;

    // Get the length to copy
    fname_len = strlen((const char*)boot_block->direntries[offset].filename);
    if(fname_len > MAX_FILE_LEN) {fname_len = MAX_FILE_LEN;}
    fname_len = fname_len > nbytes ? nbytes : fname_len;
    ret_buf = (uint8_t*) boot_block->direntries[offset].filename;
    memcpy(buf, ret_buf, fname_len);
    // buf[fname_len] = '\0';

    if(curr_pcb->fdt[fd].file_pos < file_count) {
        curr_pcb->fdt[fd].file_pos++;
    } else {
        return 0;
    }

    return fname_len;
}

/*  fs_init
 *  
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: initializes the filesystem
 *                  
 */
void fs_init(){
    init_boot_block();
    init_cur_dentry();
}
