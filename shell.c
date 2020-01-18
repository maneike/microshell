#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void sh_loop() {
    /*
    char *line;
    char **args;
    int status;
    */
    const char* username = getenv("USER");
    const char* pcname = getenv("COMPUTERNAME");
    const char* path = getenv("PWD");
    
    printf("%s@%s %s $ ", username, pcname, path);
}

int main(int argc, char **argv) {
    
    sh_loop();
    
    return EXIT_SUCCESS;
}
