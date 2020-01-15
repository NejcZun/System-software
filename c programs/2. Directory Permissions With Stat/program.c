#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

void main(int argc, char *argv[]){
	DIR* dp;
	if((dp=opendir(argv[1])) == NULL){
		printf("Cant open %s", argv[1]);
		exit(1);
	}
	struct dirent *dirp;
	while((dirp = readdir(dp)) != NULL){
		struct stat fileStat;
		if(stat(dirp->d_name, &fileStat) < 0) exit(1);
		if(S_ISDIR(fileStat.st_mode)) printf("d");
		else if(S_ISREG(fileStat.st_mode)) printf("-");
		else if(S_ISCHR(fileStat.st_mode)) printf("c");
		else if(S_ISBLK(fileStat.st_mode)) printf("b");
		else if(S_ISFIFO(fileStat.st_mode)) printf("f");
		else if(S_ISLNK(fileStat.st_mode)) printf("l");
		else if(S_ISSOCK(fileStat.st_mode)) printf("s");
		printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
		printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
		if(fileStat.st_mode & S_IXUSR){ 
			if(fileStat.st_mode & S_ISUID) printf("S");
			else printf("x"); 
		} else if(fileStat.st_mode & S_ISUID) printf("s");
		else printf("-");
		printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
		printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
		if(fileStat.st_mode & S_IXGRP){ 
			if(fileStat.st_mode & S_ISGID)printf("S");
			else printf("x");
		}
		else if(fileStat.st_mode & S_ISGID) printf("s");
		else printf("-");
		printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
		printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
		if(fileStat.st_mode & S_IXOTH){ 
			if(fileStat.st_mode & S_ISVTX) printf("t");
			else printf("x");
		}
		else if(fileStat.st_mode & S_ISVTX) printf("T");
		else printf("-");
		printf(" %lu", fileStat.st_nlink);
		printf("    %s\n", dirp->d_name);
	}
	closedir(dp);
}