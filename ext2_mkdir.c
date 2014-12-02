#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern char *ext2_image;
extern int addr_root;

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
	char *temp = argv[2];
	Inode *dir, *file;

	if ((dir_addr = traverse_path(temp)) == -1){
		fprintf(stderr, "The specified path was not found in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Find the inode for the directory where we will be making a new directory
	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - INODE_SIZE];

	//If a file with the directory's name already exists, we cannot overwrite it
	if ((index = file_exists(dir, temp)) != -1){
		fprintf(stderr, "The file already exists in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Making a directory entry as another directory
	if ((index = mk_file_entry(dir, temp, (char) 0x400, -1)) == -1){
		fprintf(stderr, "Error creating file in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Accessing the new directory's inode to add information
	file = (Inode *) &ext2_image[addr_root + index * INODE_SIZE - INODE_SIZE];
	//TODO: Write Inode properly
	file->db[0] = find_free_block();

	//If there are no data blocks available, perform cleanup
	if (file->db[0] == -1){
		fprintf(stderr, "No more space for new file or directory.\n");
		toggle_inode_bitmap(index);
		rm_file_entry(dir, temp);
		return 1;
	}

	int i;
	int data_index = BLOCK_SIZE * file->db[0] - BLOCK_SIZE;

	//Copying over . and .. pointers from root
	for (i = 0; i < 24; i++){
		ext2_image[data_index + i] = ext2_image[addr_root + i];
	}

	//Modifying . and .. pointers to suit the new directory
	uint32_t *current = (uint32_t *) &ext2_image[data_index];
	uint32_t *parent = (uint32_t *) &ext2_image[data_index + 12]
	current[0] = data_index;
	parent[0] = dir_addr;

	//Modifying the count of unallocated data blocks and inodes
	sb_unallocated_count(-1, -1);

	return 0;
}