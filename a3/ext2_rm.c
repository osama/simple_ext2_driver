#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ext2.h"

extern char *ext2_image;
extern int addr_root;

int main (int argc, char **argv){
	if (argc != 3){	
		printf("usage: ext2_rm virtual_disk path\n");
		return 1;
	}

	//Read the disk image into memory
	if (read_image(argv[1])){
		return 1;
	}

	//Find the directory address by traversing the given path
	int dir_addr, index;
	char *temp = argv[2];
	Inode *dir, *file;

	if ((dir_addr = traverse_path(temp)) == -1){
		fprintf(stderr, "The specified path was not found in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Set the directory inode
	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - ROOT_BLOCK * INODE_SIZE];

	//Check if the file exists in the given directory
	if ((index = file_exists(dir, temp)) == -1){
		fprintf(stderr, "The specified file was not found in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Set file's inode
	file = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - ROOT_BLOCK * INODE_SIZE];
	rm_file_entry(dir, temp);	//Remove file entry from the directory
	file->hard_links--;			//Decrement the number of links to file's inode

	//If the number of links isn't 0, we can't remove the data
	if (file->hard_links > 0)
		return 0;

	//If there are no further links to the file, time to deallocate it's data
	int i, count = 0;

	//Unallocate all set direct block pointers
	for (i = 0; i < 12; i++){
		if (!file->db[i]){
			toggle_data_bitmap(file->db[i]);
			count++;
		}
	}

	//If the file is using its singly indirect pointer, deallocate additional data
	if (file->db_singly){
		uint32_t *data = (uint32_t *) &ext2_image[BLOCK_SIZE * file->db_singly];
		
		for (i = 0; i < 256; i++){
			if (data[i]){
				toggle_data_bitmap(data[i]);
				count++;
			} else{
				break;
			}
		}
		//Deallocate the data block holding the singly indirect
		toggle_data_bitmap(file->db_singly);
	}

	//Deallocate the file's inode
	toggle_inode_bitmap(index);

	//Modify hte number of unallocated data blocks and inodes
	sb_unallocated_count(count, 1);

	close_image();	//Close the disk image file, saving all changes

	return 0;
}