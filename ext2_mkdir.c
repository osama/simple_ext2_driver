#include <stdio.h>
#include <stdlib.h>
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
	//TODO: Write Inode properly
	file->db[0] = find_free_block();

	if (debug)
		printf("New dir's inode byte: %d\n", addr_root + index * INODE_SIZE - ROOT_BLOCK * INODE_SIZE);

	//If there are no data blocks available, perform cleanup
	if (file->db[0] == -1){
		fprintf(stderr, "No more space for new file or directory.\n");
		toggle_inode_bitmap(index);
		rm_file_entry(dir, finalname);
		return 1;
	}

	//Setting . and .. pointers for the new directory
	int data_index = BLOCK_SIZE * file->db[0];

	Dir_entry *current = (Dir_entry *) &ext2_image[data_index];
	Dir_entry *parent = (Dir_entry *) &ext2_image[data_index + 12];

	current->inode = index;
	current->size = 12;
	current->name_length = 1;
	current->type = 2;
	current->name[] = ".";

	current->inode = dir_addr;
	current->size = BLOCK_SIZE - 12;
	current->name_length = 1;
	current->type = 2;
	current->name[] = "..";

	//Modifying the count of unallocated data blocks and inodes
	sb_unallocated_count(-1, -1);

	return 0;
}