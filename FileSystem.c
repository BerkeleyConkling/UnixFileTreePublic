#include <stdio.h>            
#include <stdlib.h>
#include <string.h>   
#define TRUE 1
#define FALSE 0

typedef struct node {
	char  name[64];       // node's name string
	char  type;
	struct node *child, *sibling, *parent;
} NODE;




NODE *root; 
NODE *cwd;
const int pathArrSize = 20;
char *cmd[] = {"mkdir", "rmdir", "cd", "ls", "pwd", "creat", "rm", "save", "reload", "quit"};  // fill with list of commands
char *pathArr[20];//default value is "NULLNODE", stores the tokens for a file path given by the user, first index stores if it is from the root or not 
// other global variables


int initialize() {
	root = (NODE *)malloc(sizeof(NODE));
	strcpy(root->name, "/");
	root->parent = root;
	root->sibling = 0;
	root->child = 0;
	root->type = 'D';
	cwd = root;

	for(int i = 0; i < 20; i ++){//initialize pathArr and set our default value
		pathArr[i] = "NULLNODE";
	}

	printf("Filesystem initialized!\n");
}

NODE* makeNode(char* name, char nodeType){
	NODE* retNode = (NODE*) malloc(sizeof(NODE));//allocate heap space for the new node
	strcpy(retNode->name, name);//copy name to new node
	//set pointers to null
	retNode->sibling = NULL;
	retNode->parent = NULL;
	retNode->child = NULL;

	retNode->type = nodeType;//set the new nodes type (Either File or Directory)
	return retNode;//return new node
}

int checkIfRoot(char* path){//checks if a file path is going from the root 
	if(strlen(path) > 0){
		if(path[0] == '/') return TRUE;//if the path starts with the root identifier rturn that it is the root
		else{ return FALSE;}
	}
	return -1;
}

int tokenizePathname(char* pathname){//return true if succesfully run otherwiser return false 
	//char* pathnameCopy[strlen(pathname)];
	//strcpy(pathnameCopy, pathname);//copy the path name so we are not modifying the original string

	if(pathname == NULL){
		return;
	}
	int isFromRoot = checkIfRoot(pathname);
	char* curToken = "";
	
	
	//reset the path array 
	for(int i = 0; i < 20; i ++){
		pathArr[i] = "NULLNODE";
	}

	if(isFromRoot != -1){//if the string is not empty 
		if(isFromRoot == TRUE){
			curToken = strtok(pathname, "/");
			pathArr[0] = "ROOT";
			int i = 1;
			while(curToken != NULL){
				pathArr[i] = curToken;
				i++;
				curToken = strtok(NULL, "/");
			}
		}
		else{
			curToken = strtok(pathname, "/");
			pathArr[0] = "NOTROOT";
			int i = 1;
			while(curToken != NULL){
				pathArr[i] = curToken;
				i++;
				curToken = strtok(NULL, "/");
			}
		}
		return 1;
	}else{
		return 0;//return false that it did not run 
	}
			
}
//need to test this
void pwd(NODE* currentDir){//pass in cwd
	if(currentDir == root){//we are at root 
		//fputs(currentDir->name, stdout);
		return;//break recursion
	}else{
		pwd(currentDir->parent);
		printf("/");
		fputs(currentDir->name, stdout);
	}
}

void getPath(NODE* curNode, char *dest){//helper function for save 
	char slash[] = "/";
	if(curNode == root){//we are at root 
		//strcpy(dest, slash);
		return;//break recursion
	}else{
		
		getPath(curNode->parent, dest);
		strcat(dest, slash);
		strcat(dest, curNode->name);

	}
}

void recursiveSave(FILE* outfile, NODE* curNode, int firstLoop){
	char destStr[200] = "";
	if(curNode != NULL){
		
		if(curNode != root){//print to file, skip root since there will always be a root
			getPath(curNode, destStr);
			if(firstLoop){//check if its our first loop so we dont get an empty first line in the teext file 
				fprintf(outfile,"%c %s",curNode->type, destStr);
			}else{
				fprintf(outfile,"\n%c %s",curNode->type, destStr);
			}
		}
		
		recursiveSave(outfile, curNode->child, FALSE);//go down child path 
		recursiveSave(outfile, curNode->sibling, FALSE);//go down sibling path 	
	}
}

void save(char* filename){
	if(filename == NULL){
		printf("Must enter filename!");
		return;
	}
	FILE * fp = fopen(filename, "w+");//open file
	if(fp != NULL){
		recursiveSave(fp, root->child, TRUE);//save to file

		fclose(fp);//close file
	}else{
		printf("Could not open file!");
	}
	
}

void cd(char* inputPathname){//if its cd alone pass in "NONE"
	NODE* curNode = NULL;
	int pathNameTokenized;
	int run = TRUE;
	int loops = 1;
	int notFound = FALSE;
	int innerLoop = TRUE;
	char baseName[99] = "";
	char pathname[200] ="";
	if(inputPathname == NULL){//make sure there was a path that was input
		printf("Must input a path!");
		return;
	}
	strcpy(pathname, inputPathname);

	if(strcmp(pathname, "NONE") == 0){
		cwd = root;
	}else{

		pathNameTokenized = tokenizePathname(pathname);

		while(run){//find target dir
			if(strcmp(pathArr[loops], "NULLNODE") == 0 || loops == pathArrSize - 1){
				run = FALSE;
				strcpy(baseName,pathArr[loops - 1]);
			}
			loops++;
		}

		run = TRUE;
		loops = 1;

		if(strcmp("NOTROOT", pathArr[0]) == 0){//if we are not coming from the root, rather we are coming from CWD
			curNode = cwd;
		}
		else if(strcmp("ROOT", pathArr[0]) == 0){//if we are coming from the root 
			curNode = root;//set current node to the root 
		}

		do{
			innerLoop = TRUE;
			curNode = curNode->child;
			if(curNode == NULL){
				innerLoop = FALSE;
				run = FALSE;
				notFound = TRUE;
			}

			while(innerLoop){//iterate through directory levels 
				if(curNode == NULL){
					innerLoop = FALSE;
					run = FALSE;
					notFound = TRUE;
				}
				else if(strcmp(pathArr[loops], curNode->name) == 0){//if we found next dir to go down
					innerLoop = FALSE;
					if(strcmp(baseName, curNode->name) == 0){//if its the target stop main loop
						run = FALSE;
					}
					if(curNode->type == 'F'){//if not a dir then we must not go down it 
						notFound = TRUE;
						run = FALSE;
						printf("\nThe input file tries to use file as a directory, cannot execute!");
					}
				}
				else{
					curNode = curNode->sibling;
				}
			}

			loops++;
		}while(run);

		if(curNode == NULL){
			printf("\n Cannot set to directory that does not exist");
		}
		else if(notFound == FALSE && curNode->type != 'F'){
			cwd = curNode;
		}else if(curNode->type=='F'){
			printf("\nFile path is not a directory");
		}
		else{
			printf("\nCould not set cwd");
		}

	}
}

void rm(char* inputPathname){
	NODE* curNode = NULL, *parentNode = NULL, *siblingNode = NULL, *prevNode = NULL;
	
	int pathNameTokenized;
	int run = TRUE;
	int loops = 1;
	int notFound = FALSE;
	int innerLoop = TRUE;
	char baseName[99];

	char pathname[200] ="";
	if(inputPathname == NULL){//make sure there was a path that was input
		printf("Must input a path!");
		return;
	}
	strcpy(pathname, inputPathname);
	pathNameTokenized = tokenizePathname(pathname);

	while(run){
		if(strcmp(pathArr[loops], "NULLNODE") == 0 || loops == pathArrSize - 1){
			run = FALSE;
			strcpy(baseName,pathArr[loops - 1]);
		}
		loops++;
	}

	run = TRUE;
	loops = 1;
	
	if(strcmp("NOTROOT", pathArr[0]) == 0){//if we are not coming from the root, rather we are coming from CWD
		curNode = cwd;
	}
	else if(strcmp("ROOT", pathArr[0]) == 0){//if we are coming from the root 
		curNode = root;//set current node to the root 
	}

	do{
		innerLoop = TRUE;
		prevNode = curNode;
		curNode = curNode->child;
		if(curNode == NULL){//if we are at a null node the path wasnt found 
			innerLoop = FALSE;
			run = FALSE;
			notFound = TRUE;
			printf("\nFile path does not exist");
		}
		while(innerLoop == TRUE){//iterate through directory levels 
			if(curNode == NULL){
				innerLoop = FALSE;
				run = FALSE;
				notFound = TRUE;
			//	printf("\n Could not find path");
			}
			else if(strcmp(pathArr[loops], curNode->name) == 0){//if we found next thing to go down to continue down 
				innerLoop = FALSE;
				if(strcmp(baseName, curNode->name) == 0){//if it is the target file stop loop
					run = FALSE;
				}
				else if(curNode->type == 'F'){//if it was not the target file, and is af ile we cant move down it so the path tried to use file as dir
					notFound = TRUE;
					run = FALSE;
					printf("\nThe input file tries to use file as a directory, cannot execute!");
				}
			}else{
				prevNode = curNode;
				curNode = curNode->sibling;
			}

		}
		loops++;

	}while(run);
	//delete file
	if(curNode != NULL){
		if(curNode->type == 'F' && notFound == FALSE){

			siblingNode = curNode->sibling;
			parentNode = curNode->parent;
			if(parentNode == prevNode){//if the prev node was the parent
				parentNode->child = siblingNode;
			}else{//if the prev node was not the parent
				prevNode->sibling = siblingNode;
			}
			
			free(curNode);

		}else{
			printf("\n rm command cannot remove directory type");
		}
	}else{
		fflush(stdout);
		printf("\nInvalid path name");
	}
}

void rmdir(char* inputPathname){
	
	NODE* curNode = NULL, *parentNode = NULL, *siblingNode = NULL, *prevNode = NULL;
	
	int pathNameTokenized;
	int run = TRUE;
	int loops = 1;
	int notFound = FALSE;
	int innerLoop = TRUE;
	char baseName[99];

	char pathname[200] ="";
	if(inputPathname == NULL){//make sure there was a path that was input
		printf("Must input a path!");
		return;
	}
	strcpy(pathname, inputPathname);
	pathNameTokenized = tokenizePathname(pathname);

	while(run){
		if(strcmp(pathArr[loops], "NULLNODE") == 0 || loops == pathArrSize - 1){
			run = FALSE;
			strcpy(baseName,pathArr[loops - 1]);
		}
		loops++;
	}

	run = TRUE;
	loops = 1;
	
	if(strcmp("NOTROOT", pathArr[0]) == 0){//if we are not coming from the root, rather we are coming from CWD
		curNode = cwd;
	}
	else if(strcmp("ROOT", pathArr[0]) == 0){//if we are coming from the root 
		curNode = root;//set current node to the root 
	}

	do{
		innerLoop = TRUE;
		prevNode = curNode;
		curNode = curNode->child;
		if(curNode == NULL){//if we have reacehd a null node the path did not exist 
			innerLoop = FALSE;
			run = FALSE;
			notFound = TRUE;
			printf("\nDirectory does not exist");
		}
		while(innerLoop == TRUE){//iterate through directory levels 
			if(curNode == NULL){
				innerLoop = FALSE;
				run = FALSE;
				notFound = TRUE;
			//	printf("\n Could not find path");
			}
			else if(strcmp(pathArr[loops], curNode->name) == 0){//if we found the right directory to go down break inner loop to go to child
				innerLoop = FALSE;
				if(strcmp(baseName, curNode->name) == 0){//if we are at the delete target break main loop
					run = FALSE;
				}
				if(curNode->type == 'F'){//if it is a file not a directory terminate and give error 
					notFound = TRUE;
					run = FALSE;
					printf("\nThe input file tries to use file as a directory, cannot execute!");
				}
			}else{
				prevNode = curNode;
				curNode = curNode->sibling;
			}

		}
		loops++;

	}while(run);
	//delete file
	if(curNode != NULL){//make sure we arent working with a null node (Not sure why the earlier check wasnt preventing this but it was needed here )
		if(curNode->type == 'D' && notFound == FALSE){
			if(curNode->child == NULL){//directory must be empty to delete it
				if(curNode == cwd){//if we are at the cwd reset to root so we dont have a dangling pointer as the cwd
					cwd = root;	
				}
				siblingNode = curNode->sibling;//save sibling and parent 
				parentNode = curNode->parent;
				if(parentNode == prevNode){//if the prev node was the parent
					parentNode->child = siblingNode;
				}else{//if the prev node was not the parent
					prevNode->sibling = siblingNode;
				}
				
				free(curNode);//delete node
			}else{
				printf("\n Cannot remove non-empty directory!");
			}
		}else{
			fflush(stdout);
			printf("\n rmdir command cannot remove file type");
		}
	}else{
		fflush(stdout);
		printf("\nInvalid path name");
	}
	
}

void deleteTree(NODE* curNode){//pass in root->child, this function de allocates all the tree memory to prevent memory leaks
	if(curNode == NULL){
		return; //break recursion
	}
	deleteTree(curNode->child);
	deleteTree(curNode->sibling);
	//printf("\nDeleting %s of type %c", curNode->name, curNode->type);
	free(curNode);
}

void reload(char* filename){
	deleteTree(root->child);//if there is an existing tree remove it so we can load the new one
	root->child = NULL;
	FILE* fp = fopen(filename, "r");//open file 
	char line[200] = "";
	int run = TRUE, eof = FALSE;;
	

	while(run){

		char *type, *path;
		eof = fgets(line,200, fp);//read the line from file 
		
		if(eof == NULL){//if we are at the end of file terminate
			run = FALSE;
		}else{
			type = strtok(line, " ");
			path = strtok(NULL, "\n");//read until newline

			if(strcmp(type, "F") == 0){//if file type creat as file
				creat(path);
			}
			else if(strcmp(type, "D") == 0){//if directory type create as directory 
				mkdir(path);
			}
		}
		

	}
	fclose(fp);//close file
}

void ls(char* inputPathname){//if it is just "ls" alone pass in "NONE"
	NODE* curNode = NULL;
	int pathNameTokenized;
	int run = TRUE;
	int loops = 1;
	int notFound = FALSE;
	int innerLoop = TRUE;
	char baseName[99];

	char pathname[200] ="";
	if(inputPathname == NULL){//make sure there was a path that was input
		printf("Must input a path!");
		return;
	}
	strcpy(pathname, inputPathname);

	if(strcmp(pathname, "NONE") == 0){//if it was just a lone "ls" command print cwd
		curNode = cwd;
		curNode = curNode->child;
		printf("\n");
		while(curNode!= NULL){//go through directory and print contents
			printf("%c ", curNode->type);
			fputs(curNode->name, stdout);
			printf("\n");
			curNode = curNode->sibling;
		}
	}
	else{
		pathNameTokenized = tokenizePathname(pathname);

		while(run){
			if(strcmp(pathArr[loops], "NULLNODE") == 0 || loops == pathArrSize - 1){//get the name of the dir we want to print
				run = FALSE;
				strcpy(baseName,pathArr[loops - 1]);
			}
			loops++;
		}

		run = TRUE;
		loops = 1;
		
		if(strcmp("NOTROOT", pathArr[0]) == 0){//if we are not coming from the root, rather we are coming from CWD
			curNode = cwd;
		}
		else if(strcmp("ROOT", pathArr[0]) == 0){//if we are coming from the root 
			curNode = root;//set current node to the root 
		}

		do{
			innerLoop = TRUE;//reset inner loop check
			curNode = curNode->child;//move to child to start checking next row 
			if(curNode == NULL){//if we move to a null node the path input doesnt exist 
				innerLoop = FALSE;
				run = FALSE;
				notFound = TRUE;
				printf("\nFile path does not exist");
			}
			while(innerLoop == TRUE){//iterate through directory levels 
				if(curNode == NULL){//if we reach the end of the directory and have not found the next step print error and stop 
					innerLoop = FALSE;
					run = FALSE;
					notFound = TRUE;
					printf("\n Could not find path");
				}
				else if(strcmp(pathArr[loops], curNode->name) == 0){//if we found the next directory to go down 
					innerLoop = FALSE;
					if(strcmp(baseName, curNode->name) == 0){//if it is the base name we are searching for stop the main loop
						run = FALSE;
					}
					if(curNode->type == 'F'){//if it is a file not a dir we cannot execute 
						notFound = TRUE;
						run = FALSE;
						printf("\nThe input file tries to use file as a directory, cannot execute!");
					}
				}else{
					curNode = curNode->sibling;
				}

			}
			loops++;

		}while(run);

		printf("\n");
		curNode = curNode->child;
		while(curNode!= NULL && notFound == FALSE){//go through directory and print contents
			fputs(curNode->name, stdout);
			printf(" ");
			curNode = curNode->sibling;
		}
	}
	
}

void mkdir(char* inputPathname){
	//need to add a check for if it is just a single node, ie we are adding to the current node a new directory 
	int loops = 1;
	char baseName[99];
	NODE* curNode, *prevNode;
	int run = TRUE;
	int depth = 0;//depth stores how far we have to go down 

	char pathname[200] ="";
	if(inputPathname == NULL){//make sure there was a path that was input
		printf("Must input a path!");
		return;
	}
	strcpy(pathname, inputPathname);
	int pathNameTokenized = tokenizePathname(pathname);

	for(int i = 1; i < pathArrSize; i++){
		if(strcmp(pathArr[i], "NULLNODE") != 0){
			depth++;
		}else{
			i = pathArrSize; //cancel loop
		}
	}
	depth = depth -1;

	if(depth < 0){
		//user did not input a file path
		printf("\nMust input a file path for mkdir");
	}
	else if(pathNameTokenized == 1){//tokenize the pathname so we can look at each node we need to traverse, and check if it ran correctly
		
		strcpy(baseName, pathArr[depth + 1]);
		if(strcmp("NOTROOT", pathArr[0]) == 0){//if we are not coming from the root, rather we are coming from CWD
			curNode = cwd;
		}
		else if(strcmp("ROOT", pathArr[0]) == 0){//if we are coming from the root 
			curNode = root;//set current node to the root 
		}

		//start of main loop to search through the tree 
		do{
			if(depth == 0){
				
				if(curNode->child == NULL){
					curNode->child = makeNode(baseName, 'D');
					curNode->child->parent = curNode;
				}else{
					curNode = curNode ->child;
					while(curNode != NULL){//traverse through siblings
						if(strcmp(curNode->name, baseName) == 0){//stop running if there is a file with the same name in this directory
							run = FALSE;
							printf("\nThis directory already exists!");
						}
						prevNode = curNode;
						curNode = curNode->sibling;
					}
					if(run){
						curNode = prevNode;
						curNode->sibling = makeNode(baseName, 'D');
						prevNode = curNode;
						curNode = curNode->sibling;
						curNode->parent = prevNode->parent;//set new nodes parent
					}
					
					
				}
				run = FALSE;
			}
			else{
				prevNode = curNode;
				curNode = curNode->child;
				if(curNode == NULL){//This should never be null because of the above if statement before this else (The one with the depth check)
					printf("\n The entered file path does not exist");
					run = FALSE;
				}else{
					int siblingCheckLoop = TRUE;
					do{
						if(strcmp(curNode->name ,pathArr[loops]) == 0 ){//if we have found the next directory to go down, leave curNode as that
							if(curNode->type == 'F'){
								printf("\n Name \"%s\" in given path is a file not a directory, cannot continue", pathArr[loops]);//this might cause an error
								run = FALSE;
							}else{
								siblingCheckLoop = FALSE;
							}	
						}else{
							prevNode = curNode;
							curNode = curNode->sibling;
						}
						
					}while(curNode != NULL && siblingCheckLoop != FALSE && run == TRUE );

					if(siblingCheckLoop != FALSE){//if we did not find the directory in the file path entered
						run = FALSE;
						printf("\n The entered file path does not exist");
					}
				}
			}


			depth--;
			loops++;
		}while(run);
		
	}

}

void creat(char* inputPathname){
	//need to add a check for if it is just a single node, ie we are adding to the current node a new directory 
	int loops = 1;
	char baseName[99];
	NODE* curNode, *prevNode;
	int run = TRUE;
	int depth = 0;//depth stores how far we have to go down 

	char pathname[200] ="";
	if(inputPathname == NULL){//make sure there was a path that was input
		printf("Must input a path!");
		return;
	}
	strcpy(pathname, inputPathname);
	int pathNameTokenized = tokenizePathname(pathname);

	for(int i = 1; i < pathArrSize; i++){
		if(strcmp(pathArr[i], "NULLNODE") != 0){
			depth++;
		}else{
			i = pathArrSize; //cancel loop
		}
	}
	depth = depth -1;

	if(depth < 0){
		//user did not input a file path
		printf("\nMust input a file path for creat");
	}
	else if(pathNameTokenized == 1){//tokenize the pathname so we can look at each node we need to traverse, and check if it ran correctly
		
		strcpy(baseName, pathArr[depth + 1]);
		if(strcmp("NOTROOT", pathArr[0]) == 0){//if we are not coming from the root, rather we are coming from CWD
			curNode = cwd;
		}
		else if(strcmp("ROOT", pathArr[0]) == 0){//if we are coming from the root 
			curNode = root;//set current node to the root 
		}

		//start of main loop to search through the tree 
		do{
			if(depth == 0){
				
				if(curNode->child == NULL){
					curNode->child = makeNode(baseName, 'F');
					curNode->child->parent = curNode;
				}else{
					curNode = curNode ->child;
					while(curNode != NULL){//traverse through siblings
						if(strcmp(curNode->name, baseName) == 0){//stop running if there is a file with the same name in this directory
							run = FALSE;
							printf("\nThis file path already exists!");
						}
						prevNode = curNode;
						curNode = curNode->sibling;
					}
					if(run){
						curNode = prevNode;
						curNode->sibling = makeNode(baseName, 'F');
						prevNode = curNode;
						curNode = curNode->sibling;
						curNode->parent = prevNode->parent;//set new nodes parent
					}
					
					
				}
				run = FALSE;
			}
			else{
				prevNode = curNode;
				curNode = curNode->child;
				if(curNode == NULL){//This should never be null because of the above if statement before this else (The one with the depth check)
					printf("\n The entered file path does not exist");
					run = FALSE;
				}else{
					int siblingCheckLoop = TRUE;
					do{
						if(strcmp(curNode->name ,pathArr[loops]) == 0 ){//if we have found the next directory to go down, leave curNode as that
							if(curNode->type == 'F'){
								printf("\n Name \"%s\" in given path is a file not a directory, cannot continue", pathArr[loops]);//this might cause an error
								run = FALSE;
							}else{
								siblingCheckLoop = FALSE;
							}	
						}else{
							prevNode = curNode;
							curNode = curNode->sibling;
						}
						
					}while(curNode != NULL && siblingCheckLoop != FALSE && run == TRUE );

					if(siblingCheckLoop != FALSE){//if we did not find the directory in the file path entered
						run = FALSE;
						printf("\n The entered file path does not exist");
					}
				}
			}


			depth--;
			loops++;
		}while(run);
		
	}

}

int main() {
	initialize();
	/*Order: "mkdir", "rmdir", "cd", "ls", "pwd", "creat", "rm", "save", "reload", "quit"*/
	int run = TRUE, input = 0, matchFound = FALSE;;
	char strInput[200] = "", strInputCpy[200] = "";
	while(run) {
		char *command = "", *path = "";

		printf("\nEnter command: ");
		gets(strInput);//get input
		strcpy(strInputCpy, strInput);
		command = strtok(strInputCpy, " ");

		matchFound = FALSE;
		for(int i = 0; i < 10; i ++){
			if(strcmp(command, cmd[i]) == 0){
				input = i;
				matchFound = TRUE;
			}
		}
		if(!matchFound){
			input = -1;
		}

		path = strtok(NULL, "\0");//read rest of token as the path name


		//add checks for cd and ls to check if there is no path entered
		switch(input){
			case(0)://mkdir
				mkdir(path);
				break;

			case(1)://rmdir
				rmdir(path);
				break;

			case(2)://cd

				if(path == NULL){//if there was no input path 
					cd("NONE");
				}else{
					cd(path);
				}
				
				break;

			case(3)://ls
				
				if(path == NULL){//if there was no input path 
					ls("NONE");
				}else{
					ls(path);
				}
				break;

			case(4)://pwd
				if(cwd == root){
					printf("/");
				}else{
					//printf("\n");
					pwd(cwd);
				}
				break;

			case(5)://creat
				creat(path);
				break;
			
			case(6)://rm
				rm(path);
				break;

			case(7)://save
				save(path);
				break;

			case(8)://reload
				
				reload(path);
				break;
			
			case(9)://quit
				run = FALSE;
				save("fssim_conkling.txt");//save to file 

				deleteTree(root->child);//delete tree except root
				free(root);//delete root

				break;
			default://if none execute
				printf("\nCommand not recognized!");
				break;
		}

		
		// complete implementations
	}
}