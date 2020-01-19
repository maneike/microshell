#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
void formatError();

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


int sh_cat(char **args);
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int sh_mv(char **args);



char *builtin_str[] = {
    
    "cat",
    "cd",
    "exit",
    "help",
    "mv"
    
};



int (*builtin_func[]) (char **) = {
    
    &sh_cat,
    &sh_cd,
    &sh_exit,
    &sh_help,
    &sh_mv
    
};



int sh_num_builtins() {
    
    return sizeof(builtin_str) / sizeof(char *);
    
}



#define cat_BUFFER_SIZE 1024

int sh_cat(char **args) {
    
    char buffer[cat_BUFFER_SIZE];
    int fd_in, num;
    char* file = args[1];
    
    if(args[1]==NULL) {
        fprintf(stderr, "cat: expected argument to \"cat\"\n");
    }
    else {
        fd_in=open(file, O_RDONLY);
    
        while((num = read(fd_in, &buffer, cat_BUFFER_SIZE))>0)
        {
            write(STDOUT_FILENO, &buffer, num);
        }
        close(fd_in);
    }

    return 1;

}



int sh_cd(char **args) {

    /*
     obsługiwać polecenie cd, działające analogicznie jak cd znane nam z powłoki bash;
     */
    if (args[1] == NULL) {
        /*
         wypisywać komunikat błędu, gdy niemożliwe jest poprawne zinterpretowanie polecenia;
         */
        fprintf(stderr, "cd: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("sh");
        }
    }
    return 1;
    
}



int sh_exit(char **args) {
    
    /*
     obsługiwać polecenie exit, kończące działanie programu powłoki;
     */
    char* username = getenv("USER");
    printf("Bye, %s%s%s! \n\n", ANSI_COLOR_RED, username, ANSI_COLOR_RESET);
    return 0;
    
}



int sh_help(char **args) {
    
    /*
     obsługiwać polecenie help, wyświetlające na ekranie informacje o autorze programu i oferowanych przez niego funkcjonalnościach;
     */
    int i;
    
    printf("Softshell\n");
    printf("Martin Chelminiak\n");
    printf("Enter a command and hit enter.\n");
    printf("List of available commands:\n");

    for (i = 0; i < sh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    
    return 1;
    
}



sh_mv(char **args) {
    
    char *file = args[1];
    char *location = args[2];
    char newplace[50];
    
    if (args[3]!=NULL) {
        formatError();
    }
    else {
        if(location[0]=='/') {
            /*
             sprawdza, czy input to sciezka
             */
                strcat(location,"/");
                /*
                 jesli tak, to przygotowywuje do przeniesienia na koniec sciezki
                 */
                strcat(location,file);
                if(rename(file, location)== 0)
                    printf("Successful\n");
                else
                    printf("Error:\nDirectory not found\n");
            }
            else {
                DIR *isD;
                isD = opendir(location);
                /*
                 sprawdza czy argument jest DIR'em w CWD
                 */

                if(isD==NULL) {
                    if( rename(file,location)!= 0)
                        printf("Error: File not moved\n");
                    else
                        printf("Successful\n");
                }
                else {
                    char *ptrL;
                    ptrL = getcwd(newplace, 50);
                    /*
                     obecna sciezka katalogu
                     */
                    strcat(newplace, "/");
                    strcat(newplace, location);
                    /*
                     podczepienie lokacji mv do sciezki ptrL
                     */
                    strcat(newplace, "/");
                    strcat(newplace, file);
                    /*
                     zatrzymanie oryginalnej nazwy pliku
                     */
                    if(rename(file, ptrL)!=-1)
                        printf("Successful\n");
                    else
                        printf("Error:\nDirectory not found in CWD\n");
                    closedir(isD);
                }
            }
        }
    return 1;
    
}



void formatError() {
    
    printf("Usage:\n./mv foo.txt new.txt\n./mv foot.txt subdir\n");
    
}



int sh_launch(char **args) {
  
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        /*
         proces dziecko
         */
        if (execvp(args[0], args) == -1) {
            perror("sh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        /*
         blad podczas forkowania
         */
        perror("sh");
    } else {
        /*
         proces rodzic
         */
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
    
}



int sh_execute(char **args) {
    
    int i;
    
    if (args[0] == NULL) {
        return 1;
    }
    
    for (i = 0; i < sh_num_builtins(); i++) {
      if (strcmp(args[0], builtin_str[i]) == 0) {
        return (*builtin_func[i])(args);
      }
    }

    return sh_launch(args);
    
}



#define SH_RL_BUFSIZE 1024

char *sh_read_line(void) {
    
    int bufsize = SH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

    if (c == EOF) {
        exit(EXIT_SUCCESS);
    } else if (c == '\n') {
        buffer[position] = '\0';
        return buffer;
    } else {
        buffer[position] = c;
    }
    position++;

    /*
     realokacja przy przepelnieniu bufora
     */
    if (position >= bufsize) {
      bufsize += SH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
          fprintf(stderr, "sh: allocation error\n");
          exit(EXIT_FAILURE);
      }
    }
  }
    
}


#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"


char **sh_split_line(char *line) {
    
  int bufsize = SH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
      fprintf(stderr, "sh: allocation error\n");
      exit(EXIT_FAILURE);
  }

  token = strtok(line, SH_TOK_DELIM);
  while (token != NULL) {
      tokens[position] = token;
      position++;

    if (position >= bufsize) {
        bufsize += SH_TOK_BUFSIZE;
        tokens_backup = tokens;
        tokens = realloc(tokens, bufsize * sizeof(char*));
        if (!tokens) {
          free(tokens_backup);
          fprintf(stderr, "sh: allocation error\n");
          exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
    
}



void sh_loop() {
    
     /*
      wyświetlać znak zachęty w postaci [{path}] $, gdzie {path} jest
      ścieżką do bieżącego katalogu roboczego;
      */
    char *line;
    char **args;
    int status;
     
    const char* username = getenv("USER");
    const char* pcname = getenv("COMPUTERNAME");
    const char* path = getenv("PWD");
    
    do {
    printf("[%s%s@%s %s%s]\n%s$%s ", ANSI_COLOR_MAGENTA , username, pcname, ANSI_COLOR_CYAN, path, ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
    line = sh_read_line();
    args = sh_split_line(line);
    status = sh_execute(args);
        
    free(line);
    free(args);
    } while (status);
    
}



int main(int argc, char **argv) {
    
    sh_loop();
    
    return EXIT_SUCCESS;
    
}
