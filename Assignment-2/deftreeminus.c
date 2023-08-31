/*
  Aathik Thayyil Radhakrishnan, Section 3, 110094762

  1. gcc -o deftreeFunction deftreeFunction.c
  2. ./deftreeFunction [root_process] [OPTION1] [OPTION2] [-processid]

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_SIZE 250

bool isProcessInSubtreeRecursive(int rootPID, int pid) //Function to check if a process is a part of the root subprocess tree
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    int currentPID, currentPPID;

    if (rootPID == pid)  //if root_pid and pid equal, then return true
    {
        return true;
    }

    //Linux command to list the PID and PPID of all the processes
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid,ppid"); //Write into linux_command buffer

    FILE *file_pointer = popen(linux_command, "r");   //popen() to execute the linux_command and return a file pointer that can open it in read mode
    
    if (file_pointer == NULL)     //If file_pointer is NULL, exit
    {
        printf("ERROR\n");
        exit(1);
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL)   // Traverse the entire process tree
    {
        sscanf(output, "%d%d", &currentPID, &currentPPID);  // sscanf() to read the PID & PPID from the output of the linux linux_command
        if (currentPPID == rootPID)
        {
            if (currentPID == pid)  // if PID is found in the process tree, return true
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

    return false;//If process is not found, return false
} // end isProcessInSubtreeRecursive

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

void terminateParentProcessesWithExclusionOption(int rootProcess, int excludeProcess) //Function to terminate Parent process of all zombie process excluding the excluded process
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    char state[3];
    int toKill[100], count = 0;
    int currentPID, currentPPID;

    //Linux Command to list the pid, ppid, state of all the process 
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid,ppid,state");

    FILE *file_pointer = popen(linux_command, "r");  //open it in read mode using popen()

    if (file_pointer == NULL)    //If file_pointer is NULL, exit
    {
        printf("ERROR\n");
        return;
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL)  //Traverse the output
    {
        sscanf(output, "%d %d %s", &currentPID, &currentPPID, state);   //Get PID,PPID and state
        removeWhitespace(state);
        if ((strcmp(state, "Z") == 0 || strcmp(state, "Z+") == 0) && isProcessInSubtreeRecursive(rootProcess, currentPID) && currentPPID != excludeProcess) //Check if process is ZOMBIE && Within the process Tree && NOT in the excluded process
        {
            toKill[count++] = currentPPID;  //Add to kill list
        }
    }

    for (int i = 0; i < count; i++)
    {
        printf("Killed: %d\n", toKill[i]);
        // kill(toKill[i],SIGKILL);     //Kill the process using SIGKILL signal
    }
    pclose(file_pointer);
}   //end terminateParentProcessesWithExclusionOption

void terminateParentProcesses(int rootProcess)  //Function to terminate the parent process of all the zombie process
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    int currentPID, currentPPID;
    char state[3];
    int toKill[100], count = 0;

    //Linux Command to get PID,PPID,STATE of all the process
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid,ppid,state"); //Write it into linux_command buffer

    FILE *file_pointer = popen(linux_command, "r"); //open it in read mode using popen()

    if (file_pointer == NULL)   //If File_pointer is NULL, exit
    {
        printf("ERROR\n");
        return; 
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) //Traverse the output
    {
        sscanf(output, "%d %d %s", &currentPID, &currentPPID, state); //get PID,PPID and state
        removeWhitespace(state);
        if ((strcmp(state, "Z") == 0 || strcmp(state, "Z+") == 0) && isProcessInSubtreeRecursive(rootProcess, currentPID))  //Check if zombie process and process is within the process subtree
        {
            toKill[count++] = currentPPID;  //add to kill list
        }
    }

    for (int i = 0; i < count; i++)
    {
        printf("Killed: %d\n", toKill[i]);
        // kill(toKill[i],SIGKILL);     //Kill the process using SIGKILL signal
    }
    pclose(file_pointer);
} //end terminateParentProcesses

void terminateParentProcessWithElapsedTime(int rootPID, int procElapsedTime)    //Function to terminate parent process of zombie process whole elpased time is greater than the time given procElapsedTime
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    char state[3];
    int toKill[100], count = 0;
    int currentPID, currentPPID, currentElapsedTimeMin, currentElapsedTimeSec;
    bool defunctFound = false;

    // Linux Command to get pid,ppid,etime,state of all the process
    snprintf(linux_command, 200, "ps -ao pid,ppid,etime,state");

    FILE *file_pointer = popen(linux_command, "r"); //open in reading mode using popen()

    if (file_pointer == NULL)   //if file_pointer is NULL, exit
    {
        printf("ERROR\n");
        return;
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) // Traverse the process tree
    {
        sscanf(output, "%d%d%d:%d%s", &currentPID, &currentPPID, &currentElapsedTimeMin, &currentElapsedTimeSec, state);
        removeWhitespace(state);
        currentElapsedTimeMin = currentElapsedTimeMin + (currentElapsedTimeSec / 60); //get the elapsed time in min
        if ((strcmp(state, "Z") == 0 || strcmp(state, "Z+") == 0) && currentElapsedTimeMin > procElapsedTime && isProcessInSubtreeRecursive(rootPID, currentPID)) //check if proces is zombie and time elapsed is greater and process in process tree
        {
            defunctFound = true;            //Parent Found to terminate
            toKill[count++] = currentPPID;  //Add it in the kill list
        }
    }

    // Close the file pointer
    pclose(file_pointer);

    if (!defunctFound)      //if no pid is found to terminate
    {
        printf("No processes found to terminate\n");
    }
    else  
    {
        for (int i = 0; i < count; i++)
        {
            printf("Killed: %d\n", toKill[i]);
            // kill(toKill[i],SIGKILL);     //Kill using SIGKILL
        }
    }
} //end terminateParentProcesses

//Function to terminate the parent process of Zombie process whose elapsed time is greater excluding the excluded process
void terminateParentProcessWithElapsedTimeAndExcludedProcess(int rootPID, int procElapsedTime, int ex)
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    char state[3];
    int currentPID, currentPPID, currentElapsedTimeMin, currentElapsedTimeSec;
    bool defunctFound = false;
    int toKill[100], count = 0;

    // Linux Command to get pid,ppid,etime,state of all the process
    snprintf(linux_command, 200, "ps -ao pid,ppid,etime,state");

    FILE *file_pointer = popen(linux_command, "r"); //open in readmode

    if (file_pointer == NULL)   //exit if filepointer is NULL
    {
        printf("ERROR\n");
        exit(1);
    }

    int exclude = ex;
    while (fgets(output, sizeof(output), file_pointer) != NULL) // Traverse the process tree
    {   
        sscanf(output, "%d%d%d:%d%s", &currentPID, &currentPPID, &currentElapsedTimeMin, &currentElapsedTimeSec, state); 
        
        currentElapsedTimeMin = currentElapsedTimeMin + (currentElapsedTimeSec * 60); //Calculate the current time in min
        removeWhitespace(state);
        if ((strcmp(state, "Z") == 0 || strcmp(state, "Z+") == 0) && currentElapsedTimeMin > procElapsedTime && isProcessInSubtreeRecursive(rootPID, currentPID)  && (currentPPID != exclude))  //Additionally check if process != excludedProcess
        {
            defunctFound = true;
            toKill[count++] = currentPPID;  //add to kill list
        }
    }

    pclose(file_pointer);

    if (!defunctFound)  //if no pid is found to terminate
    {
        printf("No processes found to terminate\n");
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            printf("Killed: %d\n", toKill[i]);
            // kill(toKill[i],SIGKILL);     //Kill using SIGKILL
        }
    }
}   //end terminateParentProcessWithElapsedTimeAndExcludedProcess

int numOfZombieChildren(int rootProcess) //Function that returns the number of zombie children
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    int numOfDefunct = 0;

    //linux command that lists pid of all the zombie children of a process pid
    snprintf(linux_command, 200, "ps -ao pid=,ppid=,stat= | awk '$2 == %d { if($3 == \"Z+\" || $3 == \"Z\") print $1}'", rootProcess);
    
    FILE *childFp = popen(linux_command, "r");  //open in read mode
    if (childFp == NULL)    //exit if file_pointer NULL
    {
        printf("ERROR\n");
        exit(1);
    }
    
    while (fgets(output, sizeof(output), childFp) != NULL)  //Traverse
    {
        int child_pid;
        sscanf(output, "%d", &child_pid);
        numOfDefunct++;     //Increment the Zombie process counter
    }
    pclose(childFp);
    return numOfDefunct;    //return the zombie process count
}   //numOfZombieChildren

//Function to terminate Parent Process of a Zombie Process with >= NUM_OF_DEF_PROC
void terminateParentProcessesWithNumberOfDefunct(int rootPID, int NUM_OF_DEF_PROC)
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    int toKill[100];
    int num_of_zombie_Children, count = 0;
    bool processTerminated = false;
    int currentPID, currentPPID, currentDefunctChildren;

    //Linux command to get the pid,ppid,state
    snprintf(linux_command, sizeof(linux_command), "ps  -ao pid=,ppid=,stat=");

    FILE *file_pointer = popen(linux_command, "r"); //open in read mode
    if (file_pointer == NULL)   //exit if file_pointer is NULL
    {
        printf("ERROR\n");
        exit(1);
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) //Traverse
    {
        sscanf(output, "%d%d", &currentPID, &currentPPID);
        if (isProcessInSubtreeRecursive(rootPID, currentPID)) //Check if the process is part of the root process tree
        {
            
            num_of_zombie_Children = numOfZombieChildren(currentPID); //get the number of zombie children
            if (num_of_zombie_Children >= NUM_OF_DEF_PROC) //check zombie children condition
            {
                toKill[count++] = currentPID; //Add to kill list
                processTerminated = true;
            }
        }
    }

    if (!processTerminated) //If no process is found to kill
    {
        printf("No processes found to terminate\n");
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            printf("Killed: %d\n", toKill[i]);
            // kill(toKill[i],SIGKILL);     //Kill using SIGKILL
        }
    }
    pclose(file_pointer);
} //end terminateParentProcessesWithNumberOfDefunct

//Function to terminate Parent Process of a Zombie Process with >= NUM_OF_DEF_PROC (excluding the excluded process)
void terminateParentProcessesWithNumberOfDefunctAndExcludedProcess(int rootPID, int NUM_OF_DEF_PROC, int excludedProcess)
{
    char linux_command[MAX_SIZE];
    char output[MAX_SIZE];
    int toKill[100];
    int num_of_zombie_Children, count = 0;
    int currentPID, currentPPID, currentDefunctChildren;

    //Linux Command to get pid,ppid and state
    snprintf(linux_command, sizeof(linux_command), "ps -ao pid=,ppid=,stat=");

    FILE *file_pointer = popen(linux_command, "r"); //open in read mode
    if (file_pointer == NULL)   //exit if file_pointer is NULL
    {
        printf("ERROR\n");
        exit(1);
    }

    while (fgets(output, sizeof(output), file_pointer) != NULL) //Traverse
    {
        sscanf(output, "%d%d", &currentPID, &currentPPID);

        if (isProcessInSubtreeRecursive(rootPID, currentPID))   //Check if PID part of root PID subtree
        {
            num_of_zombie_Children = numOfZombieChildren(currentPID);   //get the number of zombie children
            if (num_of_zombie_Children >= NUM_OF_DEF_PROC && currentPID != excludedProcess) //check the zombie children condition and whether the process is not the excluded process
            {
                toKill[count++] = currentPID;   //add to kill list
            }
        }
    }

    for (int i = 0; i < count; i++)
    {
        printf("Killed: %d\n", toKill[i]);
        // kill(toKill[i],SIGKILL);     //Kill using SIGKILL
    }

    pclose(file_pointer);
} //end terminateParentProcessesWithNumberOfDefunctAndExcludedProcess

//Function that calls the appropriate functions based on the arguments
void deftreeFunction(int rootprocess, int excluded_process, int option, int elapsedtime, int numofdefuncts)
{

    if (option == 1) //Terminate the parent process of all zombie process in the root subtree excluding the excludedProcess
    {
        terminateParentProcessesWithExclusionOption(rootprocess, excluded_process);
    }
    else if (option == 0) //Terminate the parent process of all zombie process in the root subtree 
    {
        terminateParentProcesses(rootprocess);
    }
    else if (option == 2) //Terminate the parent process of all zombie process whose elapsed time is greater than the given time
    {
        terminateParentProcessWithElapsedTime(rootprocess, elapsedtime);
    }
    else if (option == 3) //Terminate the parent process of all zombie process whose parent has >= NUM_OF_DEF_PROC
    {
        terminateParentProcessesWithNumberOfDefunct(rootprocess, numofdefuncts);
    }
    else if (option == 4) //Terminate the parent process of all zombie process whose parent has >= NUM_OF_DEF_PROC excluding the excludedProcess
    {
        terminateParentProcessesWithNumberOfDefunctAndExcludedProcess(rootprocess, numofdefuncts, excluded_process);
    }
    else if (option == 5) //Terminate the parent process of all zombie process whose elapsed time is greater than the given time excluding the excludedProcess
    {
        terminateParentProcessWithElapsedTimeAndExcludedProcess(rootprocess, elapsedtime, excluded_process);
    }
}   //end deftreeFunction

//Main Function
int main(int argc, char *argv[])
{
    if (argc < 2 || argc>5) // Error if argument number is more
    {
        printf("Usage: deftreeminus [root_process] [OPTION1] [OPTION2] [-processid]\n");
        return 1;
    }

    int root_process = atoi(argv[1]);
    int option;
    int excluded_process = 0;
    int elapsed_time = 0;
    int num_of_defuncts = 0;

    if (argc == 2)  // normal execution
    {
        option = 0; 
    }
    else if (argc == 3) // normal execution with excludedProcess
    {
        option = 1;        
        excluded_process = atoi(argv[2]) * -1;
        // printf("%d\n",excluded_process);
    }
    else if (argc > 2 && argc <= 5)
    {
        if (strcmp(argv[2], "-t") == 0) // Check the elapsed time condition 
        {
            option = 2;
            elapsed_time = atoi(argv[3]);
            if (argc == 5)  // Check the elapsed time condition with excludedProcess
            {
                option = 5;
                excluded_process = atoi(argv[4]) * -1;
            }
        }
        else if (strcmp(argv[2], "-b") == 0)    //Check the NUM_OF_DEF_PROC condition
        {
            option = 3;
            num_of_defuncts = atoi(argv[3]);
            if (argc == 5)  //Check the NUM_OF_DEF_PROC condition with excludedProcess
            {
                option = 4;
                excluded_process = atoi(argv[4]) * -1;
            }
        }
    }
    
    deftreeFunction(root_process, excluded_process, option, elapsed_time, num_of_defuncts); //call the deftreeminus function

} //end main