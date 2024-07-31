#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include "FAT_operation.h"


//global variable
char fileName[9]="";
char extension[4]="";
int fileType=0;

//function prototypes
int search_procedure(char* file_name);
void toUpperString(char *str);
int diskimagechecker(char *str);
int rootcrawler(char* file_memory, char* file_name, char* extension);
int get_last_dot_index(const char *str);
void filecopier(char* file_memory, char * file_memory2, int cluster_start, char* subname);
void directory_traverser(char* file_memory,int cluster_start, int is_root,char* subname);



//boot sector information
struct boot_sector{

	uint16_t bytes_per_sector; //TOTAL SIZE OF DISK
	
}boot_info;


int main(int argc, char * argv[]){

	if(argc<3){

		printf("Lacking the sufficient number of required arguments\n");
		return 1;

	}
	int input_validation_1=diskimagechecker(argv[1]);

	if(argc>4){
		printf("There are too many arguments!\n");
		return 1;
	
	}

	
	

	int file_descriptor = open(argv[1],O_RDWR);
	//printf("fd: %s", argv[1]);
	if(file_descriptor==-1){

		perror("Failed to open file\n");
		return 1;
	}


	struct stat file_info;
	if(fstat(file_descriptor,&file_info)==-1){

		perror("Failed to get file information\n");
		close(file_descriptor);
		return 1;

	}

	char* file_memory= mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor,0);
	if(file_memory==MAP_FAILED){

		perror("Failed to memory map file\n");
		close(file_descriptor);
		return 1;


	}


	char system_type[9];
	char* fat_type="FAT12   ";
	strncpy(system_type,file_memory+54,8);
	system_type[9]='\0';

	if((input_validation_1==0 && strcmp(system_type,fat_type)!=0) ||strcmp(system_type,fat_type)!=0 ){

		printf("Please ensure that you know how to use the program \n");
		return 1;

	}

	

	boot_info.bytes_per_sector=file_memory[11]+(file_memory[12]<<8);


	char* input_string=argv[2];
	toUpperString(input_string);

	int search_ret_val=search_procedure(input_string);

	if(search_ret_val==0){
		printf("That file doesn't exist in the root directory of the disk!\n");
		return 0;
	}
	int root_ret_val=0;

	if(search_ret_val==1){
	root_ret_val=rootcrawler(file_memory,fileName,extension);

				if(root_ret_val==0){
					printf("The file doesn't exist\n");
					return 0;
				}
				munmap(file_memory,file_info.st_size);

		close(file_descriptor);

	
	}
	else{
		printf("Invalid File input!\n");
		return 0;
	}

	printf("Thank you, everything worked out in the end! \n");
	return 0;
}







int diskimagechecker(char *str){

	char* extension_type= ".ima";
	char* extension_type_upper=".IMA";
	char* extension_type_1=".Ima";
	char* extension_type_2=".IMa";
	char* extension_type_3=".ImA";
	char* extension_type_4=".imA";
	char* extension_type_5=".iMA";
	char* extension_type_6=".iMa";


	char* ret_val_1=strstr(str,extension_type);
	char* ret_val_2=strstr(str,extension_type_upper);
	char* ret_val_3=strstr(str,extension_type);
	char* ret_val_4=strstr(str,extension_type_1);
	char* ret_val_5=strstr(str,extension_type_2);
	char* ret_val_6=strstr(str,extension_type_3);
	char* ret_val_7=strstr(str,extension_type_4);
	char* ret_val_8=strstr(str,extension_type_5);
	char* ret_val_9=strstr(str,extension_type_6);



	//printf("%s %s %s %s %s %s %s %s ",ret_val_1,ret_val_2,ret_val_3,ret_val_4,ret_val_5,ret_val_6,ret_val_7,ret_val_8);
	
	if(ret_val_1==NULL && ret_val_2==NULL && ret_val_3==NULL && ret_val_4==NULL && ret_val_5==NULL && ret_val_6==NULL && ret_val_7==NULL && ret_val_8==NULL && ret_val_9==NULL){
		return 0;
	}
	
	
	

	return 1;
}

int search_procedure(char* file_name){


	char* space_string=" ";
	int i;
	 int index = get_last_dot_index(file_name);
    //printf("index: %d\n",index);


	if (index != -1) {
       
		if(strlen(file_name)>12){
			return 0;
		}
	

	
	for(i=0;i<index;i++){

	
		fileName[i]=file_name[i];

		if(i==8){
			break;
		}
		
	}

	//printf("filename: %s\n",fileName);
	
	if(i<8){
		for(i;i<8;i++){
			fileName[i]=space_string[0];
		}
	}
	//printf("filename: %s.\n",fileName);


	for(i=0;i<3;i++){
		extension[i]=file_name[index+i+1];
	}

	fileType=0; //regular file
	return 1;
	}
	else{

		if(strlen(file_name)>8){
			return 0;
		}


		
		for(i=0;i<strlen(file_name);i++){

		
		fileName[i]=file_name[i];

		if(i==8){
			break;
		}

		

	}

	if(i<8){
			for(i;i<8;i++){
			fileName[i]=space_string[0];
		}
	}

	//printf("filename: %s.",fileName);

	for(i=0;i<3;i++){
		extension[i]=space_string[0];
	}


	//printf("extension: %s \n",extension);
		fileType=1; //directory
	
	return 1;
	}
	


 return 0;
}

int rootcrawler(char* file_memory, char* file_name, char* extension_type){

	file_name=fileName;
	extension_type=extension;
	const char* empty_string="";

	if(strcmp(extension_type,empty_string)==0){
		fileType=1;
	}
	else{
		fileType=0;
	}

	// printf("FILE: %s\n",file_name);
	// printf("%s\n",extension_type);


char* temporary_pointer = file_memory+(boot_info.bytes_per_sector * 19);

while(temporary_pointer[0]!= 0x00){

	char read_file[9];
	char read_attribute[4];

	strncpy(read_file,temporary_pointer,8);
	read_file[8]='\0';

	strncpy(read_attribute,temporary_pointer+8,3);
	read_attribute[3]='\0';

	
	// printf("HELP: %s == %s\n",read_file,fileName);
	// printf("HELP: %s \n",read_attribute);
		
	
		if(strcmp(read_file,file_name)==0 && strcmp(read_attribute,extension_type)==0){


			uint16_t first_cluster = temporary_pointer[26] | (temporary_pointer[27] << 8);

			// printf("temp1: %s",temporary_pointer);
			char* rudimentary_subname="";
			//temporary_pointer
			filecopier(temporary_pointer,file_memory,first_cluster,rudimentary_subname);
			return first_cluster;
		
		}




temporary_pointer+=32;
}	
		

	return 0;
}

void toUpperString(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
}





int get_last_dot_index(const char *str) {
    char *last_dot = strrchr(str, '.');

    if (last_dot == NULL) {
        return -1; // No dot found
    }
	int returnvalue=(int) (last_dot - str);
	//printf("return value: %d\n",returnvalue);
    return returnvalue;
}

void filecopier(char* file_memory, char * file_memory2, int cluster_start, char* subname){

	char* temporary_pointer = file_memory;
	char read_file[9];
	char read_attribute[4];
	uint32_t file_size;
	char concatenated_string[15]="";
	char* empty_space=" ";
	char* dot=".";

	
uint16_t cluster_number = (temporary_pointer[26] | (temporary_pointer[27] << 8));

	

	strncpy(read_file,(char*)(temporary_pointer),8);
	read_file[8]='\0';

	strncpy(read_attribute,(char*)(temporary_pointer+8),3);
	read_attribute[3]='\0';

	int k=0;
	
	for(int i=0;i<8 && read_file[i]!=' ';i++){

		concatenated_string[i]=read_file[i];
		k++;

	}
	
		
	strcat(concatenated_string,dot);
		k++;

	for(int i=0;i<3 && read_attribute[i]!=' ';i++){
		concatenated_string[k]=read_attribute[i];
		k++;
	}


	memcpy(&file_size,(temporary_pointer+28),sizeof(uint32_t));

	//printf("CONCATED: %s\n",concatenated_string);
	

	int dest_file = open(concatenated_string, O_RDWR  | O_CREAT, 0666);
        if (dest_file==-1) {
            perror("Error opening destination file");
			close(dest_file);
            return;
        }

		
		int return_val_lseek = lseek(dest_file, file_size - 1, SEEK_SET);
    	if (return_val_lseek == -1) {
        perror("Error seeking to end of file");
        close(dest_file);
        return;
    	}
    int return_val_write = write(dest_file, "", 1);
    if (return_val_write != 1) {
        perror("Error writing last byte");
        close(dest_file);
        return;
    }

	char* destination_file = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, dest_file, 0);
		if (destination_file == MAP_FAILED) {
			printf("Error: failed to map file memory\n");
			close(dest_file);
			exit(1);
		}



	int current_cluster = cluster_start;
	int current_cluster2=current_cluster;
    int bytes_remaining = file_size;
	int physical_address = boot_info.bytes_per_sector * (31+current_cluster2);

    do{
		current_cluster2=(bytes_remaining==file_size) ? current_cluster : getFatEntry(current_cluster2,file_memory2);
		physical_address = boot_info.bytes_per_sector * (31 + current_cluster2);

		int i;
		for(i=0; i<boot_info.bytes_per_sector;i++){
			if(bytes_remaining==0){
				break;
			}
		
		destination_file[file_size-bytes_remaining]= file_memory2[i+physical_address];
		bytes_remaining--;
		}

	}
	while(getFatEntry(current_cluster,file_memory2)!=0xFFF && bytes_remaining >0);












        if (munmap(destination_file, file_size) == -1) {
            perror("Error unmapping destination file");
			exit(1);

        }

		close(dest_file);



	return;
}
