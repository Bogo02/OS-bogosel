#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <time.h>
#include <wait.h>

void script_exec(const char* filename){
FILE *fp=fopen(filename,"r");
if (fp == NULL) {
        fprintf(stderr, "Failed to execute script.\n");
        exit(1);
    }
char output[1000];
    if (fgets(output, sizeof(output), fp) != NULL) {
        int nrErrors = 0;
        int nrWarnings = 0;
        // count errors and warnings
        char* errLine = strstr(output, "error:");
        char* warningLine = strstr(output, "warning:");
        while (errLine != NULL) {
            nrErrors++;
            errLine = strstr(errLine + 1, "error:");
        }
        while (warningLine != NULL) {
            nrWarnings++;
            warningLine = strstr(warningLine + 1, "warning:");
        }
        // Compute the score based on the number of errors and warnings
        int score;
        if (nrErrors == 0 && nrWarnings == 0)
            score = 10;
        else if (nrErrors > 0)
            score = 1;
        else if (nrWarnings > 10)
            score = 2;
        else
            score = 2 + 8 * (10 - nrWarnings) / 10;
           //printf("%s:%d\n",filename,nrWarnings);
           //printf("%s: %d\n",filename,nrErrors);  
       printf("%s: %d\n",filename,score);
    FILE* ScoreFile = fopen("score.txt","w");
        if(ScoreFile==NULL){
            fprintf(stderr,"failed to open score.txt to write\n");
            exit(1);
        }
        fprintf(ScoreFile,"%s: %d\n",filename,score);
        fclose(ScoreFile);}

}

void access_rights(mode_t mode) {
    printf("User:\n");
    printf("Read - %s\n", (mode & S_IRUSR) ? "yes" : "no");
    printf("Write - %s\n", (mode & S_IWUSR) ? "yes" : "no");
    printf("Exec - %s\n", (mode & S_IXUSR) ? "yes" : "no");
    printf("Group:\n");
    printf("Read - %s\n", (mode & S_IRGRP) ? "yes" : "no");
    printf("Write - %s\n", (mode & S_IWGRP) ? "yes" : "no");
    printf("Exec - %s\n", (mode & S_IXGRP) ? "yes" : "no");
    printf("Others:\n");
    printf("Read - %s\n", (mode & S_IROTH) ? "yes" : "no");
    printf("Write - %s\n", (mode & S_IWOTH) ? "yes" : "no");
    printf("Exec - %s\n", (mode & S_IXOTH) ? "yes" : "no");
}
//function creates a symbolic link to file specified by the program 
void create_symboliclink(const char* path) {
    char link_name[100];
    //promts user for a name for the symb. link
    printf("Enter a name for the symbolic link: ");
    //reads input from user
    if (fgets(link_name, sizeof(link_name), stdin) == NULL) {
        perror("Failed to read input");
        return;
    }
    link_name[strcspn(link_name, "\n")] = '\0';

    // Check if link name is empty
    if (link_name[0] == '\0') {
        perror("Link name cannot be empty");
        return;
    }

    // Check if link name already exists
    if (access(link_name, F_OK) == 0) {
        perror("Link name already exists");
        return;
    }
//create the symbolic link using the path and link name
    if (symlink(path, link_name) == -1) {
        perror("Failed to create symbolic link");
        return;
    }
    printf("Symbolic link created successfully\n");
}

int create_symbolic_link(const char* path, const char* link_name) {

    // Check if link name is empty
    if (link_name[0] == '\0') {
        perror("Link name cannot be empty");
        return -1;
    }

    // Check if link name already exists
    if (access(link_name, F_OK) == 0) {
        perror("Link name already exists");
        return -1;
    }

    if (symlink(path, link_name) == -1) {
        perror("Failed to create symbolic link");
        return -1;
    }
    printf("Symbolic link created successfully\n");
    return 0;
}

void execute_symboliclink_option(char option, char* path) {
    struct stat sb;
    char target_path[PATH_MAX];
    ssize_t target_size;
    int del_flag = 0;

    if (lstat(path, &sb) == -1) {
        perror("lstat");
        return;
    }

    if (S_ISLNK(sb.st_mode)) {
        // Get path/ size if symbolic link
        target_size = readlink(path, target_path, PATH_MAX - 1);
        if (target_size == -1) {
            perror("readlink");
            return;
        }
        target_path[target_size] = '\0';
    }

    switch (option) {
        case 'n':
            printf("\n%s\n", path);
            break;

        case 'l':
            // Delete symbolic link
            if (unlink(path) == -1) {
                perror("unlink");
            }
            del_flag = 1;
            break;

        case 'd':
            // Size of symbolic link
            if (!S_ISLNK(sb.st_mode)) {
                printf("Not a symbolic link\n");
                return;
            }
            printf("\n%ld\n", target_size);
            break;

        case 't':
            // Size of file
            if (!S_ISLNK(sb.st_mode)) {
                printf("\nNot a symbolic link\n");
                return;
            }
            if (lstat(target_path, &sb) == -1) {
                perror("\nlstat");
                return;
            }
            printf("\n%ld\n", sb.st_size);
            break;

        case 'a':
            // Access rights
            access_rights(sb.st_mode);
            break;

        default:
            printf("Invalid option: %c\n", option);
            break;
    }

    if (!del_flag && option != '\0') {
        printf("Additional options not supported\n");
    }
}

void print_error_message(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
}

void display_symboliclink_menu(char* path) {
    printf("Options:\n");
    printf("-n: display name of file\n");
    printf("-d: display file size\n");
    printf("-a: display access rights\n");
    printf("-c: number of files with .c extension\n");
    printf("Enter options as a single string (e.g., -nhd): ");
    printf("\n");
    char options[10];
    fgets(options, sizeof(options), stdin);
    options[strcspn(options, "\n")] = '\0'; // remove newline char
    int i;
    int check = 1;
    char option;
    for(i = 1; i <strlen(options); i++) {
        option = options[i];
        if(!(strchr("ndac", option))){
            check = 0;
            break;
        }
    }
    
    if(check == 1){
    for (i = 1; i < strlen(options); i++) {
        option = options[i];
        execute_symboliclink_option(option, path);
    }
     
     }else {
            print_error_message("Invalid option");
            display_symboliclink_menu(path);
            return;
        }
    }

void execute_regular_file(char option, const char* path) {
    struct stat sb;
    if (stat(path, &sb) == -1) {
        perror("Failed to get file information");
    } else {
        switch (option) {
        case 'n':
            printf("\nName (-n): %s\n", path);
            break;
        case 'd':
            printf("\nSize (-d): %ld\n", sb.st_size);
            break;
        case 'h':
            printf("\nHard link count (-h): %ld\n", sb.st_nlink);
            break;
        case 'm':
            printf("\nLast modification time(-m): %s", ctime(&sb.st_mtime));
            break;
        case 'a':
            printf("\nAccess rights (-a):\n");
            access_rights(sb.st_mode);
            break;
        case 'l': {
            char link_name[100];
            printf("\nEnter a name for the symbolic link: ");
            if (fgets(link_name, sizeof(link_name), stdin) == NULL) {
                print_error_message("Failed to read input");
                break;
            }
            link_name[strcspn(link_name, "\n")] = '\0';
            create_symbolic_link(path, link_name);
            break;
        }
        default:
            printf("\nInvalid option: %c\n", option);
            break;
        }
    }
}

void display_regular_file_menu(char* path) {
    printf("Options:\n");
    printf("-n: display name of file\n");
    printf("-h: display hard link count\n");
    printf("-d: display file size\n");
    printf("-m: display time of last modification\n");
    printf("-a: display access rights\n");
    printf("-l: create symbolic link\n");
    printf("Enter options as a single string (e.g., -nhd): ");
    //printf("\n");
    char options[10];
    fgets(options, sizeof(options), stdin);
    options[strcspn(options, "\n")] = '\0'; // remove newline char
    int i;
    int check = 1;
    char option;
    for(i = 1; i <strlen(options); i++) {
        option = options[i];
        if(!(strchr("nhdmal", option))){
            check = 0;
            break;
        }
    }
    
    if(check == 1){
    for (i = 1; i < strlen(options); i++) {
        option = options[i];
        execute_regular_file(option, path);
    }
     
     }else {
            print_error_message("Invalid option");
            display_regular_file_menu(path);
            return;
        }
    }

void execute_directory_option(char option, const char* path) {
    struct dirent *dp;
    struct stat sb;
    DIR *dir = opendir(path);
    int c_count = 0;

    if (!dir) {
        perror("\nCan't open directory");
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        char file_path[PATH_MAX];
        snprintf(file_path, PATH_MAX, "%s/%s", path, dp->d_name);

        if (lstat(file_path, &sb) == -1) {
            perror("\nCan't get file stats for file in directory.");
            continue;
        }

        switch (option) {
            case 'n':
                printf("\n%s\n", dp->d_name);
                break;

            case 'd':
                printf("\n%ld\n", sb.st_size);
                break;

            case 'a':
                access_rights(sb.st_mode);
                break;

            case 'c':
                if (S_ISREG(sb.st_mode) && strstr(dp->d_name, ".c") != NULL) {
                    c_count++;
                }
                break;

            default:
                printf("\nInvalid option: %c\n", option);
                break;
        }
    }

    if (option == 'c') {
        printf("Total .c files: %d\n", c_count);
    }

    closedir(dir);
}

void display_directory_menu(char* path) {
    printf("Options:\n");
    printf("-n: display name of file\n");
    printf("-d: display file size\n");
    printf("-a: display access rights\n");
    printf("-c: number of files with .c extension\n");
    printf("Enter options as a single string (e.g., -nhd): ");
    printf("\n");
    char options[10];
    fgets(options, sizeof(options), stdin);
    options[strcspn(options, "\n")] = '\0'; // remove newline character
    int i;
    int check = 1;
    char option;
    for(i = 1; i <strlen(options); i++) {
        option = options[i];
        if(!(strchr("ndac", option))){
            check = 0;
            break;
        }
    }
    if(check == 1){
    for (i = 1; i < strlen(options); i++) {
        option = options[i];
        execute_directory_option(option, path);
    }
     }else {
            print_error_message("Invalid option");
            display_directory_menu(path);
            return;
        }
}

void display_file_info(char* path) {
    struct stat sb;
    if (stat(path, &sb) == -1) {
        print_error_message("Failed to get file information");
        return;
    }
    int status;
    pid_t child_a,child_b,child_c;
    switch (sb.st_mode & S_IFMT) {
        case S_IFREG: // regular file
            printf("Regular File\n");
			display_regular_file_menu(path);
                if(strstr(path,".c")!=NULL){
            // Fork a child process
            child_a = fork();
    if (child_a < 0) {
        // Error occurred
        fprintf(stderr, "can't create child\n");
        return;
    } else if (child_a == 0) {
        // Child process
        script_exec(path);
        exit(0);
    }  //parent
        waitpid(child_a,&status,0);
        if(WIFEXITED(status)){
            int exit_c=WEXITSTATUS(status);
            printf("process %d ended with code %d\n",child_a, exit_c);
        }
        }else { //nr of lines
            FILE* file=fopen(path,"r");
            if(file==NULL){
                fprintf(stderr,"no file\n");
                return;
            }
            int lineCnt=0;
            char chr;
            while((chr=fgetc(file)) != EOF){
                if(chr=='\n'){
                    lineCnt++;
                }
            } fclose(file);
            printf("nr of lines:%d\n",lineCnt);
        } break;

            case S_IFDIR: //directory
            printf("Directory\n");
        display_directory_menu(path);
        child_b=fork();
            if(child_b<0){
                fprintf(stderr,"can't create child\n");
                return;
            } else if(child_b == 0){
                //child process
               /* char filename[50];
                sprintf(filename, "%s_file.txt",path);
                char cmd[50];
                sprintf(filename,"touch %s",filename);
                system(cmd);*/
                script_exec(path);
                //I did not manage to succsessfully work with the symbolic links
                //I got errors that I did not manage to fix
                exit(0);
            }
            //parent process
            waitpid(child_b, &status,0);
            if(WIFEXITED(status)){
                int exit_c=WEXITSTATUS(status);
                printf("Process %d ended with code %d\n",child_b,exit_c);
            } break;
            case S_IFLNK: //symbolic link
            printf("Symbolic Link\n");
            display_symboliclink_menu(path);
                child_c=fork();
            if(child_c<0){
                fprintf(stderr,"can't create child\n");
                return;
            }else if(child_c == 0){
                //child process
            char cmd[200];
            sprintf(cmd,"%s",path);
            system(cmd);
                exit(0);
            }
            //parent process
            waitpid(child_c,&status,0);
                if(WIFEXITED(status)){
                    int exit_c=WEXITSTATUS(status);
                    printf("Process %d ended with code %d\n",child_c,exit_c);  
            }    break;
            default:
            printf("unknown file type\n");
    }
}
  
  
 int main(int argc, char* argv[]) {
    //wanted to add more comments, ran out of time
      display_file_info(argv[1]);
    int fd[2];
    if (pipe(fd) == -1) {
        fprintf(stderr, "Failed to create pipe.\n");
        return 1;
    }

    pid_t child_pid = fork();
    if (child_pid < 0) {
        fprintf(stderr, "Failed to create child process.\n");
        return 1;
    } else if (child_pid == 0) {
        // Child process
        close(fd[0]); // Close the read end of the pipe
        dup2(fd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
        close(fd[1]); // Close the write end of the pipe

        execlp("ls", "ls", "-1", NULL); // Execute the ls -1 command
        fprintf(stderr, "Failed to execute ls -1.\n");
        exit(1);
    }

    // Parent process
    close(fd[1]); // Close the write end of the pipe

    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0) {
        printf("\n List of all possible files/directories in target directory:\n");
        fwrite(buffer, 1, bytes_read, stdout);
    }

    close(fd[0]); // Close the read end of the pipe

    int status;
    waitpid(child_pid, &status, 0);
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("The process with PID %d has ended with exit code %d.\n", child_pid, exit_code);
    }

    return 0;
}
