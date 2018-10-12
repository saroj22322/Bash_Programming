#ifndef EOF
#include <stdio.h>
#endif
#ifndef EXIT_SUCCESS
#include <stdlib.h>
#endif
#include <string.h>
#include <errno.h>
#include <utime.h>
#include <dirent.h>
#include <time.h>

#ifdef __unix__
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define defMod 0711
#define OS 0
#elif __WIN32__ || _MS_DOS_
#include <dir.h>
#include <sys\stat.h>
#include <io.h>
#define OS 1
#else
#include <direct.h>
#define OS 2
#endif

enum State {SUCCESS = 0, FAIL = -1};

int argCheck(int, char**, int*);
void printErr(char *);
void syntaxErr(char *);

int c(char *, char* );
int rm(char *, char* );
int inf(char *, char* );
int del(char *, char* );
int cp(char*,char*,char*);
int mv(char*,char*,char*);
int rn(char*,char*,char*);
int l(char*, char*);
int ch(char*, char*);
int edit(char*, char*);

int oneCaller(int, char **, int);
int twoCaller(int, char **, int);
int multiCaller(int, char **, int);


int main(int argc, char* argv[]) {
	int ind = 0;
	char err[BUFSIZ];
	memset(err, '\0',BUFSIZ);
	if(argc == 1) {
		char *sar[4] = {"fd", "-l","."};
		oneCaller(argc, sar, 0);
		return EXIT_SUCCESS;
	}
	switch(argCheck(argc, argv, &ind)) {
			break;
		case 0:
			oneCaller(argc, argv, ind);
			break;
		case 1:
			twoCaller(argc,argv, ind);
			break;
		case 2:
			multiCaller(argc,argv, ind);
			break;
		default:
			syntaxErr("all");		
	}	
	return EXIT_SUCCESS;	
}

void syntaxErr(char *hint) {
	char* ind[] = {"-l","-ch","-edit","-cp","-mv","-rn","-c","-rm","-inf","-del"};
	char* err[] = {
		"Syntax Error : fd  OR  fd -l  OR  fd -l [directoryname]",
		"Syntax Error : fd -ch [directoryname]",
		"Syntax Error : fd -edit [filename]",
		"Syntax Error : fd -cp [source] [destination]",
		"Syntax Error : fd -mv [source] [destination]",
		"Syntax Error : fd -rn [oldname] [newname]",
		"Syntax Error : fd -c [directory1] [directory2] ...",
		"Syntax Error : fd -rm [diretory1] [directory2] ...",
		"Syntax Error : fd -inf [dataname1] [dataname2] ...",
		"Syntax Error : fd -del [dataname1] [dataname2] ..."
	};
	for (int i = 0; i < 10; ++i)
	{
		if(strcmp(hint, ind[i]) == 0) {
			fprintf(stderr,"\n%s\n\n", err[i]);
		}
	}

	if(strcmp(hint, "all") == 0) {
		fprintf(stdout, "\n\t Help for 'fd' command : \n"\
			"\t Syntax : fd -[FLAG] [OPTIONS] ...\n\n"\
			"\t  Flags : Command\n"\
			"\t -l	: List the files and folder one given directory\n"\
			"\t -ch 	: Change the working directory\n"\
			"\t -edit	: Create or edit a data\n"\
			"\t -cp	: Copy a data from source to destination\n"\
			"\t -mv	: Move a data from source to destination\n"\
			"\t -rn	: Rename a data\n"\
			"\t -c 	: Create directory\n"\
			"\t -r 	: Remove directory\n"\
			"\t -inf 	: Show the property of data\n"\
			"\t -del 	: Delete given data\n\n");
	}
}


int argCheck(int argc, char* argv[], int* indP) {
	int found = -1;
	char *indxs[4][5] = {{"l","ch","edit",NULL},{"cp","mv","rn",NULL},{"c","rm","inf","del"}};
	if(argv[1][0] != '-' || argv[1][1] == '\0') {
		return found;
	}
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if(indxs[i][j] == NULL) continue;
			if(strcmp(&argv[1][1], indxs[i][j]) == 0) {
				*indP = j;
				found = i;
				break;
			}
		}
		if(found != -1) break;
	}
	return found;
}

void printErr(char *err) {
	if (errno)
	{
		strcat(err, " - ");
		strcat(err, strerror(errno));
		strcat(err, "\n");
	}
	fprintf(stderr, "\n");
	fprintf(stderr, err);
	fprintf(stderr, "\n");
	memset(err, '\0',BUFSIZ);
	errno = 0;
}

//Calls the functions taking 1 argument.
int oneCaller(int count, char **names, int ind) {
	if(strcmp(names[1],"-l") != 0) {
		if(count != 3) {
			syntaxErr(names[1]);
			return FAIL;
		}
	} else {
		if(count > 3) {
			syntaxErr(names[1]);
			return FAIL;
		}
	}	
	int (*oCaller[])(char*, char*) = {l,ch,edit};
	char err[BUFSIZ];
	if((*oCaller[ind])(names[2], err) == SUCCESS) {
		errno = 0;
		return SUCCESS;

	} else {
		printErr(err);
		return FAIL;
	}
}

// Calls the functions taking 2 arguments.
int twoCaller(int count, char **names, int ind) {
	if(count != 4) {
		syntaxErr(names[1]);
		return FAIL;
	}
	int (*tCall[]) (char*, char*, char*) = {cp, mv, rn};
	char err[BUFSIZ];
	if((*tCall[ind])(names[2], names[3], err) == SUCCESS) {
		errno = 0;
		return SUCCESS;
	} else {
		printErr(err);
		return FAIL;
	}
}

// Calls the functions taking undefined number of arguments.
int multiCaller(int count, char **names, int ind) {
	if(count == 2) {
		syntaxErr(names[1]);
		return FAIL;
	}
	int (*mCall[]) (char*, char*) = {c,rm,inf,del};
	char err[BUFSIZ];
	for (int i = 2; i < count; i++)
	{
		if((*mCall[ind])(names[i], err) == SUCCESS) {
			errno = 0;
		} else {
			printErr(err);
		}
	}
	return SUCCESS;
}

//Create a directory
int c(char* name, char* err) {
	sprintf(err, "Error: Can't create directory named \"%s\"", name);
	#if OS == 1 || OS == 2
		return mkdir(name);
	#else
		return mkdir(name, defMod);
	#endif
}

int rm(char* name, char* err) {
	sprintf(err, "Error: Can't remove directory named \"%s\"", name);
		return rmdir(name);
}

int inf(char* name, char* err) {
	printf("\n");
	int i = 0, id = -1;
	struct stat info;
	mode_t types[5] = {S_IFBLK, S_IFCHR, S_IFDIR, S_IFIFO, S_IFREG};

	char *types_name[] = {"Block Special Device","Character Special Device", "Directory",
				"FIFO Special", "Regular"};
	char perm[10];
	mode_t permission[9] = {S_IRUSR,S_IWUSR,S_IXUSR,S_IRGRP,S_IWGRP,S_IXGRP,S_IROTH,S_IWOTH,S_IXOTH};
	char bits[3] = {'r','w','x'};	
	if(stat(name,&info) != -1){
		fprintf(stdout,"\t Name : %s\n", name);
		fprintf(stdout,"\t Size : %ld bytes\n", info.st_size);
		//Printing the size of data.
		for( i = 0; i < 5; i++) {
			if(info.st_mode & types[i]){
				id = i;
				break;
			}			
		}
		if(id != -1)
		fprintf(stdout,"\t Data type : %s\n",types_name[id]);
		else
		fprintf(stdout,"\t Data type : Undefined\n");
		//Printing the data type if available.	
		for( i = 0; i < 9;) {
			int j;
			for( j = 0; j <= 2; j++) {
				if(info.st_mode & permission[i]) {
					perm[i] = bits[j];
				} else {
					perm[i] = '-';
				}
				i++;
			}			
		}
		perm[i] = '\0';	
		fprintf(stdout,"\t Permission : (%o / %s)\n", info.st_mode, perm);
		//Printing the file permission for user, group and other.	
		fprintf(stdout,"\t User ID : %u\t Group ID : %u\n", info.st_uid, info.st_gid);
		//Printing the User ID and Group ID of the file owner.
		#if OS == 0
			fprintf(stdout,"\t Parent Device ID : %2d/%2d\n",major(info.st_dev),minor(info.st_dev));
			//Printing the device ID where the file is located.
			if(id == 0 || id == 1)
				fprintf(stdout,"\t Device ID : %2d/%2d\n",major(info.st_rdev),minor(info.st_rdev));				
				//Printing the ID of the device if file is a device.
		#else  
			fprintf(stdout,"\t Device ID : %c\n",info.st_dev + 'A');
		#endif
		fprintf(stdout,"\t Node number : %ld\n", info.st_ino);
		//Printing the file serial number.
		#if OS == 0				
		fprintf(stdout,"\t Blocks : %ld\t Block size : %ld bytes\n", info.st_blocks, info.st_blksize);
		//Printing the number of blocks allocated for this object and size of each block.
		#endif
		fprintf(stdout,"\t Number of links : %u\n", info.st_nlink);	
		//Printing the number of hard links to the file.
		fprintf(stdout,"\t Last Accessed : %s", ctime(&info.st_atime));
		fprintf(stdout,"\t Last Modified : %s", ctime(&info.st_mtime));
		fprintf(stdout,"\t Last Changed : %s", ctime(&info.st_ctime));
		//Printing out last accessed, last modified and last status changed time of the file.

	} else {
		sprintf(err,"\tError: \"%s\" data doesnot exist.", name);	
		return FAIL;	
	}	 
	return SUCCESS;
}

int del(char* name, char* err) {
	sprintf(err, "Error: Can't delete file/directory named \"%s\"", name);
	if(remove(name) != SUCCESS){
		return FAIL;
	}
	return SUCCESS;
}

int cp(char *sourcedata, char* destination,  char* err) {
	struct stat source;
	struct stat dest;
	struct utimbuf udate;
	if(access(sourcedata, F_OK | R_OK) != -1) {
		if(stat(sourcedata, &source) != -1) {
			udate.actime = source.st_atime;
			udate.modtime = source.st_mtime;
			FILE  *fp, *fp1;
			fp = fopen(sourcedata, "r");
			if(fp == NULL) {
				sprintf(err, "Error: Can't read the file \"%s\"", sourcedata);
				return FAIL;
			}
			if(stat(destination, &dest) != -1) {
				if(!(dest.st_mode & S_IFREG)) {
					if(chdir(destination) == -1) {
						sprintf(err, "Error: No such directory \"%s\"", destination);
						return FAIL;
					} else {
						strcpy(destination, sourcedata);
					}
				}
			}
			errno = 0;			
			fp1 = fopen(destination, "w");
			if(fp1 == NULL){
				sprintf(err, "Error: Can't create the file \"%s\"", destination);
				return FAIL;
			}
			char a;
			while(fread(&a, sizeof a, 1, fp) == 1) {
				fwrite(&a, sizeof a, 1, fp1);
			}
			fclose(fp);
			fclose(fp1);
			if((utime(destination, &udate)) != 0) {
					sprintf(err, "Warning ");			
			}
			#if OS == 0 
				chmod(destination, source.st_mode);
			#endif
		}
		else {
			fprintf(stderr, "Failure : Error in source file");
			return FAIL;
		}	
	} else {
		fprintf(stderr, "Failure : Source \"%s\" not exist", sourcedata);	
		return FAIL;
	}
	return SUCCESS;
}

int mv(char* sourcedata, char* destination, char *err) {
	char puff[BUFSIZ];
	if(getcwd(puff, sizeof puff) == NULL) {
		sprintf(err, "Error: Error getting working directory.");
		return FAIL;
	}
	if(cp(sourcedata, destination, err) != SUCCESS) {
		return FAIL;
	}
	if(chdir(puff) != 0) {
		sprintf(err, "Error: Error deleting the source file \"%s\"", sourcedata);
	}
	if(remove(sourcedata) != 0) {
		sprintf(err, "Error: Can't remove the source file \"%s\"", sourcedata);
		return FAIL;
	}
	return SUCCESS;
}

int rn(char *oldname, char* newname, char *err) {
	printf("\nWorking on the programming. Sorry for inconvenience\n\n");
	return SUCCESS;
}

int l(char* dirpath, char* err) {
	if(dirpath == NULL) {
		return l(".", err);
	}
	sprintf(err,"Error : Can't list the files and directory from \"%s\"", dirpath);
	DIR *dir;
	struct dirent *dirzeiger;
	if((dir = opendir(dirpath)) == NULL) return FAIL;
	while((dirzeiger = readdir(dir)) != NULL) {
		if((strcmp(dirzeiger->d_name, ".") != 0) && (strcmp(dirzeiger->d_name,"..") != 0))
			printf("%s\n",dirzeiger->d_name);
	}
	closedir(dir);
	return SUCCESS;
}

int ch(char* dir, char* err) {
	if(chdir(dir) != 0) {
		sprintf(err, "Error: Error changing to the directory \"%s\"", dir);
		return FAIL;
	}
	printf("\nWorking on the programming. Sorry for inconvenience\n\n");
	return SUCCESS;
}

int edit(char* filename, char* err) {
	printf("\nWorking on the programming. Sorry for inconvenience\n\n");
	return SUCCESS;
}