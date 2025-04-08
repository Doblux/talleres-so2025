#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "constants.h"

int generate_random_number(){
	return (rand() % 50);
}


void proceso_hijo(int childNumber, int n, int pipes[][2], int pipe_padre[][2], int start){
	for (size_t i = 0; i < n; i++){
		if ((i != childNumber)&&(i != (childNumber + 1)%n)){
			close(pipes[i][PIPE_READ]);
			close(pipes[i][PIPE_WRITE]);
		}
	}
	close(pipes[childNumber][PIPE_WRITE]);
	close(pipes[(childNumber + 1) % n][PIPE_READ]);

	int s;
	if (childNumber == start)
	{
		read(pipe_padre[0][PIPE_READ], &s, sizeof(int));
		printf("hijo numero %d, recibió %d\n", childNumber, s);
		int secret = generate_random_number();
		printf("numero secreto generado: %d\n", secret);
		s++;
		write(pipes[(childNumber+1)%n][PIPE_WRITE], &s, sizeof(int));
		while (read(pipes[childNumber][PIPE_READ], &s, sizeof(int)))
		{
			printf("hijo numero %d, recibió %d\n", childNumber, s);
			if (s > secret)
			{
				write(pipe_padre[1][PIPE_WRITE], &s, sizeof(int));
				close(pipe_padre[0][PIPE_READ]);
				close(pipe_padre[0][PIPE_WRITE]);
				close(pipe_padre[1][PIPE_READ]);
				close(pipe_padre[1][PIPE_WRITE]);
				close(pipes[(childNumber+1)%n][PIPE_WRITE]);
				exit(EXIT_SUCCESS);
			} else {
				s++;
				write(pipes[(childNumber+1)%n][PIPE_WRITE], &s, sizeof(int));
			}
		}
	} else {
		close(pipe_padre[0][PIPE_READ]);
		close(pipe_padre[0][PIPE_WRITE]);
		close(pipe_padre[1][PIPE_READ]);
		close(pipe_padre[1][PIPE_WRITE]);
		while (read(pipes[childNumber][PIPE_READ], &s, sizeof(int)))
		{
			printf("hijo numero %d, recibió %d\n", childNumber, s);
			s++;
			write(pipes[(childNumber+1)%n][PIPE_WRITE], &s, sizeof(int));
		}
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char **argv)
{	
	//Funcion para cargar nueva semilla para el numero aleatorio
	srand(time(NULL));

	int status, pid, n, start, buffer;
	n = atoi(argv[1]); // cant hijos
	buffer = atoi(argv[2]); // mensaje inicial
	start = atoi(argv[3]); // numero de proceso que inicia

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
  	/* COMPLETAR */
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer, start);
    int pipes[n][2];
	int pipe_padre[2][2];
	for (size_t i = 0; i < n; i++){
		pipe(pipes[i]);
	}
	pipe(pipe_padre[0]);
	pipe(pipe_padre[1]);

	for (size_t i = 0; i < n; i++){
		if (fork() == 0){
			proceso_hijo(i, n, pipes, pipe_padre, start);
		}
	}
	
	for (size_t i = 0; i < n; i++)
	{
		close(pipes[i][PIPE_READ]);
		close(pipes[i][PIPE_WRITE]);
	}
	
    write(pipe_padre[0][PIPE_WRITE], &buffer, sizeof(int));
	int result;
	read(pipe_padre[1][PIPE_READ], &result, sizeof(int));
	for (size_t i = 0; i < n; i++)
	{
		wait(NULL);
	}

	printf("resultado %d\n", result);
	return 0;
}
