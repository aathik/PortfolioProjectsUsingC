/* 

Name: Aathik Thayyil Radhakrishnan
Student Id: 110094762
Section No: 3

Usage: 
1.      gcc -o shell23s shell23s.c
2.      ./shell23s

*/


//Header Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

//Define size for arrays
#define ARR_MAX_SIZE 1000
#define ARR_MED_SIZE 50

//global variables for 
int  atr_bg_proc_gpid;
int atr_bg_proc_pid;

//This is a structure to store the commands along with their size
struct atr_structure_for_commands
{
    int atr_argc_of_cmd;
    char* atr_argv_of_cmd[ARR_MED_SIZE];
};
/* 
Use the structure to initilize an array atr_buffer_command_with_size to 
store multiple instances of this structure
*/
struct atr_structure_for_commands atr_buffer_command_with_size[ARR_MED_SIZE]; 



/*
 atr_replace_Tilda_With_Home_Path function changes the tilda (~) in the input atr_command and replaces it with the HOME directory
*/
char * atr_replace_Tilda_With_Home_Path(char atr_str_path[ARR_MAX_SIZE]){
    char *atr_home_dir = getenv("HOME");    //getenv("HOME") returns the home directory
    char *atr_cmd_args[ARR_MED_SIZE];
    char *atr_destination_dir = atr_home_dir;   //add the home directory path to the destination path first
    int atr_num_args = 0;   
    
    //Tokenize the atr_command with the delimiter "~" using strtok and store it in atr_cmd_args
    char *atr_cmd_arg = strtok(atr_str_path, "~");
    while (atr_cmd_arg != NULL) {       //Checks if the next aat_token is NULL
        atr_cmd_args[atr_num_args++] = atr_cmd_arg; //store the aat_token
        atr_cmd_arg = strtok(NULL, "~");    //Change atr_cmd_arg to next aat_token
    }

    //Append/Concatenate the first aat_token to the destination directory
    strcat(atr_destination_dir, atr_cmd_args[0]);
    return atr_destination_dir;
}   // end  atr_replace_Tilda_With_Home_Path


/*
atr_get_delim_position function returns searches for the first occurrence of the atr_delim string within the atr_value 
string and returns the position of the delimiter. If the delimiter is not found, the function returns -1.
*/
int atr_get_delim_position(char *atr_value,char *atr_delim) {
    char *atr_delimiter = strstr(atr_value, atr_delim); // determine where the "&&" delimiter is located.
    if (atr_delimiter != NULL) {
        int atr_pos = atr_delimiter - atr_value; // Determine the position by deducting the string's starting point from it.
        return atr_pos;
    }
    else {
        return -1; // return -1 if the delimiter is not found
    }
}   //end atr_get_delim_position


/*
split_string_first_delim function searches for the first occurrence of the delimiter in the atr_value array 
and then splits it into two parts -  atr_first_part and atr_last_part
*/
void split_string_first_delim(char *atr_value, char *atr_delim, char *atr_first_part, char *atr_last_part) {
    char *atr_str = strchr(atr_value, *atr_delim); // find position of the first delimiter
    if (atr_str != NULL) {
        strncpy(atr_first_part, atr_value, atr_str - atr_value); // copy first aat_token to 'first' string
        atr_first_part[atr_str - atr_value] = '\0'; // terminate 'first' string
        strcpy(atr_last_part, atr_str + 2); // copy the rest of the string to 'rest' string
    }
} // end split_string_first_delim

/*
atr_check_command_length function returns the value of atr_limit, which represents the total number of tokens in the atr_command string.
*/
int atr_check_command_length (char atr_command[ARR_MAX_SIZE], char *atr_delim){
    //Tokenize and increment the 'limit' counter for the atr_command
    char *atr_tok = strtok(atr_command, atr_delim);
    int atr_limit=0;
    while (atr_tok != NULL) {
        atr_limit++;    
        atr_tok = strtok(NULL, atr_delim);
    }
    return atr_limit;   //return the limit
} // end atr_check_command_length

/*
atr_sequences_check function checks if the array atr_seq contains commands whose argc >5 or argc<1, and returns a check flag atr_check.
It is 1, if there is a violation(presence of command with argc>5 || argc<1). Else, 0.
*/
int atr_sequences_check(char **atr_seq, int atr_num){
    int atr_limit = 0;
    int atr_check = 0;
    for (int f = 0; f < atr_num;f++){
        atr_limit = atr_check_command_length(atr_seq[f]," \t\n");
        if(atr_limit<1 || atr_limit>5){
            atr_check = 1;
            break;
        }
    }
    return atr_check;
}   // end atr_sequences_check

/*
This atr_Copy_String_Array fucntion is used to copy source array to destination array
*/
void atr_Copy_String_Array(char** atr_source, char** atr_destination, int size) {
    for (int v = 0; v < size; v++) {
        // Allocate memory for the atr_destination array
        atr_destination[v] = malloc(strlen(atr_source[v]) + 1);
        if (atr_destination[v] == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }

        // Copy the string from atr_source to atr_destination 
        strcpy(atr_destination[v], atr_source[v]);
    }
}   // end atr_Copy_String_Array

/*
    atr_command_execution_function function forks a new process and executes the command specified in the arguments. 
    If the type is 'O', we do redirection of type > (Write or create)
    If the type is 'A', we do redirection of type >> (Append or create)
    If the type is 'I', we do redirection of type < (read)
    If the type is 'B', we execute the executable in the background
    The function returns 0 if the command is executed successfully and 1 if there is an error.
*/
int atr_command_execution_function(char **atr_comm_args, int atr_comm_count,char atr_file[], char type) {
    //Determine if the argument count is within the rules
    if (atr_comm_count < 1 || atr_comm_count > 5) { 
        printf("---RULE Violation (Invalid number of arguments: %d commands)---\n", atr_comm_count);
        return 1;
    }

    //This is to fork() a new child
    int atr_proc_id = fork();       
    if (atr_proc_id == 0) {
        //This is the Child Process
        for (int c = 0; c < atr_comm_count; c++)
        {
            //This is to replace ~ with HOME directory path
            if(strstr(atr_comm_args[c],"~")){
                *(atr_comm_args + c) =  atr_replace_Tilda_With_Home_Path(atr_comm_args[c]);
            }
        }
        //When the I/O redirection is >, create a file if required and write into it.
        if(type=='O'){
            atr_file[strlen(atr_file)] = '\0';
            int atr_fd = open(atr_file, O_RDWR|O_CREAT,0777);   //open file in R/W or Create mode
            int atr_cp1=dup(1);                                 //save the dup of Standard O/P
            int atr_return= dup2(atr_fd,1);                     //Output redirection using dup2
            execvp(atr_comm_args[0],atr_comm_args);             
            atr_return=dup2(atr_cp1,1);
        }
        //When the I/O redirection is >>, create a file if required and append into it.
        else if(type=='A'){
            atr_file[strlen(atr_file)] = '\0';
            int atr_fd = open(atr_file, O_RDWR|O_APPEND|O_CREAT,0777);  //open file in R/W or Create or Append mode
            int atr_cp1=dup(1);                                         //save the dup of Standard O/P
            int atr_return= dup2(atr_fd,1);                             //Output redirection using dup2
            execvp(atr_comm_args[0],atr_comm_args);
            atr_return=dup2(atr_cp1,1);
        }//When the I/O redirection is <, create a file if required and append into it.
        else if(type=='I'){
            atr_file[strlen(atr_file)] = '\0';
            int atr_fd = open(atr_file, O_RDWR, 0777);              //open file in R/W mode
            int atr_cp1=dup(0);                                     //save the dup of Standard O/P
            int atr_return= dup2(atr_fd,0);                         //Output redirection using dup2
            execvp(atr_comm_args[0],atr_comm_args);
            atr_return=dup2(atr_cp1,0);
        }
        //When the type is B, to put/run the executable as a background process
        else if(type=='B'){
            atr_bg_proc_gpid = getpgid(getpid());       //get the pgid of the current pid
            atr_bg_proc_pid = getpid();                 //get the pid
            setpgid(0, getpid());                       //set the new pgid as the PID 
            long atr_fd_max = sysconf(_SC_OPEN_MAX);    
            for (int cl = 3; cl < atr_fd_max; cl++) {   // Close all the file_descriptors except stdin,stdout,stderr
                close(cl);
            }
            execvp(atr_comm_args[0], atr_comm_args);
        }
        else{
            //When type is N - or Normal
            execvp(atr_comm_args[0],atr_comm_args);
        }
        perror("atr_mshell(ERROR)$");
        exit(1);
    } 
    else if (atr_proc_id < 0) {
        printf("ERROR in Fork\n");
        exit(1);
    } 
    else {
        //This is the parent process
        if(type=='B'){           
            //If type is B
            printf("PID[1] %d\n", atr_proc_id);
        }
        else{
            //Else, wait for the child process
            int status_of_pid;
            waitpid(atr_proc_id, &status_of_pid, 0);
            return WIFEXITED(status_of_pid) ? WEXITSTATUS(status_of_pid) : 1;
        }
    }
    return 0;
}   // end atr_command_execution_function


/*
 atr_normal_execution function is for the Default operation of the mini shell. The individual 
 commands without any special expressions are executed here.
*/
int atr_normal_execution(char atr_command[ARR_MAX_SIZE]){
    char *atr_arr_args[ARR_MED_SIZE];
    int atr_num_args = 0;

    //Generate tokens from the atr_command
    char *atr_arg = strtok(atr_command, " \t\n");
    while (atr_arg != NULL) {
        atr_arr_args[atr_num_args++] = atr_arg;
        atr_arg = strtok(NULL, " \t\n");
    }
    //Check if rules are violated
    if (atr_num_args < 1 || atr_num_args > 5) {
        printf("---RULE Violation (Invalid number of arguments: %d commands)---\n", atr_num_args);
        return 1;
    }
    if (atr_num_args == 0)
    {
        return 0;
    }
    atr_arr_args[atr_num_args] = NULL;

    //Execute the command using the atr_command_execution_function. Type is provided as N- Normal
    int atr_status = atr_command_execution_function(atr_arr_args,atr_num_args,NULL,'N');
    if (atr_status != 0) {
        //skip if the status is not equal to 0
    }
    return 0;
}   // end atr_normal_execution


/*
atr_pipe_function creates a pipeline of commands by forking child processes and redirecting their input and output streams.
*/
void atr_pipe_function(int atr_num_c){

    int atr_fd_pipe[2], atr_buffer_fd;
    int atr_num_of_pipes = atr_num_c-1;
    for(int t=0;t<=atr_num_of_pipes;t++)
    {
        //Change the tilda into the HOME directory path
        for (int w = 0; w < atr_buffer_command_with_size[t].atr_argc_of_cmd; w++)
        {
            if(strstr(atr_buffer_command_with_size[t].atr_argv_of_cmd[w],"~")){
                atr_buffer_command_with_size[t].atr_argv_of_cmd[w] =  atr_replace_Tilda_With_Home_Path(atr_buffer_command_with_size[t].atr_argv_of_cmd[w]);
            }
        }

        // This is to create pipes
        if (t < atr_num_of_pipes) {
            if (pipe(atr_fd_pipe) == -1) {
                perror("pipe failed");
                exit(1);
            }
        }

        // This is to fork a child
        int aat_proc_id = fork();
        if(aat_proc_id == 0)
        {
            //This is the Child Process
            if(t==0)
            {            
                dup2(atr_fd_pipe[1], STDOUT_FILENO);    //redirect the std_out to the write end of pipe
            }
            else if(t == atr_num_c-1)
            {
                dup2(atr_buffer_fd,STDIN_FILENO);       // redirect the std_in from the atr_buffer_fd
            }
            else
            {
                dup2(atr_buffer_fd,STDIN_FILENO);       // redirect the std_in from the atr_buffer_fd
                dup2(atr_fd_pipe[1],STDOUT_FILENO);     //redirect the std_out to the write end of pipe
            }

            //Executed using execvp()
            execvp(atr_buffer_command_with_size[t].atr_argv_of_cmd[0],atr_buffer_command_with_size[t].atr_argv_of_cmd);
        }
        else
        {
            //This is the Parent Process
            wait(NULL);
            if(t!=atr_num_of_pipes)
            {
                atr_buffer_fd = atr_fd_pipe[0];     //redirect the read end to the atr_buffer_fd
                close(atr_fd_pipe[1]);              //Close the pipe's wite end
            }
        }
    }
  
}   // end atr_pipe_function


/*
    execute_pipe_command function tokenizes the input string into individual commands and then to individual arguments and stores
    them in a structure along with its count
*/
void execute_pipe_command (char atr_input_str[ARR_MAX_SIZE]) {
    char *atr_cmds_for_pipe[ARR_MED_SIZE];
    int atr_num_of_cmds=0;
    
    //Tokenize the string with delimiter as "|"
    char *atr_cmd_tok = strtok(atr_input_str,"|");
    while (atr_cmd_tok != NULL) {
        atr_cmds_for_pipe[atr_num_of_cmds++] = atr_cmd_tok;
        atr_cmd_tok = strtok(NULL, "|");
    }

    //This is to check the number of pipes, for any rule violation
    if(atr_num_of_cmds-1>7){
        printf("----RULE Violation(Invalid Number of Pipes: %d Pipes)------\n",atr_num_of_cmds-1);
        return;
    }

    
    for (int z = 0; z < atr_num_of_cmds;z++){
        int atr_num_of_cmds_inside_the_pipe = 0;

        //Tokenize each command into individual arguments
        char *atr_args_inside_the_pipe = strtok(atr_cmds_for_pipe[z], " \t\n");
        while (atr_args_inside_the_pipe != NULL) {
            atr_buffer_command_with_size[z].atr_argv_of_cmd[atr_num_of_cmds_inside_the_pipe++] = atr_args_inside_the_pipe;
            atr_args_inside_the_pipe = strtok(NULL, " \t\n");
        }
        //storing it into an array of type struct
        atr_buffer_command_with_size[z].atr_argc_of_cmd = atr_num_of_cmds_inside_the_pipe;
        //This is to check the number of individual arguments for rule violation
        if(atr_num_of_cmds_inside_the_pipe>5){
            printf("-------RULE Violation(Exiting)---------\n");
            return;
        }
    }
    atr_pipe_function(atr_num_of_cmds);
  
}   // end execute_pipe_command


/*
    atr_conditional_command_execution function splits the atr_input_str into two parts based on the first occurrence of && or ||.
    It then executes the first part of the string, and checks the status code. If it is a non-zero value, the function returns.
    Or else, it will execute the second part and so on. The process continues until the entire string has been executed or until
    the non-zero status code is returned.
*/
void atr_conditional_command_execution(char atr_command[ARR_MAX_SIZE]){
    char atr_first_part[800], atr_last_part[800],copy[ARR_MAX_SIZE];
    char *atr_cpy_comm=atr_command;
    char *atr_cpy_pos=atr_command;
    char *atr_cmds[ARR_MED_SIZE];
    int atr_num=0;

    //This is to Tokenize the input using "&&||" as delimites
    strcpy(copy,atr_command);
    char *atr_tok = strtok(copy,"&&||");
    while( atr_tok != NULL){
        atr_cmds[atr_num++] = atr_tok;
        atr_tok = strtok(NULL, "&&||");
    }
    atr_cmds[atr_num] = NULL;

    //This is to check if it violates any of the rules
    int check = atr_sequences_check(atr_cmds,atr_num);
    if(check == 1){
        printf("----RULE Violation(Invalid number of commands)----\n");
        return;
    }

    //Gets the first AND position and OR position from the input string
    int atr_pos_of_AND = atr_get_delim_position(atr_cpy_pos, "&&");
    int atr_pos_of_OR = atr_get_delim_position(atr_cpy_pos, "||");

    

    // executing commands while splitting itself
    while (strlen(atr_cpy_comm)>0)
    {   
        //input has only OR
        if(atr_pos_of_AND==-1 &&atr_pos_of_OR!=-1){
            //This is to split the string with the delimiter
            split_string_first_delim(atr_cpy_comm, "||", atr_first_part, atr_last_part);

            //This is to Tokenize the command into individual arguments
            char *aat_args[ARR_MED_SIZE];
            int aat_num_args = 0;   
            char *aat_arg = strtok(atr_first_part, " \t\n");
            while (aat_arg != NULL) {
                aat_args[aat_num_args++] = aat_arg;
                aat_arg = strtok(NULL, " \t\n");
            }
            aat_args[aat_num_args] = NULL;

            //This is to execute the individual arguments
            int aat_status = atr_command_execution_function(aat_args, aat_num_args, NULL, 'N');
            if (aat_status != 0) {
                 //printf("----\n");
            }
            else{
                break;
            }

        }
        //input has only AND
        else if(atr_pos_of_AND!=-1 &&atr_pos_of_OR==-1){
            //This is to split the string with the delimiter
            split_string_first_delim(atr_cpy_comm, "&&", atr_first_part, atr_last_part);

             //This is to Tokenize the command into individual arguments
            char *aat_args[ARR_MED_SIZE];
            int aat_num_args = 0;
            char *aat_arg = strtok(atr_first_part, " \t\n");
            while (aat_arg != NULL) {
                aat_args[aat_num_args++] = aat_arg;
                aat_arg = strtok(NULL, " \t\n");
            }
            aat_args[aat_num_args] = NULL;

            //This is to execute the individual arguments
            int aat_status = atr_command_execution_function(aat_args, aat_num_args, NULL, 'N');
            if (aat_status != 0) {
                break;
            }
        }
        
        else if(atr_pos_of_AND>atr_pos_of_OR){
            //This is to split the string with the delimiter
            split_string_first_delim(atr_cpy_comm, "||", atr_first_part, atr_last_part);

             //This is to Tokenize the command into individual arguments
            char *aat_args[ARR_MED_SIZE];
            int aat_num_args = 0;
            char *aat_arg = strtok(atr_first_part, " \t\n");
            while (aat_arg != NULL) {
                aat_args[aat_num_args++] = aat_arg;
                aat_arg = strtok(NULL, " \t\n");
            }
            aat_args[aat_num_args] = NULL;

            int aat_status = atr_command_execution_function(aat_args,aat_num_args, NULL, 'N');
            if (aat_status != 0) {
                // printf("Failure with  status %d\n", status);
            }else{
                if(atr_pos_of_AND!=1){
                    split_string_first_delim(atr_cpy_comm, "&&", atr_first_part, atr_last_part);
                }else{
                    break;
                }
            }
        }
        else if(atr_pos_of_AND<atr_pos_of_OR){
            //This is to split the string with the delimiter
            split_string_first_delim(atr_cpy_comm, "&&", atr_first_part, atr_last_part);

             //This is to Tokenize the command into individual arguments
            char *aat_args[ARR_MED_SIZE];
            int aat_num_args = 0;
            char *aat_arg = strtok(atr_first_part, " \t\n");
            while (aat_arg != NULL) {
                aat_args[aat_num_args++] = aat_arg;
                aat_arg = strtok(NULL, " \t\n");
            }
            aat_args[aat_num_args] = NULL;

            //This is to execute the individual arguments
            int aat_status = atr_command_execution_function(aat_args,aat_num_args, NULL,'N');
            if (aat_status != 0) {
                if(atr_pos_of_OR!=1){
                    split_string_first_delim(atr_cpy_comm, "||", atr_first_part, atr_last_part);
                }else{
                    break;
                }
            }
        }
        else if(atr_pos_of_AND==-1 && atr_pos_of_OR==-1){

             //This is to Tokenize the command into individual arguments
            char *aat_args[ARR_MED_SIZE];
            int aat_num_args = 0;
            atr_last_part[strlen(atr_last_part) - 1] = '\0';
            char *aat_arg = strtok(atr_last_part, " \t\n");
            while (aat_arg != NULL) {
                aat_args[aat_num_args++] = aat_arg;
                aat_arg = strtok(NULL, " \t\n");
            }
            aat_args[aat_num_args] = NULL;


            //This is to execute the individual arguments
            int aat_status = atr_command_execution_function(aat_args,aat_num_args,NULL, 'N');
            if (aat_status != 0) {
                //printf("----\n");
                break;
            }
            break;
        }

        strcpy(atr_cpy_comm, atr_last_part);
        strcpy(atr_cpy_pos, atr_last_part);
        atr_pos_of_AND = atr_get_delim_position(atr_cpy_pos, "&&");
        atr_pos_of_OR = atr_get_delim_position(atr_cpy_pos, "||");
    }
}

/*
    atr_redirection_of_Output_TypeA function tokenizes the command using ">>" and executes the command by calling
    atr_command_execution_function. The type is passed as A (Append Or Create)
*/
int atr_redirection_of_Output_TypeA(char atr_command[ARR_MAX_SIZE]){
    
    
    char *aat_args[ARR_MED_SIZE], *aat_SC_args[ARR_MED_SIZE], *aat_file_name[ARR_MED_SIZE];
    int aat_num_args = 0, aat_aat_num_of_SC_args = 0, aat_num_of_files = 0;
    
     //This is to Tokenize the command using ">>" as delimiter
    char *aat_token = strtok(atr_command,">>");
    while (aat_token != NULL) {
        aat_SC_args[aat_aat_num_of_SC_args++] = aat_token;
        aat_token = strtok(NULL, ">>");
        
    }

    //This is to Tokenize the command into individual arguments
    char *aat_arg = strtok(aat_SC_args[0], " \t\n");
    while (aat_arg != NULL) {
        aat_args[aat_num_args++] = aat_arg;
        aat_arg = strtok(NULL, " \t\n");
    }
    //This is to check if any rules are violated
    if (aat_num_args < 1 || aat_num_args > 5) {
        printf("---RULE Violation (Invalid number of arguments: %d arguments)---\n", aat_num_args);
        return 1;
    }

    //This is to tokenize the fileName given at the end of redirection
    char *atr_file = strtok(aat_SC_args[1], " \t\n");
    while (atr_file != NULL) {
        aat_file_name[aat_num_of_files++] = atr_file;
        atr_file = strtok(NULL, " \t\n");
    }

    if (aat_num_args == 0)
    {
        return 0;
    }
    aat_args[aat_num_args] = NULL;

    // This is to execute the command
    int aat_status = atr_command_execution_function(aat_args,aat_num_args,aat_file_name[0],'A');
    if (aat_status != 0) {
        //printf("----\n");
    }
    return 0;
} //end atr_conditional_command_execution

/*
    atr_redirection_of_Output_TypeO function tokenizes the command using ">" and executes the command by calling
    atr_command_execution_function. The type is passed as O (Write Or Create)
*/
int atr_redirection_of_Output_TypeO(char atr_command[ARR_MAX_SIZE]){
    
    char *aat_args[ARR_MED_SIZE], *aat_SC_args[ARR_MED_SIZE], *aat_file_name[ARR_MED_SIZE];
    int aat_num_args = 0, aat_aat_num_of_SC_args = 0, aat_num_of_files = 0;
    
    //This is to Tokenize the command using ">" as delimiter
    char *aat_token = strtok(atr_command,">");
    while (aat_token != NULL) {
        aat_SC_args[aat_aat_num_of_SC_args++] = aat_token;
        aat_token = strtok(NULL, ">");
        
    }

    //This is to Tokenize the command into individual arguments
    char *aat_arg = strtok(aat_SC_args[0], " \t\n");
    while (aat_arg != NULL) {
        aat_args[aat_num_args++] = aat_arg;
        aat_arg = strtok(NULL, " \t\n");
    }
    //This is to check if any rules are violated
    if (aat_num_args < 1 || aat_num_args > 5) {
        printf("---RULE Violation (Invalid number of arguments: %d commands)---\n", aat_num_args);
        return 1;
    }
    
     //This is to tokenize the fileName given at the end of redirection
    char *atr_file = strtok(aat_SC_args[1], " \t\n");
    while (atr_file != NULL) {
        aat_file_name[aat_num_of_files++] = atr_file;
        atr_file = strtok(NULL, " \t\n");
        
    }

    if (aat_num_args == 0)
    {
        return 0;
    }
    aat_args[aat_num_args] = NULL;

    // This is to execute the command
    int aat_status = atr_command_execution_function(aat_args,aat_num_args,aat_file_name[0],'O');
    if (aat_status != 0) {
        //printf("----\n");
    }
    return 0;
}       // end atr_redirection_of_Output_TypeO

/*
    atr_redirection_of_Input_TypeI function tokenizes the command using "<" and executes the command by calling
    atr_command_execution_function. The type is passed as I (Read)
*/
int atr_redirection_of_Input_TypeI(char atr_command[ARR_MAX_SIZE]){
    char *aat_args[ARR_MED_SIZE], *aat_SC_args[ARR_MED_SIZE], *aat_file_name[ARR_MED_SIZE];
    int aat_num_args = 0, aat_aat_num_of_SC_args = 0, aat_num_of_files = 0;

    //This is to Tokenize the command using "<" as delimiter
    char *aat_token = strtok(atr_command, "<");
    while (aat_token != NULL) {
        aat_SC_args[aat_aat_num_of_SC_args++] = aat_token;
        aat_token = strtok(NULL, "<");
        
    }

    //This is to Tokenize the command into individual arguments
    char *aat_arg = strtok(aat_SC_args[0], " \t\n");
    while (aat_arg != NULL) {
        aat_args[aat_num_args++] = aat_arg;
        aat_arg = strtok(NULL, " \t\n");
    }
    //This is to check if any rules are violated
    if (aat_num_args < 1 || aat_num_args > 5) {
        printf("---RULE Violation (Invalid number of arguments: %d commands)---\n", aat_num_args);
        return 1;
    }

     //This is to tokenize the fileName given at the end of redirection
    char *atr_file = strtok(aat_SC_args[1], " \t\n");
    while (atr_file != NULL) {
        aat_file_name[aat_num_of_files++] = atr_file;
        atr_file = strtok(NULL, " \t\n");
    }

    if (aat_num_args == 0)
    {
        return 0;
    }
    
    aat_args[aat_num_args] = NULL;

    // This is to execute the command
    int aat_status = atr_command_execution_function(aat_args,aat_num_args,aat_file_name[0], 'I');
    if (aat_status != 0) {
        //printf("----\n");
    }
    return 0;
} // end atr_redirection_of_Input_TypeI

/*
    atr_sequential_Execution function tokenizes the input with ";" as delimiter and it calls the respective functions
    to execute the respective individual commands.
*/
int atr_sequential_Execution(char atr_command[ARR_MAX_SIZE]){
    char *atr_sequences[ARR_MED_SIZE], *copy_sequence[ARR_MED_SIZE];
    int atr_num_sequences = 0;

    //This is to Tokenize the command using ";" as delimiter
    char *seq = strtok(atr_command, ";");
    while (seq != NULL) {
        
        atr_sequences[atr_num_sequences] = seq;
        seq = strtok(NULL, ";");
        atr_num_sequences++;
    }

    //This is to check if any rules are violated
    if(atr_num_sequences>7 || atr_num_sequences<0){
        printf("----RULE Violation(Invalid number of sequences: %d sequences)----\n",atr_num_sequences);
        return 0;
    }

    atr_sequences[atr_num_sequences] = NULL;

    //This is to check if any rules are violated
    atr_Copy_String_Array(atr_sequences, copy_sequence, atr_num_sequences);
    int check = atr_sequences_check(copy_sequence,atr_num_sequences);
    if (check==1)
    {
        printf("----RULE Violation(Invalid number of commands)----\n");
        return 1;
    }

    for (int m = 0; m < atr_num_sequences;m++){
        
        if(strstr(atr_sequences[m], ">>") != NULL){
            //Check if the command sequence contains ">>"
            atr_redirection_of_Output_TypeA(atr_sequences[m]);
        }else if(strstr(atr_sequences[m],">")!=NULL){
            //Check if the command sequence contains ">"
            atr_redirection_of_Output_TypeO(atr_sequences[m]);
        }else if(strstr(atr_sequences[m],"<")!=NULL){
            //Check if the command sequence contains "<"
            atr_redirection_of_Input_TypeI(atr_sequences[m]);
        }
        else{
            atr_normal_execution(atr_sequences[m]);
        }
    }
    return 0;
} // end atr_sequential_Execution

/*
    atr_bg_proc_function function tokenizes the input command and calls the function to execute it
*/
int atr_bg_proc_function(char atr_command[ARR_MAX_SIZE]){
    char *aat_args[ARR_MED_SIZE];
    int aat_num_args = 0;
    int aat_aat_num_of_SC_args = 0;
    char *aat_SC_args[ARR_MED_SIZE];
    
    //This is to Tokenize the command using "&" as delimiter
    char *aat_token = strtok(atr_command,"&");
    while (aat_token != NULL) {
        aat_SC_args[aat_aat_num_of_SC_args++] = aat_token;
        aat_token = strtok(NULL, "&");
        
    }

    //This is to Tokenize the input into individiual commands
    char *aat_arg = strtok(aat_SC_args[0], " \t\n");
    while (aat_arg != NULL) {
        aat_args[aat_num_args++] = aat_arg;
        aat_arg = strtok(NULL, " \t\n");
    }

    if (aat_num_args == 0)
    {
        return 0;
    }
    
    aat_args[aat_num_args] = NULL;

    // This is to execute the command
    int aat_status = atr_command_execution_function(aat_args,aat_num_args, NULL,'B');
    if (aat_status != 0) {
        //printf("----\n");
    }
    return 0;
}

//Main Function
int main(){

    while (1)
    {
        char atr_input_cmd[ARR_MAX_SIZE];
        char atr_cpy_cmd[ARR_MAX_SIZE];
        int num_atr_commd_args = 0;

        printf("atr_mshell$ ");

        //This is to read the input into atr_input_cmd
        if(fgets(atr_input_cmd, ARR_MAX_SIZE, stdin) == NULL){
            printf("ERROR\n");
            exit(1);
        }

        //This is to make a copy of the input command
        strcpy(atr_cpy_cmd, atr_input_cmd);

        //This is to check for the "fg", to bring the code to the foreground
        if(strcmp(atr_input_cmd,"fg")==0){
            setpgid(atr_bg_proc_pid,  atr_bg_proc_gpid);    //set the pgid for atr_bg_proc_pid
            continue;
        } 
        //This is to check for conditional statements
        else if (strstr(atr_input_cmd, "&&") != NULL || strstr(atr_input_cmd, "||") != NULL)
        {
            // This is to check for any rule violation
            if(atr_check_command_length(atr_cpy_cmd,"&&||")>7){
                printf("----RULE Violation(Invalid number of commands)----\n");
            }
            else{
                atr_conditional_command_execution(atr_input_cmd);
            }
            
        }
        //This is to check for pipes
        else if(strstr(atr_input_cmd, "|") != NULL){
            execute_pipe_command(atr_input_cmd);
        }
        //This is to check for sequential commands
        else if (strstr(atr_input_cmd, ";") != NULL)
        {
            atr_sequential_Execution(atr_input_cmd);
        }
        //This is to check for redirection_typeA commands
        else if (strstr(atr_input_cmd, ">>") != NULL)
        {
            atr_redirection_of_Output_TypeA(atr_input_cmd);
        }
        //This is to check for redirection_typeO commands
        else if (strstr(atr_input_cmd, ">") != NULL)
        {
            atr_redirection_of_Output_TypeO(atr_input_cmd);
        }
        //This is to check for redirection_typeI commands
        else if (strstr(atr_input_cmd, "<") != NULL)
        {
            atr_redirection_of_Input_TypeI(atr_input_cmd);
        }
        //This is to check for executing an executable in the background
        else if (strstr(atr_input_cmd, "&") != NULL)
        {
            atr_bg_proc_function(atr_input_cmd);
        }
        //Otherwise, normal/default execution of commands
        else{
            atr_normal_execution(atr_input_cmd);
        }
    }

    return 0;
}   //end main()