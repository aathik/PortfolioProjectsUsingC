#!/bin/bash

# Name: Aathik Thayyil Radhakrishnan
# Section: 3
# Student No: 110094762

AATHIK_FULLORINC=0  #Variable to check if we need to do a FULL backup or an INCREMENTAL backup 
AATHIK_DEST_DIR_FOR_BACKLOG="$HOME/home/backup"     # This is the directory where we are going to save the backup as per the question
AATHIK_DEST_DIR_FOR_LATEST_BACKUP=""       # This variable will store the last/latest backup tar file destination
AATHIK_FULLBACKUPFILE=20000
AATHIK_INCREMENTALBACKUPFILE=10000 


#aathik_IncrementalBackUp_Function is to perform INCREMENTAL Backup of only those .txt files that are created after the previous backup
function aathik_IncrementalBackUp_Function() {
    #Here we set AATHIK_DIR_FOR_BACKUP as the backup directory for incremental backup as described in the question
    AATHIK_DIR_FOR_BACKUP="$HOME/home/backup/ib"

    # Here we check if AATHIK_DIR_FOR_BACKUP (directory for Incremental backup) is present in the current working directory
    if [ ! -d "$AATHIK_DIR_FOR_BACKUP" ]; then
        #If the AATHIK_DIR_FOR_BACKUP is not present in the current working directory, we create it using mkdir
        mkdir -p "$AATHIK_DIR_FOR_BACKUP"
    fi

    # Here we check for all the new.txt files in the current working directory that have been modified more recently than the file or directory at the path stored in AATHIK_DEST_DIR_FOR_LATEST_BACKUP
    if ! [ $(find $HOME -type f -name "*.txt" -newer $AATHIK_DEST_DIR_FOR_LATEST_BACKUP | wc -l) -gt 0 ]; then
        #If new files are not present in the directory before the previous backup, we update the below message in the backuplog
        echo "$(date +"%a %d %b %Y %I:%M:%S %p %Z") No changes-Incremental backup was not created" >> $AATHIK_DEST_DIR_FOR_BACKLOG/backup.log
        return 0
    fi

    # Here we create use the date&time to create the .tar file name as per the question
    AATHIK_DATETIMEFUNCTION=$(date "+%Y%m%d%H%M%S")
    ((AATHIK_INCREMENTALBACKUPFILE++))
    AATHIK_TARFILENAME="ib$AATHIK_INCREMENTALBACKUPFILE.tar"
    #Here we pipe the new .txt files in the specified working directory to tar command, which creates the .tar file according to the filename
    find $HOME -type f -name "*.txt" -newer $AATHIK_DEST_DIR_FOR_LATEST_BACKUP -print0 | tar -cvf $AATHIK_DIR_FOR_BACKUP/$AATHIK_TARFILENAME --null -T - > /dev/null 2>&1

    # Here we update the destination directory for the latest/most recent backup in AATHIK_DEST_DIR_FOR_LATEST_BACKUP
    AATHIK_DEST_DIR_FOR_LATEST_BACKUP="$AATHIK_DIR_FOR_BACKUP/$AATHIK_TARFILENAME"

    # And we update the below message into the BackupLog file
    echo "$(date +"%a %d %b %Y %I:%M:%S %p %Z") $AATHIK_TARFILENAME was created" >> $AATHIK_DEST_DIR_FOR_BACKLOG/backup.log
}

#aathik_FullBackUp_Function is to perform FULL Backup of all the files in the specified directory
function aathik_FullBackUp_Function() {

    # Here we the set AATHIK_DIR_FOR_BACKUP to the destination directory to store the .tar files after complete backup
    AATHIK_DIR_FOR_BACKUP="$HOME/home/backup/cb"

    #  Here we check if the directory AATHIK_DIR_FOR_BACKUP exists or not
    if [ ! -d "$AATHIK_DIR_FOR_BACKUP" ]; then
        # If it does not exist we create the directory AATHIK_DIR_FOR_BACKUP  using mkdir function
        mkdir -p "$AATHIK_DIR_FOR_BACKUP"
    fi

    # Here we create use the date&time to create the .tar file name as per the question
    AATHIK_DATETIMEFUNCTION=$(date "+%Y%m%d%H%M%S")
    ((AATHIK_FULLBACKUPFILE++))
    AATHIK_TARFILENAME="cb$AATHIK_FULLBACKUPFILE.tar"
    # Here we use find to get all the files with .txt extension and pipes it to tar command which creates a tar file for all the .txt files in the respective directory
    find $HOME -type f -name "*.txt" -print0 | tar -cvf $AATHIK_DIR_FOR_BACKUP/$AATHIK_TARFILENAME --null -T - > /dev/null 2>&1

    # Here we update the destination directory for the latest/most recent backup in AATHIK_DEST_DIR_FOR_LATEST_BACKUP
    AATHIK_DEST_DIR_FOR_LATEST_BACKUP="$AATHIK_DIR_FOR_BACKUP/$AATHIK_TARFILENAME"

    # And we update the below message into the BackupLog file
    echo "$(date +"%a %d %b %Y %I:%M:%S %p %Z") $AATHIK_TARFILENAME was created" >> $AATHIK_DEST_DIR_FOR_BACKLOG/backup.log
}

# This is the infinite loop that we use to make the functions aathik_FullBackUp_Function and aathik_IncrementalBackUp_Function run continuously
while true; do
    #Here we check whether to do FullBackup or IncrementalBackup using modulus operator
    if [ $(expr $AATHIK_FULLORINC % 4) -eq 0 ]; then
        # aathik_FullBackUp_Function:- This is to do the full backup
        aathik_FullBackUp_Function
    else 
        # aathik_IncrementalBackUp_Function:- This is to do the incremental backup
        aathik_IncrementalBackUp_Function
    fi
    #Here we increment the AATHIK_FULLORINC after each iteration
    ((AATHIK_FULLORINC++))
    # Here we make the process sleep for 120 seconds which is equal to 2 minutes
    sleep 120
    # This makes the script to continue running even if the terminal or session that started it is closed.
done &
