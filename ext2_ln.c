#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern char *ext2_image;

int main (int argc, char **argv){
	if (argc != 4){	
		printf("usage: ext2_ln virtual_disk path1 path2\n");
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

	create_link(argv[3], index);
	

	return 0;
}

void create_link(char *path, int index){

	int dir_addr, index;
	char *temp = path;
	Inode *dir, *file;

	if ((dir_addr = traverse_path(temp)) == -1){
		fprintf(stderr, "The specified path was not found in %s.\n", argv[1]);
	}

	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - INODE_SIZE];

	if ((index = file_exists(dir, temp)) != -1){
		fprintf(stderr, "The file already exists in %s.\n", argv[1]);
	}

	if ((index = mk_file_entry(dir, temp, (char) 0xA00), index) == -1){
		fprintf(stderr, "Error creating file in %s.\n", argv[1]);
	}

	file = (Inode *) &ext2_image[addr_root + index * INODE_SIZE - INODE_SIZE];
	file->hard_links++;

}