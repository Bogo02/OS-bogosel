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
#include <errno.h>
int main(){

    printf("Hello");
    return 0;
}