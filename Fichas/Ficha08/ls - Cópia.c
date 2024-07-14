#include <dirent.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    const char *dirname;
    struct dirent *dp;

    if(argc>1){
        dirname = argv[1];
    }
    else{
        dirname = '.';
    }

    DIR *dir = opendir(dirname);
    if(dir == NULL){
        perror("opendir");
        exit(1);
    }

    while((dp = readdir(dir)) != NULL){
    }


}