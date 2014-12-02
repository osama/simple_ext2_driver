#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern char *ext2_image;

int main (int argc, char **argv){
	if (argc != 4){	
		printf("usage: ext2_ln virtual_disk path1 path2\n");
		return 1;
	}

	//Read the disk image into memory
	if (read_image(argv[1])){
		return 1;
	}

	//Find the first directory address by traversing the given path
	int dir_addr, index;
	char *temp = argv[2];
	Inode *dir, *file;

	if ((dir_addr = traverse_path(temp)) == -1){
		fprintf(stderr, "The specified path was not found in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Find the inode for the directory in which the file to be linked resides
	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - INODE_SIZE];

	//Check if the file to be linked exists
	if ((index = file_exists(dir, temp)) == -1){
		fprintf(stderr, "The specified file was not found in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	return create_link(argv[3], index);
}

/* This method takes a Linux path and an inode index and creates a
 * hardlink in the given path to the given inode.
 * Return: 0 if successful, 1 if an error was encountered
 */
void create_link(char *path, int index){
	//Finding the directory where the link will be created
	int dir_addr, index;
	char *temp = path;
	Inode *dir, *file;

	if ((dir_addr = traverse_path(temp)) == -1){
		fprintf(stderr, "The specified path was not found in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Setting the directory's inode
	dir = (Inode *) &ext2_image[addr_root + dir_addr * INODE_SIZE - INODE_SIZE];

	//If a file of the same name already exists, we cannot modify its link
	if ((index = file_exists(dir, temp)) != -1){
		fprintf(stderr, "The file already exists in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//If no existing file, we can make a file entry pointing to the given inode
	if ((index = mk_file_entry(dir, temp, (char) 0xA00, index) == -1){
		fprintf(stderr, "Error creating file in %s.\n", argv[1]);
		close_image();
		return 1;
	}

	//Increment the hard link pointer in the inode to indicate multiple links to data
	file = (Inode *) &ext2_image[addr_root + index * INODE_SIZE - INODE_SIZE];
	file->hard_links++;

	return 0;
}