
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define buff_size 256

int cd(char **args);
int exitinstr();
void INandOUT(char **args);
int order(char **args);
bool input, output;
char *inFile, *outFile;


//redirection
void INandOUT(char **args){

    int i = 0;

    while(args[i] != NULL){   //loops through command line

        if (!strcmp(args[i], "<")){
            strcpy(args[i], "\0");
            inFile = args[i+1];
            input = true;
        }
        else if (!strcmp(args[i], ">")){
            outFile = args[i+1];
            args[i] = NULL;
            output = true;
            break;
        }
        i++;
    }
}

// & operation/command
bool background_op(char **args){
    int i = 0;
    bool wait = true;
    while(args[i] != NULL){
        if (!strcmp(args[i], "&")){
            wait = false;
            args[i] = NULL;
        }
        i++;
    }
    return wait;
}

//input commands that perform action
int execute(char **args)
{

    if (args[0] == NULL) {
        return 1;
    }

    if (strcmp(args[0], "cd") == 0) {
        return cd(args);
    }
    else if (strcmp(args[0], "exit") == 0) {
        return exitinstr();
    }
    else {
        return order(args);
    }
}

int cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "Error, shell did not change dir\n");
    }
    else {
        if (chdir(args[1]) != 0) {
            perror("error changing dir");
        }
    }
  return 1;
}

int exitinstr()
{
    return 0;
}

int order(char **args)
{
    pid_t pid, wpid;
    int status;
    bool wait = true;


    wait = background_op(args);
    pid = fork();

    if (pid == 0) {  //child
        if(output == true)
            freopen(outFile, "w", stdout);

        if (execvp(args[0], args) == -1) {
            perror("error on  fork");
        }
        exit(1);
    }
    else if (pid < 0) {
        perror("error on fork");
    }
    else {    //parent
        do {
            if(wait)wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

  return 1;
}

int cntr(char **args){

    int i =0, num_cmds = 0;
	while (args[i] != NULL){
		if (strcmp(args[i],"|") == 0){
			num_cmds++;
		}
		i++;
	}
	num_cmds++;

	return num_cmds;
}

char **alloc(char *line)
{
    int bufsize = buff_size, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(1);
    }

    token = strtok(line, " \n");

    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += buff_size;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if (!tokens) {
                fprintf(stderr, "allocation error\n");
                exit(1);
            }
        }
        token = strtok(NULL, " \t\n");
    }
    tokens[position] = NULL;
    return tokens;
}

char *parse(void){

    int bufsize = buff_size;
    char *buffer = malloc(sizeof(char) * bufsize);
    int index = 0, c;

    if (!buffer) {
        fprintf(stderr, "parse error\n");
        exit(1);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[index] = '\0';
            return buffer;
        }
        else {
            buffer[index] = c;
        }
        index++;

        if (index >= bufsize) {
            bufsize += buff_size;
            buffer = realloc(buffer, bufsize);
            if(!buffer) {
                fprintf(stderr, "parse error\n");
                exit(1);
            }
        }
    }
}

void command(void)
{
    char *line;
    char **args;
    int status;

    do {
        line = parse();
        args = alloc(line);
        INandOUT(args);

        if(input){

            printf("input from txt ");
            FILE *f = fopen(inFile, "r");

            while(fgets(line, buff_size, f)){
                printf("Output of %s",line);
                args = alloc(line);
                status = execute(args);
            }
            fclose(f);
        }
        else{
            status = execute(args);
        }
        free(line);
        free(args);

    } while (status);
}

//main loops through command list
int main(int argc, char **argv)
{
    printf("Daniel's x86 shell: \n");
    command();

    return 1;
}

