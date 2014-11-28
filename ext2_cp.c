#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern char *ext2_image;

int main (int argc, char **argv){
	if (argc != 4){	//Checking if an incorrect number of arguments have been provided.
		printf("usage: ext2_cp virtual_disk local_path absolute_path\n");
		return 1;
	}

	if (read_image(argv[1])){
		return 1;
	}


}