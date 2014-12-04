#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <string.h>

#include "ext2.h"

const int debug = 12;		//Used to print debug messages

unsigned char *ext2_image;	//Points to the mapped data from the image file
int fd, addr_root = -1;		//File descriptor for file image and the root inode's address
struct stat image;			//Information relating to the provided image file

extern char* finalname;

/* This function is used to read the binary data from a provided
 * file. This data is mapped into memory and read with the use
 * of the global variable ext2_image.
 */
int read_image(char *filename){
	if ((fd = open(filename, O_RDWR)) == -1) {
        perror("Opening image");
        return -1;
    }

    fstat(fd, &image);	//Obtain file information to read filesize

    ext2_image = mmap(NULL, image.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if (ext2_image == MAP_FAILED){
    	perror("Mapping image");
    	return -1;
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
	int index = -1, steps, i, next;
	Inode *walk;
	char *tmp = path;

	//Calculating number of steps needed to reach the required destination
	for (steps=0; tmp[steps]; tmp++){
		if (tmp[steps] == '/')
			steps++;
	}

	if(debug){
		printf("Steps: %d\n", steps);
	}

	//Calculating the byte address of the root inode if it hasn't been found yet
	if (addr_root == -1){
		//The block group descriptor table is always the third data block in the image
		Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*2];
		addr_root = bgr->addr_inode_table * BLOCK_SIZE + INODE_SIZE;	//The root inode is always the second

		if (debug){
			printf("Inode table: %d\n", bgr->addr_inode_table * BLOCK_SIZE);
			printf("Root address: %d\n", addr_root);
		}
	}

	//The path walk starts at the root inode
	walk = (Inode *) &ext2_image[addr_root];

	tmp = path;
	tmp = strtok(path, "/");
  	for (i = 0; i < steps && tmp != NULL && steps > 1; i++){
  		finalname = tmp;

  		if (i == steps - 1){
  			index = next;	//If the file is found, it is set as the index
  			break;
  		}

  		//If the given file exists, we can continue
  		next = file_exists(walk, tmp);

  		if (next == -1){
  			index = next;	//If the file is not found, we return -1
  			break;
  		}

  		if (debug){
  			printf("Current path walk: %s\n", tmp);
  		}

  		//If the given directory is found, its inode is set as the current 
  		walk = (Inode *) &ext2_image[addr_root + next * INODE_SIZE - ROOT_BLOCK * INODE_SIZE];

  		if (debug){
  			printf("Current inode: %d\n", next);
  			printf(" Address: %d\n", addr_root + next * INODE_SIZE - ROOT_BLOCK * INODE_SIZE);
  		}

  		tmp = strtok (NULL, "/");	//Finds the next filename to find
  	}

  	//If there were zero steps to be taken, the root node is the index
  	if (steps == 1){
  		index = ROOT_BLOCK;
  		finalname = tmp;
  	}

  	if (debug){
  		printf("File name: %s\n", finalname);
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
			break;		//If the data block is not being used, nothing was found

		int crossed = 0;
		int dblock = dir->db[i] * BLOCK_SIZE;
	  	Dir_entry *dentry = (Dir_entry *) &ext2_image[dblock];	//Loading current directory
	
		if (debug){
			printf("Searching for file in data block %d.\n", dblock);
		}
		//Search while the data entries are not null and 
		//the end of the data block has not been reached
	  	while (dentry->inode && crossed < BLOCK_SIZE){
	  		if (debug){
	  			printf("Current Dir Entry: %s\n", dentry->name);
	  		}

	  		//If a directory entry matches the provided filename, we can obtain its index
	  		if (!strncmp(dentry->name, filename, dentry->name_length)){
	  			if (debug)
	  				printf("Found!\n");
	  			index = dentry->inode;
	  			break;
	  		}
	  		

	  		if (debug){
	  			printf("Current dir entry size: %d\n", dentry->size);
	  		}
	  		//If nothing has been found, proceed to the next directory entry
	  		crossed += dentry->size;
	  		dentry = (Dir_entry *) ((char *) dentry + dentry->size);
	  	}

	  	//If the index has been set, the loop can exit
	  	if (index != -1)
	  		break;
	  }

  	return index;
}

/* This function takes a directory, a filename, a type, and an inode index
 * and uses this information to create a file entry in the given directory.
 */
int mk_file_entry(Inode *dir, char *filename, char type, int index){
	int i, done = 0, totalcrossed = 0;
	short totalsize = strlen(filename) + 10;	//Size of the new entry
	Dir_entry *dentry, *prev;

	//If there is no existing index to match to, try to find a free inode
	if (index == -1){
	  	index = find_free_inode();

	  	//If we can't find more inodes, there is no need to create the entry
	  	if (index == -1){
	  		return index;
	  	}
	}

	for(i = 0; i < 12 && !done; i++){
		if (!(dir->db[i])){	//If the directory data block is unused, allocate a new one
			dir->db[i] = find_free_block();
			done = 1;		//If we have allocated a free block, no repeat needed

			if (debug){
				printf("Allocating block %d for %d direct block pointer\n", dir->db[i], i);
			}
			
			//If no data block was found, reset address
			if (dir->db[i] == -1){
				dir->db[i] = 0;
				sb_unallocated_count(-1, 0);
			}
		}

		if (debug){
			printf("Current data block: %d\n", dir->db[i]);
		}

		int crossed = 0;
		int dblock = dir->db[i] * BLOCK_SIZE;
	  	dentry = (Dir_entry *) &ext2_image[dblock];
		
		//Keep traversing until an empty space is found in the data block
	  	while (dentry->inode && crossed < BLOCK_SIZE){	
			prev = dentry; 
	  		crossed += dentry->size;

	  		if (debug){
	  			printf("Current Dir Entry: %s\n", dentry->name);
	  		}	

	  		//If there is empty space in the current data block large enough
	  		if (crossed < BLOCK_SIZE - totalsize){
	  			dentry =  (Dir_entry *) ((char *) dentry + dentry->size);
	  		}

	  		//If the previous data block was the last block, add a new entry after it
	  		if (prev->size - prev->name_length - 10 > strlen(filename) + 10){
	  			dentry = (Dir_entry *)((char *)prev + (uint32_t) prev->name_length + 10);
	  			crossed -= prev->size;
	  			crossed += prev->name_length + 10;
	  			done = 1;
	  			break;
	  		}
	  	}

	  	//The size of the data entry 
	  	totalcrossed = BLOCK_SIZE - crossed;

	}

	  //Set values of the new directory entry
	if (index != -1){
	  	prev->size = (char ) (10 + prev->name_length);

	  	dentry->inode = (uint32_t) index;
		dentry->size = (uint16_t) totalcrossed;
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
		if (!dir->db[i])	//If the directory data block is unused, no match
			break;

		//Set the data block containing directory entries
		int crossed = 0;
		int dblock = dir->db[i] * BLOCK_SIZE;
	  	Dir_entry *dentry = (Dir_entry *) &ext2_image[dblock];
	  	Dir_entry *prev;
	
	  	while (dentry->inode && crossed < BLOCK_SIZE){
	  		if (debug){
	  			printf("RM - Current Dir Entry: %s\n", dentry->name);
	  		}

	  		//If the directory entry matches, reset all values
	  		if (!strncmp(dentry->name, filename, dentry->name_length)){
	  			if(debug)
	  				printf("RM - Found!\n");

	  			prev->size += dentry->size;
				dentry->inode = 0;
				dentry->size = 0;
				dentry->name_length = 0;
				dentry->type = 0;
	  			return;
	  		}

	  		prev = dentry;
	  		//Move onto the next entry in the data block
	  		crossed += dentry->size;
	  		dentry = (Dir_entry *) ((char *) dentry + dentry->size);
	  	}
	  }
}

/* This function modifies the counts for unallocated blocks and
 * inodes in the superblock and the block descriptor table.
 */ 
void sb_unallocated_count(int block_change, int inode_change){
	//Find the superblock and the block group descriptor table
	Superblock *sb = (Superblock *) &ext2_image[BLOCK_SIZE];
	Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*2];

	//Modify unallocated block and inode values as specified
	sb->unallocated_blocks += block_change;
	sb->unallocated_inodes += inode_change;

	bgr->unallocated_blocks += block_change;
	bgr->unallocated_inodes += inode_change;
}

/* This function checks for an unset bit in the free block bitmap
 * and returns the appropriate block index.
 */
int find_free_block(){
	Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*2];
	int bitmap_addr = BLOCK_SIZE * bgr->addr_block_usage;
	int free_index = -1, c, d, found = 0;

	if (bgr->unallocated_blocks <= 0){
		return -1;
	}

	//Go through all of the bytes in the bitmap
	for (c = 0; c < 16 && !found; c++){
		char test = ext2_image[bitmap_addr + c];

		//Go through each bit in the byte
		for (d = 0; d < 8 && !found; d++){
			//Check value of bit and trigger if it is unset
			if (!(test & (1 << d))){
				free_index = 8 - d + c * 8;
				found = 1;
			}
		}
	}

	toggle_data_bitmap(free_index);	//Toggle the bit to indicate that it's taken
	return free_index;
}

/* This function checks for an unset bit in the free inode bitmap
 * and returns the appropriate block index.
 */
int find_free_inode(){
	Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*2];
	int bitmap_addr = BLOCK_SIZE * bgr->addr_inode_usage;
	int free_index = -1, c, d, found = 0;

	if (bgr->unallocated_inodes <= 0){
		return -1;
	}

	//Go through all of the bytes in the bitmap
	for (c = 0; c < 2 && !found; c++){
		char test = ext2_image[bitmap_addr + c];

		//Go through each bit in the byte
		for (d = 0; d < 8 && !found; d++){
			//Check value of bit and trigger if it is unset
			if (!(test & (1 << d))){
				free_index = 8 - d + c * 8;
				found = 1;
			}
		}
	}

	toggle_inode_bitmap(free_index); //Toggle the bit to indicate that it's taken
	return free_index;
}

/* This function toggles the given bit in the free block bitmap.
 */
void toggle_data_bitmap(int index){
	Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*2];
	int bitmap_addr = BLOCK_SIZE * bgr->addr_block_usage;

	//C represents the byte to check and d represents the bit
	int c = index/8, d = 8 - index + c * 8;

	ext2_image[bitmap_addr + c]	^= 1 << d;
}

/* This function toggles the given bit in the free inode bitmap.
 */
void toggle_inode_bitmap(int index){
	Block_group *bgr = (Block_group *) &ext2_image[BLOCK_SIZE*2];
	int bitmap_addr = BLOCK_SIZE * bgr->addr_inode_usage;

	//C represents the byte to check and d represents the bit
	int c = index/8, d = 8 - index + c * 8;

	if(debug){
		printf("Toggling inode bit %d\n", d);
	}

	ext2_image[bitmap_addr + c]	^= 1 << d;
}