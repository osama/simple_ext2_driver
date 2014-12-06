simple_ext2_driver
==================
This program was written for an assignment for my course on 
[Operating Systems](http://www.cs.utoronto.ca/~reid/curriculum/csc369.html).
The four programs that the makefile generates are meant to manipulate a provided ext2 filesystem
image as per the following specification. The [OSDev wiki](http://wiki.osdev.org/Ext2) is a great resource for those 
interesting in learning more about the implementation of the ext2 filesystem.

Specifications
--------------
The [makefile](https://github.com/mcoded/simple_ext2_driver/blob/master/Makefile) generates the following programs
(these descriptions are from the original assignment specification). These programs are relatively simplistic
and do not handle cases not mentioned in the given descriptions.

+ **ext2_cp:** This program takes three command line arguments. The first is the name of an ext2 formatted virtual disk. 
The second is the path to a file on your native operating system, and the third is an absolute path on your 
ext2 formatted disk. The program should work like cp, copying the file on your native file system onto the 
specified location on the disk. If the specified file or target location does not exist, then your program should 
return the appropriate error.  
+ **ext2_mkdir:** This program takes two command line arguments. The first is the name of an ext2 formatted virtual disk. 
The second is an absolute path on your ext2 formatted disk. The program should work like mkdir, creating the final 
directory on the specified path on the disk. If location where the final directory is to be created does not exist 
or if the specified directory already exists, then your program should return the appropriate error.  
+ **ext2_ln:** This program takes three command line arguments. The first is the name of an ext2 formatted virtual disk. 
The other two are absolute paths on your ext2 formatted disk. The program should work like ln, creating a link from the 
first specified file to the second specified path. If the first location does not exist, if the second location already 
exists, or if the first location refers to a directory, then your program should return the appropriate error.  
+ **ext2_rm:** This program takes two command line arguments. The first is the name of an ext2 formatted virtual disk, 
and the second is an absolute path to a file or link (not a directory) on that disk. The program should work like rm, 
removing the specified file from the disk. If the file does not exist or if it is a directory, then your program should 
return the appropriate error.
