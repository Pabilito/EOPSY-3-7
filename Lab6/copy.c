#include <sys/types.h> 					//ftruncate()	
#include <sys/stat.h>
#include <sys/mman.h>					//mmap()
#include <fcntl.h>
#include <unistd.h> 					//getopt()

#include <string.h>					//memcpy()
#include <stdio.h>					//for printing
#include <stdlib.h>					//exit()

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)		//Error messages as in linux manual

//PASTE to console before first execution ---> alias copy=./a.out

void copyfile(char* file, char* new_file, int mode){

	FILE *fptr1, *fptr2;						//declare pointers to files

	fptr1 = fopen(file, "r");					//check if file was opened correctly
	if (fptr1 == NULL)
	{
		printf("Cannot open file %s \n", file);			//if not exit
		exit(-1);
	}
  	
	fptr2 = fopen(new_file, "w+");					//wrtie mode with creation of new file if it doesn't exist
	if (fptr2 == NULL)
	{
		printf("Cannot open file %s \n", new_file);
		exit(-2);
	}

	int fd = fileno(fptr1); 						//get file descriptors of our files
	int fd2 = fileno(fptr2);

	//read() and write() version
	if(mode == 1){	
		char buf[20];							//I will read 20 bytes at a time (but could be any other number)
		size_t nbytes = 20;
		ssize_t bytes_read;
		nbytes = sizeof(buf);
		
		bytes_read = read(fd, &buf, 20); 				//at first we read, only then we can write
		while (bytes_read != 0){						
			//HERE INSERT WRITE
			write(fd2, &buf, bytes_read);				//write that many bytes as we read to our copied file
			bytes_read = read(fd, &buf, 20); 			//read 20 bytes to t
	    	}
	}
	//mmap() and memcpy() version
	else{
		char* addr1;
		char* addr2;
		struct stat sb1;						//store information about file here
		if (fstat(fd, &sb1) == -1){        				//obtain file size
               		handle_error("fstat1");
		}
	
		//we need to map source and destination

		//void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
		addr1 = mmap(NULL, sb1.st_size, PROT_READ, MAP_PRIVATE, fd, 0);	// If addr is NULL, then the kernel chooses 
										// the (page-aligned) address at which to create the mapping;
										//st_size = length of all file, PROT_READ - page may be read
										//MAP_PRIVATE - private copy, 0=no offset
		if (addr1 == MAP_FAILED){
			handle_error("mmap 1");					//handle error
		}
	
		ftruncate(fd2, sb1.st_size);					//causes the regular file named by path or referenced by fd2
										//to be truncated to a size of precisely length bytes	
		
		addr2 = mmap(NULL, sb1.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);	//map destination file, this time read and wrote permisions
										//I needed to change to MAP_SHARED
										//MAP_SHARED - updates to the mapping are visible to other processes that map 
										//this file and are carried through to the underlying file
		
		if (addr2 == MAP_FAILED){
			handle_error("mmap 2");					//handle error
		}
		
		//memcpy(dest, src, filesize)
		memcpy(addr2, addr1, sb1.st_size);				//copy contents

		munmap(addr1, sb1.st_size);					//delete mappings, allegedely calls update
		munmap(addr2, sb1.st_size);

	}

	fclose(fptr1);								//at the end close files
	fclose(fptr2);
	printf("Copy done\n");

}

int main(int argc, char *argv[]) {

	int opt, mode = 1;

	//Checking flags
	while ((opt = getopt(argc, argv, ":hm")) != -1) 					//':'as first character means that I will handle errors
	{
	   switch (opt) 
	   {
		    case 'h':
		    	printf("Welcome to microsoft technical support \n");
			printf("Use command below to copy contents of a file \n");
			printf("copy [-m] <file_name> <new_file_name> \n");
		    	exit(1);
		    case 'm':
		    	printf("Option -m detected \n");
			mode = 2;								//different way of copying
		    	break;
		    default:									//error handling
		      	fprintf(stderr, "Please use: 'copy -h' to access help\n");		//when incorrect flag detected
		      	exit(EXIT_FAILURE);
	   }
	}
	
	//Checking if number of arguments related to filenames is 2
	if(argc - optind != 2){
		printf("Incorrect number of arguments! \n");
		printf("Please use: 'copy -h' to access help\n");
		exit(-1);
	}
	
	char* file_name = argv[optind];
	char* new_file_name = argv[optind+1];

	copyfile(file_name, new_file_name, mode);

	return 0;
}
