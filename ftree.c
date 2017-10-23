#include <stdio.h>
// Add your system includes here.
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "ftree.h"
#include "hash.h"

int cp(const char *from, const char *to) {

	FILE *source, *target;
	char buffer[4096];
	source = fopen(from, "rb");
	if (source == NULL) {
		printf("fopen read failed.\n");
		exit(EXIT_FAILURE);
	}
	target = fopen(to, "wb");

	if (target == NULL) {
		fclose(source);
		printf("fopen write failed.\n");
		exit(EXIT_FAILURE);
	}
	int num1, num2;
	while (feof(source) == 0) {
		if ((num1 = fread(buffer, 1, 100, source)) != 100) {
			if (ferror(source) != 0) {
				fprintf(stderr, "read file error.\n");
				exit(1);
			} else if (feof(source) != 0)
				;
		}
		if ((num2 = fwrite(buffer, 1, num1, target)) != num1) {
			fprintf(stderr, "write file error.\n");
			exit(1);
		}
	}

	fclose(source);
	fclose(target);

	return 0;
}

int copy_ftree(const char *src, const char *dest) {
	// Your implementation here.
//	int fd[2];
//	int depth = 0;
//	char *a = "xx";

	struct stat file_info, file_info1, file_info2;
	static int depth1 = 0;
	struct dirent *dp;
	int count = 0;

//	if ((pipe(fd)) == -1) {
//		perror("pipe");
//		exit(1);
//	}

	DIR *dirp = opendir(dest);
	if (dirp == NULL) {
		perror("opendir");
		exit(1);
	}
	dirp = opendir(src);
	if (dirp == NULL) {
		perror("opendir");
		exit(1);
	}

	if (!(dp = readdir(dirp))) {
		perror("readdir");
		return -1;
	}
	if (!strcmp(dp->d_name, ".") && depth1 == 0) {
		lstat(src, &file_info);
		char str[1024];
		int len = snprintf(str, sizeof(str) - 1, "%s/%s", dest, src);
		str[len] = 0;
		struct stat st = { 0 };

		if (stat(str, &st) == -1) {
			mkdir(str, file_info.st_mode);
		}
		depth1++;
		count += copy_ftree(src, str);
//		char other[1];
//		while (read(fd[0], other, 2) > 0) {
//			count++;
//		}

		return count + 1;
	}

	do {
		if (dp->d_type == DT_DIR) {

			char path1[1024];
			int len1 = snprintf(path1, sizeof(path1) - 1, "%s/%s", src,
					dp->d_name);
			path1[len1] = 0;

			if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
				continue;
			lstat(path1, &file_info);
			char path2[1024];
			int len2 = snprintf(path2, sizeof(path2) - 1, "%s/%s", dest,
					dp->d_name);
			path2[len2] = 0;

			struct stat st = { 0 };
			if (stat(path2, &st) == -1) {
				mkdir(path2, (file_info.st_mode & 0777));
			}

			int r = fork();
			if (r == -1) {
				perror("fork:");
				exit(1);
			} else if (r != 0) {
//				if ((close(fd[1])) == -1) {
//					perror("close");
//				}
				int status;
				if (waitpid(r, &status, 0) != -1) {
					if (WIFEXITED(status)) {
						if (WEXITSTATUS(status) > 0) {
							count = WEXITSTATUS(status);

						} else {
							count = 1;
						}
//						if (!WEXITSTATUS(status)) {
//						} else {
//							printf(
//									"Child %d exited not with 0 but with signal %d\n",
//									r, WEXITSTATUS(status));
//						}
					} else if (WIFSIGNALED(status)) /* killed */
					{
						printf("Child %d terminated with signal %d\n", r,
								WTERMSIG(status));
					} else if (WIFSTOPPED(status)) /* stopped */
					{
						printf("Child %d stopped with signal %d\n", r,
								WTERMSIG(status));
					} else {
						printf("[%d] Child exited abnormally\n", getpid());
					}

				}

			} else {
				//child
//				if ((close(fd[0])) == -1) {
//					perror("close");
//				}
//				if (write(fd[1], a, 2) == -1) {
//					perror("write to pipe");
//				}
				depth1++;
				count += copy_ftree(path1, path2);
				exit(count + 1);

			}

		} else {
			if (dp->d_type == DT_LNK
					|| (strncmp(dp->d_name, ".", strlen(".")) == 0)) {
				continue;
			}
			char path4[1024];
			int len4 = snprintf(path4, sizeof(path4) - 1, "%s/%s", dest,
					dp->d_name);
			path4[len4] = 0;
			char path5[1024];
			int len5 = snprintf(path5, sizeof(path5) - 1, "%s/%s", src,
					dp->d_name);
			path5[len5] = 0;
			FILE *f = fopen(path4, "r");
			if (f != NULL) {
				FILE *f1 = fopen(path5, "rb");
				if (f1 == NULL) {
					fprintf(stderr, "Error opening file\n");
				}
				int i2, i3;
				lstat(path4, &file_info1);
				lstat(path5, &file_info2);
				i2 = file_info1.st_size;
				i3 = file_info2.st_size;
				if (i2 == i3) {
					if (!strcmp(hash(f), hash(f1))) {
						if (file_info1.st_mode == file_info2.st_mode) {
							continue;
						} else {

							if (chmod(path4, file_info2.st_mode) < 0) {
								fprintf(stderr, "%s error in chmod %s - %m\n",
										path4, strerror(errno));

							}
							continue;
						}
					}

				}
				fclose(f1);
			}
			cp(path5, path4);
			lstat(path5, &file_info2);
			if (chmod(path4, file_info2.st_mode) < 0) {
				fprintf(stderr, "%s: error in chmod %s - %m\n", path4,
						strerror(errno));

			}
		}

	} while ((dp = readdir(dirp)) != NULL);
	closedir(dirp);
	return count;
}
