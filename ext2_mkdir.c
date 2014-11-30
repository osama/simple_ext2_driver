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

	if (read_image(argv[1])){
		return 1;
	}

	int dir_addr, index;
	char *temp = argv[2];
	Inode *dir, *file;

	if ((dir_addr = traverse_path(temp)) == -1){
		fprintf(stderr, "The specified path was not found in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - INODE_SIZE];

	if ((index = file_exists(dir, temp)) != -1){
		fprintf(stderr, "The file already exists in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	if ((index = mk_file_entry(dir, temp, (char) 0x400), -1) == -1){
		fprintf(stderr, "Error creating file in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	file = (Inode *) &ext2_image[addr_root + index * INODE_SIZE - INODE_SIZE];
	//TODO: Write Inode properly
	file->db[0] = find_free_block();

	int i;
	int data_index = BLOCK_SIZE * file->db[0];

	for (i = 0; i < 24; i++){
		ext2_image[data_index + i] = ext2_image[addr_root + i];
	}

	uint32_t *current = &ext2_image[data_index], *parent = &ext2_image[data_index + 12]
	*current = data_index;
	*parent = dir_addr;

	sb_unallocated_count(-1, -1);

	return 0;
}