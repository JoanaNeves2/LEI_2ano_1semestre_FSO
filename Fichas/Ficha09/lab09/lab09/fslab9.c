/*
 ============================================================================
 Name        : fslab9.c
 Author      : vad
 Version     :
 Copyright   : FSO 17/18 - 2023 DI-FCT/UNL
 Description : Lab 09 - sistema de ficheiros
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "fs.h"
#include "disk.h"

#define STRSZ  1024   // max string size

/**
 * do_copyout - read contents of a file, print it to stdout
 * 		uses fs_open to get file id (dirent/inode number) and then fs_read to read its contents
 */
static int do_copyout(char *fsname) {
	int offset = 0, result, fid;
	char buffer[16384];

	fid = fs_open(fsname);
	while (1) {
		result = fs_read(fid, buffer, sizeof(buffer), offset);
		if (result <= 0)
			break;
		fwrite(buffer, 1, result, stdout);
		offset += result;
	}

	printf("%d bytes copied\n", offset);
	return 1;
}


/**
 * MAIN 
 * just a shell to work as a test program for the file system
 */
int main(int argc, char*argv[]) {
	char line[STRSZ];
	char cmd[STRSZ];
	char arg1[STRSZ];
	char arg2[STRSZ];
	int args, nblocks;

	if (argc != 3 && argc != 2) {
		printf("use: %s <diskfile> [nblocks]\n", argv[0]);
		return 1;
	}
	if (argc == 3)
		nblocks = atoi(argv[2]);
	else
		nblocks = -1;

	if (!disk_init(argv[1], nblocks)) {
		printf("couldn't initialize %s: %s\n", argv[1], strerror(errno));
		return 1;
	}

	printf("opened emulated disk image %s with %d blocks\n", argv[1],
			disk_size());

	while (1) {
		printf("fslab09> ");
		fflush(stdout);

		if (!fgets(line, sizeof(line), stdin))
			break;
		if (line[0] == '\n')  // empty line
			continue;
		line[strlen(line) - 1] = '\0';

		args = sscanf(line, "%s %s %s", cmd, arg1, arg2);
		if (args == 0)
			continue;

		if (!strcmp(cmd, "format")) {
			if (args == 1)
				if (fs_format())
					printf("disk formatted.\n");
				else
					printf("format failed!\n");
			else
				printf("use: format\n");
		} else if (!strcmp(cmd, "mount")) {
			if (args == 1) {
				if (fs_mount()) {
					printf("disk mounted.\n");
				} else {
					printf("mount failed!\n");
				}
			} else {
				printf("use: mount\n");
			}
		} else if (!strcmp(cmd, "debug")) {
			if (args == 1) {
				fs_debug();
			} else {
				printf("use: debug\n");
			}
		} else if (!strcmp(cmd, "cat")) {
			if (args == 2) {
				if (!do_copyout(arg1)) {
					printf("cat failed!\n");
				}
			} else {
				printf("use: cat <fsname>\n");
			}
		
		} else if (!strcmp(cmd, "help") || !strcmp(cmd, "?")) {
			printf("Commands are:\n");
			printf("    format\n");
			printf("    mount\n");
			printf("    debug\n");
			printf("    cat <fsname>\n");
			printf("    help\n");
			printf("    quit\n");
			printf("    exit\n");
		} else if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit")) {
			break;
		} else {
			printf("unknown command: %s\n", cmd);
			printf("type 'help' for a list of commands.\n");
		}
	}

	printf("closing emulated disk.\n");
	disk_close();

	return EXIT_SUCCESS;
}




