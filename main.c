#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>


#define MAX_INPUT_LENGTH 1000

char user_input[MAX_INPUT_LENGTH];  //used to store user input
char *parsed_array[100];    //used to parse arguments
char current_directory[1000];
char input_file_name[1000];
char output_file_name[1000];

bool truncate_test = false;
bool append_test = false;

extern char **environ;      //posix compliant environment list

int parse_line (char *user_input, char *parsed_array[]);
void execute_command (char *parsed_array[]);

int dont_wait;          //flag set to 1 if process is run in background, 0 in foreground
int status;

FILE *fp;


int main(int argc, char* argv[])
{	
    
    dont_wait = 0;
    if(argc == 2) {// batch file present
        
        if (!access(argv[1], R_OK)){
            freopen(argv[1], "r", stdin);
        }
        else {
            fprintf(stdout, "\nERROR: couln't open the specified input file\n");
            exit(-1);
        }
        
        while (!feof(stdin)){
            
            if (fgets(user_input, MAX_INPUT_LENGTH, stdin)){
                
                if (parse_line(user_input, parsed_array)){
                    fprintf(stdout, "error parsing\n");
                }
                else {
                    execute_command(parsed_array);
                }
            }
        }
    }
    
    if (argc == 1) {
        while (1) {
            //fputs(">", stdout);
            getcwd(current_directory, 1000);
            //strcat(current_directory, " >");
            fputs( strcat(current_directory, " >") , stdout);
            //fprintf(stdout, current_directory);
            
            
            if (fgets(user_input, MAX_INPUT_LENGTH, stdin)){
                
                if (parse_line(user_input, parsed_array)){
                    fprintf(stdout, "error parsing\n");
                }
                else {
                    execute_command(parsed_array);
                }
                
            }
        }
    }
    
    
    return 0;
}


void execute_command(char *parsed_array[]){
    
    
    
    //execute clear command-----------------------------------------------
    if (strcmp(parsed_array[0], "clr") == 0) {
        
        if (parsed_array[1] != NULL){
            memset(parsed_array, '\0', 100);
            fprintf(stderr, "clr function takes no arguments\n");
            return;
        }
        system("clear");
        
        
    }
    else if (strcmp(parsed_array[0], "cd") == 0){
        
        if (parsed_array[2] != NULL){
            memset(parsed_array, '\0', 100);
            fprintf(stderr, "cd function shuold be in cd <target_path> format\n");
            return;
        }
        
        
        if (parsed_array[1] != NULL){ // <directory> argument present
            
            int test_direc_path = chdir(parsed_array[1]); // checks if specified path exits. 0 if sucess -1 if fail.
            
            if (!test_direc_path)
            {
                getcwd(current_directory, 999);
                setenv("PWD", current_directory, 1);
            }
            else
            {
                fprintf(stderr, "cd function couldn't find the specified path\n");
            }
        }
        else { // <directory> argument not present. report current directory
            char temp[1000];
            getcwd(temp, 999);
            fprintf(stdout, "%s", temp);
            fprintf(stdout, "\n");
        }
        
        
    }
    //execute dir command-------------------------------------------------
    else if (strcmp(parsed_array[0], "dir") == 0) {
        
        int k;
        
        //        for (int j = 1; parsed_array[j] != NULL; j++) {
        //            fprintf(stdout, "\n Folder: %s :", parsed_array[j]);
        //        }
        
        //        fprintf(stdout, "dir command\n");
        pid_t pid;
        
        
        
        if (parsed_array[1] == NULL){
            fprintf(stdout, "no path specified for dir function\n");
            return;
        }
        else {
                            
            for (k = 1; parsed_array[k] != NULL; k++) {
                
                //                if (!append_test && !truncate_test)
                //                {
                //                    fprintf(stdout, "\nFolder: %s :", parsed_array[i]);
                //                    fprintf(stdout, "\n");
                //                }
                
                switch (pid = fork()){
                    case -1:
                        fprintf(stderr, "forking error\n");
                        exit(-1);
                    case 0:
                        if (append_test)
                            freopen(output_file_name, "a+" , stdout);
                        else if (truncate_test)
                            freopen(output_file_name, "a+" , stdout);
                        
                        execlp("ls", "ls", "-al", parsed_array[k], NULL);
                    default:
                        if (!dont_wait)
                            waitpid(pid, &status, WUNTRACED);
                }
            }
        }
        
        append_test = false;
        truncate_test = false;
        
    }
    //execute environ command-----------------------------------------------
    else if (strcmp(parsed_array[0], "environ") == 0) {
        
        if (parsed_array[1] != NULL){
            memset(parsed_array, '\0', 100);
            fprintf(stderr, "environ function takes no arguments\n");
            return;
        }
        
        //fprintf(stdout, "environ command\n");
        pid_t pid;
        
        char **ptenviron = environ;
        
        switch (pid = fork()){
            case -1:
                fprintf(stderr, "environ error\n");
                exit(-1);
            case 0:
                if (append_test)
                    freopen(output_file_name, "a+" , stdout);
                else if (truncate_test)
                    freopen(output_file_name, "w+" , stdout);

                
                while(*ptenviron != '\0'){
                    fprintf(stdout, "%s\n", *ptenviron);
                    ptenviron++;
                }
            default:
                if (!dont_wait)
                    waitpid(pid, &status, WUNTRACED);
        }
        
        
    }
    //execute echo command-----------------------------------------------
    else if (strcmp(parsed_array[0], "echo") == 0) {
        

        
        //fprintf(stdout, "echo command\n");
        pid_t pid;
        int i;
        
        
        switch (pid = fork()){
            case -1:
                fprintf(stderr, "environ error\n");
                exit(-1);
            case 0:
                if (append_test)
                    fp = freopen(output_file_name, "a+" , stdout);
                else if (truncate_test)
                    fp = freopen(output_file_name, "w+" , stdout);
                
                for (i = 1; parsed_array[i] != NULL; i++)
                    //fputs(parsed_array[i], stdout);
                    fprintf(stdout, "%s ", parsed_array[i]);
                
                fclose(fp);
                
            default:
                if (!dont_wait)
                    waitpid(pid, &status, WUNTRACED);
        }
        
        append_test = false;
        truncate_test = false;
        
        
        
    }//execute quit command--------------------------------------------------
    else if (strcmp(parsed_array[0], "quit") == 0) {
        
        if (parsed_array[1] != NULL){
            memset(parsed_array, '\0', 100);
            fprintf(stderr, "quit function takes no arguments\n");
            return;
        }
        
        exit(0);
    }
    
    
    
    // if no internal command found then input is relayed to parent shell----
    else {
        fprintf(stdout, "execute external command '%s'\n", parsed_array[0]);
        pid_t pid;
        
        switch (pid = fork()){
            case -1:
                fprintf(stderr, "forking error\n");
                exit(-1);
            case 0:
                execvp(parsed_array[0], parsed_array);
                exit(0);
            default:
                if (!dont_wait)
                    waitpid(pid, &status, WUNTRACED);
                
        }
        
        
    }
}



int parse_line (char *user_input, char *parsed_array[]){
    int total_args = 0;
    int k,l;
    
    if (user_input == NULL){
        return -1;
    }
    else {
        
        char **parsed_arr = parsed_array;
        
        *parsed_arr = strtok(user_input, " \t\n"); //first argument saved
        ++parsed_arr;
        
        while ((*parsed_arr = strtok(NULL, " \t\n")) != NULL){
            ++parsed_arr;
        }
        
        for (k = 0; parsed_array[k] != NULL; k++)
            total_args++;
        //fprintf(stdout, "total arguments: %d", total_args);
        
        
        
        for (l = 0; parsed_array[l] != NULL; l++)
        {
            if (strcmp(parsed_array[l], "<") == 0){//input file argument
                //fprintf(stdout, "< token found\n");
                strcpy(input_file_name, parsed_array[l+1]);
                //fprintf(stdout, "input_file_name: %s" ,input_file_name);
                parsed_array[l] = NULL;
                
                if (!access(input_file_name, R_OK)){
                    freopen(input_file_name, "r", stdin);
                }
                else {
                    fprintf(stdout, "\nERROR: couln't open the specified input file\n");
                    exit(-1);
                }
                
            }
            else if (strcmp(parsed_array[l], ">") == 0){ //output file + trunc
                //fprintf(stdout, "> token found\n");
                strcpy(output_file_name, parsed_array[l+1]);
                //fprintf(stdout, "input_file_name: %s" ,input_file_name);
                parsed_array[l] = NULL;
                truncate_test = true;
                
            }
            else if (strcmp(parsed_array[l], ">>") == 0){ //output file + append
                //fprintf(stdout, "> token found\n");
                strcpy(output_file_name, parsed_array[l+1]);
                //fprintf(stdout, "input_file_name: %s" ,input_file_name);
                parsed_array[l] = NULL;
                append_test = true;
                
            }
        }
        
        return 0;
    }
}

