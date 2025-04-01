#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <syscall.h>
#include <signal.h>
#include <time.h>

int n, j;
pid_t* children;

void jugarHijo(){
    srand((int)getpid());
    while (1){
        wait(NULL);
    }
}


void sigchld_handler(int sig){
    pid_t hijo = wait(NULL);
    for (int i = 0; i < n; i++){
        if (children[i] == hijo){
            children[i] = -1;
            break;
        }
    }
    
}

void sigterm_handler(int sig){
    int numero = rand()%n;
    //printf("numero: %d\n", numero);
    if (numero == j){
        printf("me mueeeeerroooo\n");
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, const char* argv[]){
    if (argc != 4) {
        printf("error: parametros incorrectos\n");
        printf("\tuso: n, k, j\n");
    }

    
    int k;
    n = atoi(argv[1]);
    k = atoi(argv[2]);
    j = atoi(argv[3]);
    printf("%d %d %d\n", n, k, j);
    
    children = malloc(n * sizeof(pid_t));
    signal(SIGTERM, sigterm_handler);
    signal(SIGCHLD, sigchld_handler);
    printf("va a hacer forks\n");
    for (int i = 0; i < n; i++) {
        children[i] = fork();
        if (children[i] == 0)
        {
            jugarHijo();
        }
    }
    printf("rondas\n");
    for (int i = 0; i < k; i++){    
        for (int hijo = 0; hijo < n; hijo++){
            sleep(1);
            kill(children[hijo], SIGTERM);   
        }
    }
    printf("anuncia ganadores \n");
    for (int i = 0; i < n; i++){
        if (children[i] != -1){
            printf("ganó el hijo con PID %d \n", children[i]);
            kill(children[i], SIGKILL);
        }
    }
    
    
    free(children);
    return 0;
}