/*
  Aathik Thayyil Radhakrishnan, Section 3, 110094762

  1. gcc -o prcinfo prcinfo.c
  2. ./prcinfo [root_process] [process_id1] [process_id2]...[process_id(5)] [OPTION]

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_SIZE 250
#define MAX_NUM_OF_PROCESS 5

bool isProcessInSubtreeRecursive(int root_pid, int pid)      //Function to check if a process is a part of the root subprocess tree
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    int currentPID, currentPPID;
    if (root_pid == pid)            //if root_pid and pid equal, then return true
    {
        return true;
    }

    //Linux command to list the PID and PPID of all the processes
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid,ppid");  // Write it into the buffer linux_command

    FILE *file_pointer = popen(linux_command, "r");  //popen() to execute the linux_command and return a file pointer that can open it in read mode
    
    if (file_pointer == NULL)           //If file_pointer is NULL, exit
    {
        printf("ERROR \n");
        exit(1);
    }
    
    while (fgets(output, sizeof(output), file_pointer) != NULL)  // Traverse the entire process tree
    {
        sscanf(output, "%d%d", &currentPID, &currentPPID);      // sscanf() to read the PID & PPID from the output of the linux command
        if (currentPPID == root_pid)
        {
            if (currentPID == pid)          // if PID is found in the process tree, return true
            {
                pclose(file_pointer);
                return true;
            }

            
            if (isProcessInSubtreeRecursive(currentPID, pid)) // Check the process tree recursively until PID is found
            {
                pclose(file_pointer);
                return true;
            }
        }
    }

    pclose(file_pointer);

    return false; //If process is not found, return false
} // end isProcessInSubtreeRecursive

void printImmediateChildren(int pid) //Function to print all the immediate children of a process pid
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];

    // Linux command to list the PID and PPID of all the processes whose PPID==pid(immediate children)
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid=,ppid= | awk '$2 == %d {print $1}'", pid);  //Write it into the buffer linux_command

    FILE *file_pointer = popen(linux_command, "r"); //popen() to execute the linux_command and return a file pointer that can open it in read mode
    
    if (file_pointer == NULL)           //If file_pointer is NULL, exit
    {
        printf("ERROR\n");
        exit(1);
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) // Traverse the entire output
    {
        printf("%s", output);           //print the output of the linux_command
    }

    pclose(file_pointer);

} // end immediate children function

void printParentProcess(int pid)    //Function to print the parent process of the process pid
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];

    //Linux command to print the PPID of the process PID
    snprintf(linux_command, sizeof(linux_command), "ps -o ppid= -p %d", pid);  //Write it to the buffer linu_command

    FILE *file_pointer = popen(linux_command, "r");  //popen() to execute the linux_command and return a file pointer that can open it in read mode

    if (file_pointer == NULL)         //If file_pointer is NULL, exit
    {
        printf("ERROR\n");
        exit(1);
    }

    if (fgets(output, sizeof(output), file_pointer) != NULL) // Traverse the entire output
    {
        printf("%d %s", pid, output);         //print the process pid and its parent
    }

    pclose(file_pointer);
} // end parent process function

void removeWhitespace(char *str){   //Function to remove WhiteSpace and NewLine Character from a string
    int j = 0;
    const char* whitespace = " \t\n";  
    for (int i = 0; i < strlen(str); i++) {
        if (strcspn(&str[i], whitespace) == 0) {
            // Skip whitespace and newline characters
            while (str[i] && strcspn(&str[i], whitespace) == 0) {
                i++;
            }
        }

        str[j++] = str[i];
    }
    str[j] = '\0';
} //end  removeWhitespace

void printProcessStatus(int pid)        //Print if a process is DEFUNCT or NOT DEFUNCT
{
    char linux_command[MAX_SIZE];   
    char output[MAX_SIZE];

    //Linux Command to print the state of the process pid
    snprintf(linux_command, sizeof(linux_command), "ps -o state= -p %d", pid);  //Write it into the buffer linux_command

    FILE *file_pointer = popen(linux_command, "r");   //popen() to execute the linux_command and return a file pointer that can open it in read mode

    if (file_pointer == NULL)       //If file_pointer is NULL, exit
    {
        printf("ERROR\n");
        exit(1);
    }

    if (fgets(output, sizeof(output), file_pointer) != NULL)    //Traverse through the result
    {
        removeWhitespace(output);   //to remove whitespace or newLine character from output

        if (strcmp(output, "Z+") == 0 || strcmp(output, "Z") == 0)      //If the process state is Z+ or Z, it is a zombie process
        {
            printf("DEFUNCT\n");           //print DEFUNCT if zombie 
        }
        else
        {
            printf("NOT DEFUNCT\n");            // print NOT DEFUNCT otherwise
        }
    }
    pclose(file_pointer);
} // end process status function

void recursiveDescendentPrintPath(int pid) //Function which uses recursion to print the descendents of PID
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];

    //Linux Command to list pid of procceses whose PPID == PID of the process(descendents/Children of PID) 
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid=,ppid= | awk '$2 == %d {print $1}'", pid); //Write it into the buffer linux_command

    FILE *file_pointer = popen(linux_command, "r"); //popen() to execute the linux_command and return a file pointer that can open it in read mode
   
    if (file_pointer == NULL)      //If file_pointer is NULL, exit
    {
        perror("ERROR");
    }

    
    while (fgets(output, sizeof(output), file_pointer) != NULL) //Traverse the resultant process tree
    {
        int descendent_pid = atoi(output);
        printf("%d\n", descendent_pid);             //print the descendent_pid
        recursiveDescendentPrintPath(descendent_pid);       //Print all the descendents recursively
    }

    pclose(file_pointer);
} // end recursiveDescendentPrintPath

void printAllNonDirectDescendents(int parent_pid)   //Function to print all the non-direct descendants of PID
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];

    // Linux Command to list pid of all the children of PID 
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid=,ppid= | awk '$2 == %d {print $1}'", parent_pid); //Write it into the buffer linux_command

    FILE *file_pointer = popen(linux_command, "r"); //popen() to execute the linux_command and return a file pointer that can open it in read mode
   
    if (file_pointer == NULL)       //If file_pointer is NULL, exit
    {
        perror("ERROR\n");
        return;
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) //traverse all through all the children of PID
    {
        int child_pid = atoi(output);
        recursiveDescendentPrintPath(child_pid); //Call the recursive function to print all the descendents of the children of PID
    }

    pclose(file_pointer);
} // end printAllDescendents

void printImmediateZombieChildren(int parent_pid)  //Function to print the immediate zombie children of a process pid
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    
    //Linux Command to list PID of the processes whose stat == Z or Z+ (Zombie Process)
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid=,ppid=,stat= | awk '$2 == %d {if($3 == \"Z+\" || $3 == \"Z\" ) print $1}'", parent_pid); //Write it into the buffer linux_command

    FILE *file_pointer = popen(linux_command, "r"); //popen() to execute the linux_command and return a file pointer that can open it in read mode
   
    if (file_pointer == NULL)       //If file_pointer is NULL, exit
    {
        perror("ERROR\n");
        return;
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) //Treverse the output 
    {
        int child_pid;
        sscanf(output, "%d", &child_pid);
        printf("%d \n", child_pid);             //print the pid of the children
    }

    pclose(file_pointer);
} // end printImmediateZombieChildren

void printZombieSiblingsOnly(int pid) //Function to print the Zombie Siblings of a process pid
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];

    //Linux command that stores the parent id of currentPID and lists the PID of all the child processes of that parent process whose stat == Z or Z+ (Zombie Process)
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid,ppid,state | awk -v pid=\"%d\" '$1 == pid { parent_pid = $2 } $2 == parent_pid && $1 != pid { if ($3 == \"Z\" || $3 == \"Z+\") print $1 }'", pid); //Write it into the buffer linux_command

    FILE *file_pointer = popen(linux_command, "r"); //popen() to execute the linux_command and return a file pointer that can open it in read mode
    
    if (file_pointer == NULL)       //If file_pointer is NULL, exit
    {
        printf("ERROR\n");
        return;
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) //Traverse the output
    {
        printf("%s", output);   //print the zombie siblings process ids
    }

    pclose(file_pointer);
} // end printZombieSiblingsOnly

void printAllSiblings(int pid)  //Function to print all the siblings of a process pid
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];

    //Linux Command that stores the parent of the currentPID and prints the pid of all the child processes of that parent process
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid,ppid,state | awk -v pid=\"%d\" '$1 == pid { parent_pid = $2 } $2 == parent_pid && $1 != pid { print $1 }'", pid); //Write it into the buffer linux_command

    FILE *file_pointer = popen(linux_command, "r"); //popen() to execute the linux_command and return a file pointer that can open it in read mode
    
    if (file_pointer == NULL)           //If file_pointer is NULL, exit
    {
        printf("ERROR\n");
        return;
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) // Tranvers the output
    {
        printf("%s", output); //Print the process ids of the sibling processes
    }

    pclose(file_pointer);
} // end printAllSiblings

void printAllGrandchildren(int pid) //Function to print all the Grandchildren process of a process pid
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];

    snprintf(linux_command, sizeof(linux_command), "ps -ao pid,ppid,comm | awk -v pid=\"%d\" '$2 == pid { child_pids[$1] } $2 in child_pids { grandchild_pids[$1] } $1 in grandchild_pids { print $1}'", pid); //Write it into the buffer linux_command

    FILE *file_pointer = popen(linux_command, "r"); //popen() to execute the linux_command and return a file pointer that can open it in read mode
    
    if (file_pointer == NULL)           //If file_pointer is NULL, exit
    {
        printf("ERROR\n");
        return;
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) //Traverse the output
    {
        printf("%s", output); //Print the grandchildren process ids
    }

    pclose(file_pointer);
} // end printAllGrandchildren

int prcinfo(int root_process, int processIds[], int num_of_process, int option) //Function that handles the execution of the commands according to the options
{
    bool process_id1 = true;               //Variable to check if process_id1 is part of root process tree
    for (int i = 0; i < num_of_process; i++)
    {
        if (!isProcessInSubtreeRecursive(root_process, processIds[i]))  //Check if all the processId's from the input are under the root process tree
        {
            printf("Process %d not in process tree of root process %d\n", processIds[i], root_process);
            if(i==0){
                process_id1 = false;    //process_id1 not part of the root process tree
            }
            continue;
        }
        else
        {
            printParentProcess(processIds[i]); // If process_id part of root process tree, print the parent process pid
        }
    }
    if (option == 1 && process_id1) // option1: -zz (Print DEFUNCT/NOT DEFUNCT)
    {
        printProcessStatus(processIds[0]);
    }
    else if (option == 2 && process_id1)    // option2: -nd (Print non-direct descendants)
    {
        printAllNonDirectDescendents(processIds[0]);
    }
    else if (option == 3 && process_id1)    // option3: -dd (Print direct descendants)
    {
        printImmediateChildren(processIds[0]);
    }
    else if (option == 4 && process_id1)    // option4: -zc (Print Immediate zombie children processes)
    {
        printImmediateZombieChildren(processIds[0]);
    }
    else if (option == 5 && process_id1)    // option5: -sz (Print zombie sibling processes only)
    {
        printZombieSiblingsOnly(processIds[0]);
    }
    else if (option == 6 && process_id1)    // option2: -sb (Print all sibling processes)
    {
        printAllSiblings(processIds[0]);
    }
    else if (option == 7 && process_id1)    // option2: -gc (Print all grandchildren processes)
    {
        printAllGrandchildren(processIds[0]);
    }

    return 0;
} //end prcinfo

int main(int argc, char *argv[]) //Main Function
{
    if (argc < 3)   // If cmd line arguments are < 3 - error
    {
        printf("Usage: prcinfo [root_process] [process_id1] [process_id2]...[process_id(5)] [OPTION]\n");
        return 1;
    }
    int root_process = atoi(argv[1]);
    int option = 0;
    int num_of_process = argc - 2;

    if (argc >= 4)
    {
        if (strcmp(argv[argc - 1], "-zz") == 0)     //Print DEFUNCT/NOT DEFUNCT
        {
            option = 1;
            num_of_process = argc - 3;
        }
        else if (strcmp(argv[argc - 1], "-nd") == 0)    //Print non-direct descendants
        {
            option = 2;
            num_of_process = argc - 3;
        }
        else if (strcmp(argv[argc - 1], "-dd") == 0)    //Print immediate children
        {
            option = 3;
            num_of_process = argc - 3;
        }
        else if (strcmp(argv[argc - 1], "-zc") == 0)    //Print Immediate zombie children
        {
            option = 4;
            num_of_process = argc - 3;
        }
        else if (strcmp(argv[argc - 1], "-sz") == 0)    //Print zombie siblings only
        {
            option = 5;
            num_of_process = argc - 3;
        }
        else if (strcmp(argv[argc - 1], "-sb") == 0)    //Print all siblings
        {
            option = 6;
            num_of_process = argc - 3;
        }
        else if (strcmp(argv[argc - 1], "-gc") == 0)    //Print all the grandchildren
        {
            option = 7;
            num_of_process = argc - 3;
        }
        else
        {
            num_of_process = argc - 2;
        }
    }

    int processIds[MAX_NUM_OF_PROCESS];  
    for (int i = 0; i < num_of_process; i++)        //Store the processIds in an array processId[]
    {
        processIds[i] = atoi(argv[i + 2]);
    }

    prcinfo(root_process, processIds, num_of_process, option);  //Call the prcinfo function

    return 0;
}   //end Main Function