#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "FAT_operation.h"


//directives
#define timeOffset 14 //offset of creation time in directory entry
#define dateOffset 16 //offset of creation date in directory entry


//function prototypes
void directory_traverser(char* file_memory,int cluster_start, int is_root,char* subname);
void print_date_time(char * directory_entry_startPos);
int diskimagechecker(char *str);


//boot sector information
struct boot_sector{
	
	char OS_NAME[9]; //OS_NAME
	uint16_t bytes_per_sector; //TOTAL SIZE OF DISK
	uint8_t num_of_FAT;		//Number of FAT copies
	uint16_t total_sector_count;  //TOTAL SIZE OF DISK
	uint16_t sectors_per_FAT;  //Sectors per FAT
	char volume_label[12]; //LABEL OF DISK


}boot_info;


int main(int argc, char * argv[]){

	if(argc<2){

		printf("No file was inserted!\n");
		return 1;

	}
	int input_validation_1=diskimagechecker(argv[1]);
	
	
	if(input_validation_1==0){
		printf("The argument is not a disk image file!\n");
		return 1;

	}
	else if(argc>2){

		printf("Please ensure that you know how to use the program properly (Overloaded number of arguments)\n");
		return 1;

	}

	int file_descriptor = open(argv[1],O_RDWR);
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
	boot_info.total_sector_count=file_memory[19]+(file_memory[20]<<8);	



	directory_traverser(file_memory,0,1,"");












	return 0;
}



void directory_traverser(char* file_memory,int cluster_start, int is_root, char* subname){


char* temporary_pointer;

if (is_root) {
        temporary_pointer = file_memory+(boot_info.bytes_per_sector * 19);  // Root directory starts at sector 19
		

			printf("\n\\\n");
			printf("==================\n");


    } else {
        temporary_pointer = file_memory + (boot_info.bytes_per_sector*33)+ (cluster_start - 2) * boot_info.bytes_per_sector;
	

			printf("\n%s\n",subname);
			printf("==================\n");
	
	}

	while(temporary_pointer[0]!= 0x00){

		if(((temporary_pointer[26]|= temporary_pointer[27]<<8 )&0xff)!=0 && ((temporary_pointer[26]|= temporary_pointer[27]<<8 )&0xff)!=1 && (temporary_pointer[11]&0x10)!=0x10){


	char read_file[9];
	uint32_t file_size;
	

	strncpy(read_file,(char*)(temporary_pointer),8);
	read_file[8]='\0';

	memcpy(&file_size,(temporary_pointer+28),sizeof(uint32_t));

	printf("F %10d %20s ",file_size,read_file);
	print_date_time(temporary_pointer);





	}
	else if((temporary_pointer[11]&0x10)==0x10) {

		    if (temporary_pointer[0] != '.' && temporary_pointer[1]!='.') {

				char read_file[9];
				uint32_t file_size;
				

				strncpy(read_file,(char*)(temporary_pointer),8);
				read_file[8]='\0';

				memcpy(&file_size,(temporary_pointer+28),sizeof(uint32_t));
				

				printf("D %10d %20s ",file_size,read_file);
				print_date_time(temporary_pointer);

					
			}

}
temporary_pointer+=32;
}


	temporary_pointer = is_root ? (file_memory + (boot_info.bytes_per_sector * 19)) : (file_memory + (boot_info.bytes_per_sector * 33) + (cluster_start - 2) * boot_info.bytes_per_sector);
	
	
	
	
	while(temporary_pointer[0]!=0x00){

			if((temporary_pointer[26]&0xf)!=0 && (temporary_pointer[26]&0xf)!=1 && (temporary_pointer[11]&0x10)!=0x10){
			}
		else if((temporary_pointer[11]&0x10)==0x10) {

		    if (temporary_pointer[0] != '.' && temporary_pointer[1]!='.') {
                uint16_t first_cluster = temporary_pointer[26] | (temporary_pointer[27] << 8);

				char read_file[9];
				uint32_t file_size;
				

				strncpy(read_file,(char*)(temporary_pointer),8);
				read_file[8]='\0';

				memcpy(&file_size,(temporary_pointer+28),sizeof(uint32_t));
				


				if(first_cluster != 0 || first_cluster != 1){
                directory_traverser(file_memory, first_cluster, 0,read_file);  // Recurse into subdirectory
            }
			}
	

}

temporary_pointer+=32;
}


	return;
}







void print_date_time(char * directory_entry_startPos){
	
	int time, date;
	int hours, minutes, day, month, year;
	
	time = *(unsigned short *)(directory_entry_startPos + timeOffset);
	date = *(unsigned short *)(directory_entry_startPos + dateOffset);
	
	//the year is stored as a value since 1980
	//the year is stored in the high seven bits
	year = ((date & 0xFE00) >> 9) + 1980;
	//the month is stored in the middle four bits
	month = (date & 0x1E0) >> 5;
	//the day is stored in the low five bits
	day = (date & 0x1F);
	
	printf("%d-%02d-%02d ", year, month, day);
	//the hours are stored in the high five bits
	hours = (time & 0xF800) >> 11;
	//the minutes are stored in the middle 6 bits
	minutes = (time & 0x7E0) >> 5;
	
	printf("%02d:%02d\n", hours, minutes);
	
	return;	
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
