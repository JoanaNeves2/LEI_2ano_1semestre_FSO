#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>


void printFileInfo(const char *name, struct stat *fileStat) {
    struct passwd *pwd = getpwuid(fileStat->st_uid);
    struct group *grp = getgrgid(fileStat->st_gid);
    struct tm *timeInfo = localtime(&fileStat->st_mtime);

    printf("%ld: %s %02d:%02d %s ",
        (long)fileStat->st_ino, pwd->pw_name, timeInfo->tm_hour, timeInfo->tm_min, name);

    if (S_ISDIR(fileStat->st_mode)) {
        printf("(dir)\n");
    } else if (S_ISREG(fileStat->st_mode)) {
        printf("(%ld)\n", (long)fileStat->st_size);
    } else {
        printf("(other)\n");
    }
}

void listDirectoryContents(const char *dirPath) {
    DIR *directory = opendir(dirPath);
    if (directory == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat fileStat;

    while ((entry = readdir(directory)) != NULL) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);

        if (lstat(path, &fileStat) == -1) {
            perror("lstat");
            continue;
        }

        printFileInfo(entry->d_name, &fileStat);

        if (S_ISDIR(fileStat.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("\n%s:\n", path);
            listDirectoryContents(path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *dirPath;
    
    if (argc > 1) {
        dirPath = argv[1];
    } else {
        dirPath = ".";
    }

    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    struct stat fileStat;

    while ((entry = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);

        if (lstat(path, &fileStat) == -1) {
            perror("lstat");
            continue;
        }

        printFileInfo(entry->d_name, &fileStat);
        
        if (S_ISDIR(fileStat.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // If it's a directory, and not '.' or '..', list its contents.
            printf("\n%s:\n", path);
            listDirectoryContents(path);
        }
    }

    closedir(dir);
    return 0;
}


