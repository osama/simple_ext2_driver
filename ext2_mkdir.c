#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern char *ext2_image;

int main (int argc, char **argv){
	if (argc != 3){	
		printf("usage: ext2_mkdir virtual_disk path\n");
		return 1;
	}

	if (read_image(argv[1])){
		return 1;
	}

}