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

void create_symboliclink(char* path) {
    char link_name[100];
    printf("Enter a name for the symbolic link: ");
    if (fgets(link_name, sizeof(link_name), stdin) == NULL) {
        fprintf(stderr, "Error: Failed to read input\n");
        return;
    }
    // Remove newline character from link_name if present
    link_name[strcspn(link_name, "\n")] = '\0';
    if (symlink(path, link_name) == -1) {
        fprintf(stderr, "Error: Failed to create symbolic link\n");
        return;
    }
    printf("Symbolic link created successfully\n");
}
void print_error_message(char* message) {
    fprintf(stderr, "Error: %s\n", message);
}

void execute_regular_file(char option, char* path) {
    struct stat sb;
    if (stat(path, &sb) == -1) {
        perror("no file information");
    }
    switch (option) {
    case 'n' : printf("Name (-n): %s\n", path);
    break;
	case 'd' : printf("Size (-d): %ld\n", sb.st_size);
    break;
	case 'h' : printf("Hard link count (-h): %ld\n", sb.st_nlink);
    break;
	case 'm' : printf("last modification time(-m): %s", ctime(&sb.st_mtime));
    break;
	case 'a' : printf("Access rights (-a):\n");
		access_rights(sb.st_mode);
	break;
	case 'l' : printf("Create symbolic link (-l): ");
	    create_symboliclink(path);
	break;
	default: break;   
    }
}

void execute_directory_option(char option, char* path) {
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
            printf("File type: regular file\n");
			
            //fork here
            {int fd1[2]; // store ends of first pipe
             int fd2[2]; // store ends of second pipe
             char fixed_str[] = " Test drive";
            char input_str[100];
                pid_t child_a, child_b;
    // Fork a child process
    if (pipe(fd1) == -1) {
        fprintf(stderr, "Pipe Failed");
       exit(EXIT_FAILURE);
    }
    if (pipe(fd2) == -1) {
        fprintf(stderr, "Pipe Failed");
        exit(EXIT_FAILURE);
    }
    child_a = fork();

    if (child_a < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (child_a == 0) {
        // Child process - logic here
   //     printf("                 This is the child process\n");
        close(fd1[1]); // Close writing end of first pipe
 
        // Read a string using first pipe
        char concat_str[100];
        read(fd1[0], concat_str, 100);
 
        // Concatenate a fixed string with it
        int k = strlen(concat_str);
        int i;
        for (i = 0; i < strlen(fixed_str); i++)
            concat_str[k++] = fixed_str[i];
 
        concat_str[k] = '\0'; // string ends with '\0'
 
        // Close both reading ends
        close(fd1[0]);
        close(fd2[0]);
 
        // Write concatenated string and close writing end
        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]);
 
        exit(0);

        exit(EXIT_SUCCESS);
    } else {
        child_b=fork();
            if(child_b==0){
                //2nd child process
        //        printf("              2nd child process \n");
        close(fd1[1]);
        close(fd1[0]);
        close(fd2[0]);
        close(fd2[1]);
            }else{
                char concat_str[100];
 
        close(fd1[0]); // Close reading end of first pipe
 
        // Write input string and close writing end of first
        // pipe.
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]);
 
        // Wait for child to send a string
        //wait(NULL);
 
        close(fd2[1]); // Close writing end of second pipe
 
        // Read string from child, print it and close
        // reading end.
        read(fd2[0], concat_str, 100);
        printf("string %s\n", concat_str);
        close(fd2[0]);
    }
        // Parent process
       // printf("                This is the parent process\n");
        // Add parent process logic here
        exit(EXIT_SUCCESS);
    }
}
            display_regular_file_menu(path);
			//2nd process if here
            
			//if check for .c extension
			//execute .c program
			//get back error number
			//else write number of lines
			//if .c extension compute score based on error and warning number
            break;
        case S_IFDIR: // directory
            printf("File type: directory\n");
			//fork here
            {pid_t pid;

    // Fork a child process
    pid = fork();

    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
       // printf("This is the child process\n");
        // Add child process logic here
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        //printf("This is the parent process\n");
        // Add parent process logic here
        exit(EXIT_SUCCESS);
    }

}
            display_directory_menu(path);
			//2nd process if here
			//Create  <dir_name>_file.txt
			//wait for process to end
            break;
        case S_IFLNK: // symbolic link
            printf("File type: symbolic link\n");
			//fork here
            pid_t pid;

    // Fork a child process
    pid = fork();

    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
       // printf("child process\n");
        // Add child process logic here
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        //printf("parent process\n");
        // Add parent process logic here
        exit(EXIT_SUCCESS);
    }

}
            display_symboliclink_menu(path);
			//2nd process if here
			//wait for process to end here
           // break;
        //default:
            printf("File type: unknown\n");
    }

int main(int argc, char* argv[]) {
 display_file_info(argv[1]);
    return 0;
}
