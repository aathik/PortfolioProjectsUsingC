/*
  Assignment-1

  Aathik Thayyil Radhakrishnan, Section 3, 110094762

  How to Run the program 
  1. gcc -o ncpmvdir ncpmvdir.c
  2. ./ncpmvdir ./source ./destination <option> <extension_list>

*/

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

//Variables
char *extensions[6];
int extensions_count;
char destination[100];
char source[100];
int move_or_copy;

//Function to create destination folders
int createDestination(char *destination){
	char dest[100];
	char *token;
	char actualPath[100];
	int k =0;
	char *path[10];
	strcpy(dest,destination);

	//Separate the path using delimiter '/'
	token = strtok(dest, "/");
	while(token!=NULL){
		path[k]=token;
		k++;
		token = strtok(NULL, "/");
	}

	//Check and create directories if it does not exist
	for (int i = 0; i < k; i++) {
		if(i==0){
			strcpy(actualPath,path[i]);
		}
		else{
			strcat(actualPath,path[i]);
		}
		strcat(actualPath, "/");
			if (access(actualPath, F_OK) != -1) {
				//printf("File Exist\n");	
			}
			else{
				mkdir(actualPath, 0777);
				//printf("New File Created\n");
			}
	}

	return 0;

} //end createDestination


//Function to copy files from source and write them to destination
void copy_files_to_destination(const char *src_file, const char *dest_file)
{
    FILE *fp_s, *fp_d;
	char c;

	//Get the current working directory and concatenate with dest to get the absolute path
	char cwd[256];
	getcwd(cwd, sizeof(cwd));
  	strcat(cwd,dest_file);

	//open the source file in read mode
	fp_s = fopen(src_file, "r");
	if (fp_s == NULL) {
		//Print if error and exit
		perror("Error opening source file");
		exit(1);
	}

	//open the destination file in write mode
	fp_d = fopen(cwd, "w");
	if (fp_d == NULL) {
		//Print if error and exit
		perror("Error opening destination file");
		exit(1);
	}

	//write the contents of the file to the destination
	while ((c = fgetc(fp_s)) != EOF)
		fputc(c, fp_d);

	//close the file pointers
	fclose(fp_s);
	fclose(fp_d);

} // end copy_files_to_destination


//Callback function for nftw - Copy and Move function
int callback(const char *file_path, const struct stat *file_stat, int flags, struct FTW *ftw_buf)
{
	char *file_name;
	char *file_ext;
	int i;
    char dest[100];
    char directory[100];
	bool file_exist=false;

    //To Find the filename from the file path
    file_name = strrchr(file_path, '/');
	file_name++;

	//Check if it is a directory
	if (flags == FTW_D || flags==FTW_DP) {
		strcpy(directory,destination);
		strcat(directory, strrchr(source,'/'));
		strcat(directory,"/");
		strncat(directory, file_path+strlen(source), strlen(file_path));  //create the path for that directory
		mkdir(directory+1,0777);	//directory is created
	} 

	//If its a FILE (not a directory)
	else{
		file_ext = strrchr(file_name, '.');
    	if(file_ext != NULL) {
    		file_ext = file_ext + 1;
			//if the extension list is empty
            if(extensions_count==0){
				strcpy(dest, destination);
				strcat(dest, strrchr(source,'/'));
				strcat(dest,"/");
				strcat(dest, file_path + strlen(source)); //source path of the file is created
				copy_files_to_destination(file_path, dest); //function to copy files to destination folder
            }
			//if extension list have file extensions that needs to be excluded
			 else{
    		 	for (i = 0; i < extensions_count; i++) {
    				//check if extension is in exclusion list
					if (strcmp(file_ext, extensions[i]) == 0) {
						file_exist=true;	//file exists in exclusion list
						break;
					}
            	 }

			 	//if file not in extension list - copy/move them
			 	if(!file_exist){
					strcpy(dest, "");
                    strcpy(dest, destination);
                    strcat(dest, strrchr(source,'/'));
					strcat(dest,"/");
                    strcat(dest, file_path + strlen(source)); //source path for the file not in the exclusion list is created
                    copy_files_to_destination(file_path, dest); //function to copy files to destination folder
			 	}
    		}
        }
	}
    return 0;
} // end callback fn



//callback function for nftw to remove the files
int remove_callback(const char *file_path, const struct stat *file_stat, int flags, struct FTW *ftw_buf)
{
    remove(file_path); //remove function removes/deletes the file in its argument
    return 0;
} //end remove_callback fn


//Function to execute the ncpmvdir command
int ncpmvdir(char *source, char *destination, int move_or_copy, char *extensions[], int extensions_count){
	//create destination directory
	createDestination(destination);

	//call nftw function to move/copy the files
	int res = nftw(source+1, callback, 10, FTW_PHYS); 
	// nftw(file_path, callback fn, max_fds(max no of fd's opened while traversing), FTW_PHY (does not follow symbolic links))
	if (res !=0)
	{
		printf("Error while traversing the tree\n");
	}
	
	//check if the operation is move
	if (move_or_copy==1) {
        int st = nftw(source+1, remove_callback, 10, FTW_DEPTH); //FTW_DEPTH:-Contents of directory are visited before directory
		if (st != 0)
		{
			printf("Error while removing files\n");
		}	
    }
	 return res;
} //end ncpmvdir fn


//main function
int main(int argc, char *argv[]){
	
	strcpy(source, argv[1]+1); 				//source folder
    strcpy(destination, argv[2]+1); 		// destination folder

	if(strcmp(argv[3],"-cp")==0){
		move_or_copy=0;						// if cp, move_or_copy=0
	}else if(strcmp(argv[3],"-mv")==0){
		move_or_copy=1;						// if mv, move_or_copy=1
	}else{
		printf("Invalid Option. Try again!!\n");
		exit(1);
	}
	
	char *token;
	if(argc==5){  //if extension list is present, extract the extension list with ',' as the delimiter
        token = strtok(argv[4], ",");
		extensions_count=0;
		while(token!=NULL && extensions_count<6){
			extensions[extensions_count++]=token;
			token = strtok(NULL, ",");
		}
    }else if(argc==4){
        extensions_count = 0;			//extension list is empty
    }else{
		printf("Invalid format.Follow the below Syntax!!\n");
		printf("Syntax: ./ncpmvdir ./source ./destination <option> <exclusion_list>\n");
		exit(1);  //invalid number of arguments
	}

	//Check if the source directory eists. If not, exit.
	struct stat st;
	char cwd[256];
	getcwd(cwd, sizeof(cwd));		//get current working directory
  	strcat(cwd,source);
	if(stat(cwd,&st) != 0){		//Check for Invalid Source Path
        printf("Invalid Source Path. Try again!!\n");	
		exit(1);
	}	

	//call the ncpmvdir function 
	int res  = ncpmvdir(source, destination, move_or_copy, extensions, extensions_count );
	if (res != 0)
	{
		printf("ERROR \n"); //error in the execution of the program
		exit(1);
	}
	printf("Operation Successful\n");
	return 0;

} // end main

