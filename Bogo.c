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

void create_symboliclink(const char* path) {
    char link_name[100];

    printf("Enter a name for the symbolic link: ");
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

    if (symlink(path, link_name) == -1) {
        perror("Failed to create symbolic link");
        return;
    }
    printf("Symbolic link created successfully\n");
}

void print_error_message(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
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

void execute_regular_file(char option, const char* path) {
    struct stat sb;
    if (stat(path, &sb) == -1) {
        perror("Failed to get file information");
    } else {
        switch (option) {
        case 'n':
            printf("Name (-n): %s\n", path);
            break;
        case 'd':
            printf("Size (-d): %ld\n", sb.st_size);
            break;
        case 'h':
            printf("Hard link count (-h): %ld\n", sb.st_nlink);
            break;
        case 'm':
            printf("Last modification time(-m): %s", ctime(&sb.st_mtime));
            break;
        case 'a':
            printf("Access rights (-a):\n");
            access_rights(sb.st_mode);
            break;
        case 'l': {
            char link_name[100];
            printf("Enter a name for the symbolic link: ");
            if (fgets(link_name, sizeof(link_name), stdin) == NULL) {
                print_error_message("Failed to read input");
                break;
            }
            link_name[strcspn(link_name, "\n")] = '\0';
            create_symbolic_link(path, link_name);
            break;
        }
        default:
            printf("Invalid option: %c\n", option);
            break;
        }
    }
}

void execute_directory_option(char option, const char* path) {
    struct dirent *dp;
    struct stat sb;
    DIR *dir = opendir(path);
    int c_count = 0;

    if (!dir) {
        perror("Can't open directory");
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        char file_path[PATH_MAX];
        snprintf(file_path, PATH_MAX, "%s/%s", path, dp->d_name);

        if (lstat(file_path, &sb) == -1) {
            perror("Can't get file stats for file in directory.");
            continue;
        }

        switch (option) {
            case 'n':
                printf("%s\n", dp->d_name);
                break;

            case 'd':
                printf("%ld\n", sb.st_size);
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
                printf("Invalid option: %c\n", option);
                break;
        }
    }

    if (option == 'c') {
        printf("Total .c files: %d\n", c_count);
    }

    closedir(dir);
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
            printf("%s\n", path);
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
            printf("%ld\n", target_size);
            break;

        case 't':
            // Size of file
            if (!S_ISLNK(sb.st_mode)) {
                printf("Not a symbolic link\n");
                return;
            }
            if (lstat(target_path, &sb) == -1) {
                perror("lstat");
                return;
            }
            printf("%ld\n", sb.st_size);
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

void display_regular_file_menu(char* path) {
    printf("Options:\n");
    printf("-n: display name of file\n");
    printf("-h: display hard link count\n");
    printf("-d: display file size\n");
    printf("-m: display time of last modification\n");
    printf("-a: display access rights\n");
    printf("-l: create symbolic link\n");
    printf("Enter options as a single string (e.g., -nhd): ");
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

void display_directory_menu(char* path) {
    printf("Options:\n");
    printf("-n: display name of file\n");
    printf("-d: display file size\n");
    printf("-a: display access rights\n");
    printf("-c: number of files with .c extension\n");
    printf("Enter options as a single string (e.g., -nhd): ");
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
void display_symboliclink_menu(char* path) {
    printf("Options:\n");
    printf("-n: display name of file\n");
    printf("-d: display file size\n");
    printf("-a: display access rights\n");
    printf("-c: number of files with .c extension\n");
    printf("Enter options as a single string (e.g., -nhd): ");
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

void display_file_info(char* path) {
    struct stat sb;
    if (stat(path, &sb) == -1) {
        print_error_message("Failed to get file information");
        return;
    }
    switch (sb.st_mode & S_IFMT) {
        case S_IFREG: // regular file
            printf("Regular File\n");
			display_regular_file_menu(path);
                if(strstr(path,".c")!=NULL)
            {  pid_t child_a;
    // Fork a child process
    child_a = fork();

    if (child_a < 0) {
        // Error occurred
        fprintf(stderr, "can't create child\n");
        return;
    } else if (child_a == 0) {
        // Child process - logic here
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
            while((chr==fgetc(file))!=EOF){
                if(chr=='\n'){
                    lineCnt++;
                }
            } fclose(file);
            printf("nr of lines:%d\n",lineCnt);
        } break;

            case S_IFDIR: //directory
            printf("Directory\n");
        display_directory_menu(path);
            pid_t child_b;
            if(child_b<0){
                fprintf(stderr,"can't create child\n");
                return;
            } else if(child_b == 0){
                //child process
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
            pid_t child_c = fork();
            if(child_c<0){
                fprintf(stderr,"can't create child\n");
                return;
            }else if(child_c == 0){
                //child process
                exit(0);
            }
            //parent process
            waitpid(child_c,&status,0)
                if(WIFEXITED(status)){
                    int exit_c=WEXITSTATUS(status);
                    printf("Process %d ended with code %d\n",child_c,exit_c);  
            }    break;
            default:
            printf("unknown file type\n");
    }
    }
int main(int argc, char* argv[]) {
 display_file_info(argv[1]);
    return 0;
}
