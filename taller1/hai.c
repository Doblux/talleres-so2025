#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <syscall.h>
#include <signal.h>
#include <time.h>

int n = 5;
char** args_varios;

void sigint_handler(){
    wait(NULL);
}

void sigurg_handler(int sig){
    write(STDOUT_FILENO, "ya va!\n", 7);
    n--;
    if ( n == 0){
        pid_t padre = getppid();
        kill(padre,SIGINT);
        execvp(args_varios[0], args_varios);
        perror("error de ejecucion exec");
    }
}

void hijo(char* binario){
    while (1)
    {
        wait(NULL);
    }
    
}
int main(int argc, const char* argv[]){
    char** temp = malloc(sizeof(char*) * (argc - 1));
    for (int i = 0; i < (argc - 1); i++)
    {
        temp[i] = argv[i+1];
    }
    args_varios = temp;
    signal(SIGURG, sigurg_handler);
    pid_t hojo = fork();
    
    if (hojo == 0){
        hijo(argv[1]);
    }
    signal(SIGINT,sigint_handler);
    for (int i = 0; i < 5; i++){
        write(STDOUT_FILENO, "Sup!\n", 5);
        kill(hojo, SIGURG);
        sleep(1);

    }
    return 0;
}