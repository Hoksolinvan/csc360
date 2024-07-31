#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include "FAT_operation.h"

//global variable
int file_existence=0;
int free_disk_size=0;
char* final_read_file_name;

//function prototype
int diskimagechecker(char *str);
void toUpperString(char *str);
char* fileparser(char* file_name);
int get_last_dot_index(const char *str);
int get_last_slash_index(const char *str);
void directory_traverser(char* file_memory,int cluster_start, int is_root, char* full_path);
int filemaker(char* temporary_pointer, char* original_file_memory, char* token);
int Free_disk_size(char *file_memory);
int getNextEmptyFatIndex(char *file_memory);
void FatEntryUpdate(int count, char* file_memory, int insert_value);
void fileSignature(char* file_name, int fileSize, int first_cluster, char* file_memory);




//boot sector information
struct boot_sector{

	uint16_t bytes_per_sector; //TOTAL SIZE OF DISK
	uint16_t sectors_per_FAT;  //Sectors per FAT
	uint16_t total_sector_count;

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
	boot_info.sectors_per_FAT=file_memory[22]+(file_memory[23]<<8);
	boot_info.total_sector_count=file_memory[19]+(file_memory[20]<<8);

	char* input_string=argv[2];



	char* read_file_name=fileparser(input_string);
	final_read_file_name=read_file_name;
	int real_file=open(read_file_name,O_RDONLY);

	if(real_file==-1){
		printf("The file that you are trying to insert does not exists!\n");
		close(real_file);
		return 1;
	}





	file_existence=1;

	toUpperString(input_string);



	directory_traverser(file_memory,0,1,input_string);





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



void toUpperString(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
}





char* fileparser(char* file_name){

	char *placeholder_string;
	int get_last_slash=get_last_slash_index(file_name);

	if(get_last_slash!=-1){
		placeholder_string = malloc(strlen(file_name) - get_last_slash);

        if (placeholder_string == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return NULL;
        }
        

		strcpy(placeholder_string,file_name + get_last_slash +1);
		return placeholder_string;
	}
	
		return file_name;
	

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



int get_last_slash_index(const char *str) {
    char *last_dot = strrchr(str, '/');

    if (last_dot == NULL) {
        return -1; // No dot found
    }
	int returnvalue=(int) (last_dot - str);
	//printf("return value: %d\n",returnvalue);
    return returnvalue;
}




void directory_traverser(char* file_memory,int cluster_start, int is_root, char* file_name) {
	

	char* temporary_pointer;

	
	char* input_string=malloc(strlen(file_name)+1);
	if(input_string==NULL){
		printf("failed memory allocation! \n");
		return;
	}
	strcpy(input_string,file_name);

	char* token;
	const char delimiter[2]="/";
	token=strtok(input_string,delimiter);

	

	if(token==NULL){
		printf("Invalid filepath\n");
		free(input_string);
		return;
		
	}

		
	
	if(strcmp(token,".")==0){
				

		char* output_string = strchr(input_string,'/');

		if(output_string!=NULL){
		output_string++;
		directory_traverser(file_memory,cluster_start,is_root,output_string);
		}
		free(input_string);
		return;
	}



	temporary_pointer = is_root ? (file_memory + (boot_info.bytes_per_sector * 19)) : (file_memory + (boot_info.bytes_per_sector * 33) + (cluster_start - 2) * boot_info.bytes_per_sector);
	int bit_set=0;
		

						
		printf("od");


	while (temporary_pointer[0] != 0x00) {
				printf("God");

    if ((temporary_pointer[26] & 0xf) != 0 && (temporary_pointer[26] & 0xf) != 1 && (temporary_pointer[11] & 0x10) != 0x10) {
        // Not a directory
		
		char read_file[9];

        strncpy(read_file, temporary_pointer,8);
			read_file[8]='\0';



		if(strcmp(read_file,token)==0){
			printf("The file with the exact name already exists! Therefore, we can't do anything about it!\n");
					free(input_string);
			return;
		}
		else{
			bit_set=1;
			break;
		}


    } else if ((temporary_pointer[11] & 0x10) == 0x10) {
        // Is a directory
        if (temporary_pointer[0] != '.' && temporary_pointer[1] != '.') {
            uint16_t first_cluster = temporary_pointer[26] | (temporary_pointer[27] << 8);

            char read_file[9];
            strncpy(read_file, (char*)(temporary_pointer), 8);
            read_file[8] = '\0';

            if (strcmp(read_file, token) == 0) {
                if (first_cluster != 0 && first_cluster != 1) {
                    char* next_token = strtok(NULL, delimiter);
                    if (next_token == NULL) {
                        bit_set = 1;
						break;
                    } else {
                        directory_traverser(file_memory, first_cluster, 0, next_token); 
						break; // Recurse into subdirectory
                    }
                }
            }
        }
    }
    temporary_pointer += 32;
}


		if(file_existence==1 && bit_set==1){
		temporary_pointer = file_memory+(boot_info.bytes_per_sector*33)+ (cluster_start - 2) * boot_info.bytes_per_sector;
		int val=filemaker(temporary_pointer,file_memory,token);


		if(val==1){
			printf("There was an error in inserting the file into the disk image\n");
		}
		}
		else{
		printf("The directory could not be located\n");
		}
				free(input_string);

}




int filemaker(char* temporary_pointer, char* original_file_memory, char* token){
	

	char* file_name=token;

	int file_descriptor = open(file_name,O_RDWR);
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
	int file_size=file_info.st_size;

	char* file_memory= mmap(NULL, file_info.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor,0);
	if(file_memory==MAP_FAILED){

		perror("Failed to memory map file\n");
		close(file_descriptor);
		return 1;


	}
	int free_size_left= Free_disk_size(original_file_memory);
	if(file_size<=free_size_left){
		int bytes_left=file_size;
		int current = getNextEmptyFatIndex(original_file_memory);

		fileSignature(token,file_size,current,original_file_memory);

		while(bytes_left > 0){
			
						int physicaladdress = boot_info.bytes_per_sector * (31+current);

			int i;

			for(i=0; i< boot_info.bytes_per_sector;i++){
				if(bytes_left==0){
					FatEntryUpdate(current,original_file_memory,0xFFF);
					return 1;
				}
					original_file_memory[i+physicaladdress]=file_memory[file_size-bytes_left];
			bytes_left--;




			}

			FatEntryUpdate(current,original_file_memory,0x69);
			int next=getNextEmptyFatIndex(original_file_memory);
			FatEntryUpdate(current,original_file_memory,next);
			current=next;

		}
		





	}
	else{
		printf("There isn't enough space in the disk!\n");
		if(munmap(file_memory,file_size)==-1){
			perror("The file was not properly unmapped!\n");
			exit(1);
		}

		close(file_descriptor);
		return 1;
	}


}




int getNextEmptyFatIndex(char *file_memory){
	file_memory+=boot_info.bytes_per_sector;

	int count=2;

	while(getFatEntry(2,file_memory)!=0x000){
		count++;
	}
	return count;
}





int Free_disk_size(char* file_memory){

	//count variables
	int count=0;
	int fat_entries = ((8*boot_info.sectors_per_FAT*boot_info.bytes_per_sector)/12);
	

	for(int i=2; i<fat_entries;i++){
		if((getFatEntry(i,file_memory)==0x000) && ((i+31)<boot_info.total_sector_count)){
			count++;
		}
	
	}

	return boot_info.bytes_per_sector*count;
}






void FatEntryUpdate(int count, char* file_memory, int insert_value){
	file_memory+=boot_info.bytes_per_sector;

	if((count%2)==0){
		file_memory[boot_info.bytes_per_sector + ((3*count)/2)+1] = (insert_value >> 8) & 0x0F;
		file_memory[boot_info.bytes_per_sector + ((3*count)/2)] = insert_value & 0x0F;
	}
	else{
		file_memory[boot_info.bytes_per_sector + (int)((3*count)/2)] = (insert_value << 4) & 0xF0;
		file_memory[boot_info.bytes_per_sector + (int)((3*count)/2)+1] = (insert_value >> 4) & 0xFF;

	}




}


void fileSignature(char* file_name, int fileSize, int first_cluster, char* file_memory){

	file_memory+= boot_info.bytes_per_sector * 19;

	while(file_memory[0]!=0x00){
		file_memory+=32;
	}

	int index;
	int dot_location= -1;

	for(index=0;index<8;index++){
		char charac=file_name[index];
		if(charac=='.'){
			dot_location=index;
		}
		file_memory[index]= (dot_location == -1) ? charac : ' ';
	}

	for(index=0; index<3;index++){
		file_memory[index+8]=file_memory[index+dot_location+1];
	}



	file_memory[11]=0x00;



	time_t current_time= time(NULL);
	struct tm *current = localtime(&current_time);
	int year = current->tm_year+1980;
	int month = (current->tm_mon+1);
	int day = current->tm_mday;
	int hour = current->tm_hour;
	int minute = current->tm_min;

	file_memory[14]=0;
	file_memory[15]=0;
	file_memory[16]=0;
	file_memory[17]=0;

	file_memory[17] |= (year - 1980) <<1;
	file_memory[17] |= (month - ((file_memory[16] & 0b11100000)>>5))>>3;
	file_memory[16] |= (month - ((file_memory[17] & 0b00000001)<<3))<<5;
	file_memory[16] |= (day & 0b00011111);
	file_memory[15] |= (hour << 3) & 0b11111000;
	file_memory[15] |= (minute- ((file_memory[14] & 0b11100000) >> 5)) >>3;
	file_memory[14] |= (minute- ((file_memory[15] & 0b00000111)<<3)) <<5;


	file_memory[26]= (first_cluster - (file_memory[27]<<8)) & 0xFF;
	file_memory[27]= (first_cluster - file_memory[26]) >>8;


	file_memory[28] = (fileSize & 0x000000FF);
	file_memory[29] = (fileSize & 0x0000FF00) >> 8;
	file_memory[30] = (fileSize & 0x00FF0000) >> 16;
	file_memory[31] = (fileSize & 0xFF000000) >> 24;



}
