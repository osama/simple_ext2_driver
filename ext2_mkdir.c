#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ext2.h"

extern char *ext2_image;
extern int addr_root;
extern int debug;

char* finalname;

int main (int argc, char **argv){
	if (argc != 3){	
		printf("usage: ext2_mkdir virtual_disk path\n");
		return 1;
	}

	//Read the disk image into memory
	if (read_image(argv[1])){
		return 1;
	}

	//Find the target directory's address by traversing the given path
	int dir_addr, index;
	Inode *dir, *file;

	if ((dir_addr = traverse_path(argv[2])) == -1){
		fprintf(stderr, "The specified path was not found in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Find the inode for the directory where we will be making a new directory
	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - ROOT_BLOCK * INODE_SIZE];

	if (debug){
		printf(" Directory inode: %d\n", addr_root + dir_addr * INODE_SIZE - ROOT_BLOCK * INODE_SIZE);
	}

	//If a file with the directory's name already exists, we cannot overwrite it
	if ((index = file_exists(dir, finalname)) != -1){
		fprintf(stderr, "The file already exists in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	if (debug){
		printf("File does not already exist.\n");
	}

	//Making a directory entry as another directory
	if ((index = mk_file_entry(dir, finalname, (char) 2, -1)) == -1){
		fprintf(stderr, "Error creating file in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Accessing the new directory's inode to add information
	file = (Inode *) &ext2_image[addr_root + index * INODE_SIZE - ROOT_BLOCK * INODE_SIZE];

	//Writing default information to new inode using information from root inode
	Inode * root = (Inode *) &ext2_image[addr_root];
	file->mode = root->mode;
	file->uid = root->uid;
	file->size = root->size;

	file->groupid = root->groupid;
	file->hard_links = 1;
	file->disk_sectors = root->disk_sectors;

	//Setting access and creation times for new directory
	int ftime = (int) time(NULL);
	file->access_time = ftime;
	file->creation_time = ftime;
	file->modification_time = ftime;
	
	//Finding a data block to store new directory's entries
	file->db[0] = find_free_block();

	if (debug)
		printf("New dir's inode: %d\n New dir's inode byte: %d\n", 
			addr_root + index * INODE_SIZE - ROOT_BLOCK * INODE_SIZE, index);

	//If there are no data blocks available, perform cleanup
	if (file->db[0] == -1){
		fprintf(stderr, "No more space for new file or directory.\n");
		toggle_inode_bitmap(index);
		rm_file_entry(dir, finalname);
		close_image();
		return 1;
	}

	if (debug){
		printf("New directory's data block: %d\n", file->db[0]);
	}

	//Setting . and .. pointers for the new directory
	int data_index = BLOCK_SIZE * file->db[0];

	Dir_entry *current = (Dir_entry *) &ext2_image[data_index];
	Dir_entry *parent = (Dir_entry *) &ext2_image[data_index + 12];

	current->inode = index;
	current->size = 12;
	current->name_length = 1;
	current->type = 2;
	current->name[0] = '.';

	parent->inode = dir_addr;
	parent->size = BLOCK_SIZE - 12;
	parent->name_length = 1;
	parent->type = 2;
	parent->name[0] = '.';
	parent->name[1] = '.';

	//Modifying the count of unallocated data blocks and inodes
	sb_unallocated_count(-1, -1);

	close_image();
	return 0;
}