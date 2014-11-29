#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern char *ext2_image;
extern int addr_root;

int main (int argc, char **argv){
	if (argc != 3){	
		printf("usage: ext2_rm virtual_disk path\n");
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
	}

	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - INODE_SIZE];

	if ((index = file_exists(dir, temp)) == -1){
		fprintf(stderr, "The specified file was not found in %s.\n", argv[1]);
	}

	file = (Inode *) &ext2_image[addr_root + index * INODE_SIZE - INODE_SIZE];
	rm_file_entry(dir, temp);
	file->hard_links--;

	if (file->hard_links > 0)
		return 0;

	int i, count = 0;

	for (i = 0; i < 12; i++){
		if (!db[i]){
			toggle_data_bitmap(db[i]);
			count++;
		}
	}

	toggle_inode_bitmap(index);

	sb_unallocated_count(count, 1);

	close_image();

	return 0;
}