#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <string.h>

#include "ext2.h"


unsigned char *ext2_image;	//Points to the mapped data from the image file
int fd, addr_root = -1;		//File descriptor for file image and the root inode's address
struct stat image;			//Information relating to the provided image file

/* This function is used to read the binary data from a provided
 * file. This data is mapped into memory and read with the use
 * of the global variable ext2_image.
 */
int read_image(char *filename){
	if ((fd = open(filename, O_RDWR)) == -1) {
        perror("Opening image");
        return 1;
    }

    fstat(fd, &image);	//Obtain file information to read filesize

    ext2_image = mmap(NULL, image.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)

    if (ext_image = MAP_FAILED){
    	perror("Mapping image");
    	return 1;
    }

    return 0;
}

/* This function closes the memory mapping, thus saving all modified information.
 */
void close_image(){
	if (munmap(ext2_image, image.st_size)){
		perror("Updating image");
	}

	if (close(fd)){
		perror("Closing image");
	}
}

/* This function takes a Linux filepath and uses it to traverse the specified image.*/
int traverse_path(char *path){
	int index = -1, taken = 0, steps, i, lookup = root;
	Inode *walk;
	char *buffer = NULL, *tmp = path;

	for (steps=0; tmp[steps]; s[steps]=='/' ? steps++ : *tmp++);
	
	if (addr_root == -1){
		Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*ROOT_BLOCK];
		addr_root = bgr->addr_inode_table + INODE_SIZE;
	}

	walk = (Inode *) &ext2_image[addr_root];

	int next;

	tmp = path
	buffer = strtok (tmp, "/");
  	for (i = 0; i < steps; i++){
  		buffer = strtok (NULL, "/");

  		int next = file_exists(walk, buffer);

  		if (next == -1 || i == steps - 1){
  			index = next;
  			break;
  		}

  		walk = (Inode *) &ext2_image[addr_root + next * INODE_SIZE - INODE_SIZE];

  	}

	return index;
}

/* Given a filename and a directory, this function checks if the file already
 * exists in the provided directory.
 */
int file_exists(Inode *dir, char *filename){
	int index = -1, i;

	for(i = 0; i < 12; i++){
		if (!dir->db[i])
			break;

		int crossed = 0;
		int dblock = dir->db[i] * BLOCK_SIZE;
	  	Dir_entry *dentry = (Dir_entry *) &ext2_image[dblock];
	
	  	while (*dentry && crossed < BLOCK_SIZE){
	  		if (!strncmp(&dentry->name, filename, dentry->name_length)){
	  			index = dentry->inode;
	  			break;
	  		}
	  		
	  		crossed += dentry->size;
	  		dentry += dentry->size;
	  	}

	  	if (index != -1)
	  		break;
	  }

  	return index;
}

/* This function takes a directory, a filename, a type, and an inode index
 * and uses this information to create a file entry in the given directory.
 */
int mk_file_entry(Inode *dir, char *filename, char type, int index){
	int i;
	Dir_entry *dentry = -1;

	if (index == -1){
	  	index = find_free_inode();

	  	if (index == -1){
	  		return index;
	  	}
	}

	for(i = 0; i < 12; i++){
		if (!dir->db[i])
			break;

		int crossed = 0;
		int dblock = dir->db[i] * BLOCK_SIZE;
	  	dentry = (Dir_entry *) &ext2_image[dblock];
	
	  	while (*dentry && crossed < BLOCK_SIZE){	  		
	  		crossed += dentry->size;
	  		dentry += dentry->size;
	  	}

	  }

	  if (index != -1){
	  	dentry->inode = (uint32_t) index;
		dentry->size = (uint16_t) strlen(filename) + 10;
		dentry->name_length = (char) strlen(filename);
		dentry->type = type;
		strncpy(dentry->name, filename, (int) dentry->name_length + 1);
	  }

  	return index;
}

/* This function removes the given filename's entry from the given directory.
 */
void rm_file_entry(Inode *dir, char *filename){
	int i = 0;

	for(i = 0; i < 12; i++){
		if (!dir->db[i])
			break;

		int crossed = 0;
		int dblock = dir->db[i] * BLOCK_SIZE;
	  	Dir_entry *dentry = (Dir_entry *) &ext2_image[dblock];
	
	  	while (*dentry && crossed < BLOCK_SIZE){
	  		if (!strncmp(&dentry->name, filename, dentry->name_length)){
				dentry->inode = 0;
				dentry->size = 0;
				dentry->name_length = 0;
				dentry->type = 0;
				dentry->name = 0;
	  			return;
	  		}
	  		
	  		crossed += dentry->size;
	  		dentry += dentry->size;
	  	}
	  }
}

/* This function modifies the counts for unallocated blocks and
 * inodes in the superblock and the block descriptor table.
 */ 
void sb_unallocated_count(int block_change, int inode_change){
	Superblock *sb = (Superblock *) &ext2_image[BLOCK_SIZE];
	Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*2];

	sb->unallocated_blocks += block_change;
	sb->unallocated_inodes += inode_change;

	bgr->unallocated_blocks += block_change;
	bgr->unallocated_inodes += inode_change;
}

/* This function checks for an unset bit in the free block bitmap
 * and returns the appropriate block index.
 */
int find_free_block(){
	Block_group *bgr = &ext2_image[BLOCK_SIZE*2];
	int bitmap_addr = BLOCK_SIZE * bgr->addr_block_usage;
	int free_index == -1, c, d;

	if (bgr->unallocated_blocks <= 0){
		return -1;
	}

	for (int c = 0; c < 16 && !found; c++){
		char test = ext2_image[bitmap_addr + c];
		int found = 0;

		for (int d = 0; d < 8 && !found; d++){
			if (!(test & (1 << d))){
				free_index = 8 - d + c * 8;
			}
		}
	}

	toggle_data_bitmap(free_index);
	return free_index;
}

/* This function checks for an unset bit in the free inode bitmap
 * and returns the appropriate block index.
 */
int find_free_inode(){
	Block_group *bgr = &ext2_image[BLOCK_SIZE*2];
	int bitmap_addr = BLOCK_SIZE * bgr->addr_inode_usage;
	int free_index == -1, c, d;

	if (bgr->unallocated_inodes <= 0){
		return -1;
	}

	for (int c = 0; c < 2 && !found; c++){
		char test = ext2_image[bitmap_addr + c];
		int found = 0;

		for (int d = 0; d < 8 && !found; d++){
			if (!(test & (1 << d))){
				free_index = 8 - d + c * 8;
			}
		}
	}

	toggle_inode_bitmap(free_index);
	return free_index;
}

/* This function toggles the given bit in the free block bitmap.
 */
void toggle_data_bitmap(int index){
	Block_group *bgr = &ext2_image[BLOCK_SIZE*2];
	int bitmap_addr = BLOCK_SIZE * bgr->addr_block_usage;

	int c = index/8, d = 8 - index + c * 8;

	ext2_image[bitmap_addr + c]	^= 1 << d;
}

/* This function toggles the given bit in the free inode bitmap.
 */
void toggle_inode_bitmap(int index){
	Block_group *bgr = &ext2_image[BLOCK_SIZE*2];
	int bitmap_addr = BLOCK_SIZE * bgr->addr_inode_usage;

	int c = index/8, d = 8 - index + c * 8;

	ext2_image[bitmap_addr + c]	^= 1 << d;
}