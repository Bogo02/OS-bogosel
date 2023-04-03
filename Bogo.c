/*Write a C program that recieves as parameters multiple arguments representing paths to regular files, directories and symbolic links.
Depending on the file type(rf,d,sl) print an interactive menu:
A)regular file: -n(file name)
-d(dim/size)
-h(nr of hard links)
-m(time of last modification)
-a(acces rights)
-l(create sym.link give:link name)
B)symbolic link: -n(link name)
-l(delete link)
-d(size of the link)
-t(size of the target)
-a(access rights)
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

void print_regular_file_info(const char* path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("stat");
        return;
    }

    printf("File name: %s\n", path);
    printf("Size: %ld bytes\n", st.st_size);
    printf("Number of hard links: %ld\n", st.st_nlink);
    printf("Time of last modification: %ld\n", st.st_mtime);
    printf("Access rights: %o\n", st.st_mode & 0777);
    
    char link_name[256];
    printf("Enter link name (max 256 characters): ");
    scanf("%s", link_name);
    
    if (symlink(path, link_name) == -1) {
        perror("symlink");
        return;
    }
    printf("Symbolic link created: %s -> %s\n", link_name, path);
}

void print_symbolic_link_info(const char* path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror("lstat");
        return;
    }

    printf("Link name: %s\n", path);
    printf("Size of the link: %ld bytes\n", st.st_size);
    printf("Size of the target: %ld bytes\n", st.st_blocks * 512);
    printf("Access rights: %o\n", st.st_mode & 0777);

    char link_name[256];
    printf("Enter link name to delete (max 256 characters): ");
    scanf("%s", link_name);

    if (unlink(link_name) == -1) {
        perror("unlink");
        return;
    }
    printf("Symbolic link deleted: %s\n", link_name);
}

void print_directory_info(const char* path) {
    printf("Directory name: %s\n", path);
}

void print_file_info(const char* path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("stat");
        return;
    }

    switch (st.st_mode & S_IFMT) {
        case S_IFREG:
            print_regular_file_info(path);
            break;
        case S_IFLNK:
            print_symbolic_link_info(path);
            break;
        case S_IFDIR:
            print_directory_info(path);
            break;
        default:
            printf("Unknown file type: %s\n", path);
            break;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s file1 [file2 ...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return 0;
}
