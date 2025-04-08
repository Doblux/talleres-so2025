#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "constants.h"
#include "mini-shell-parser.c"

void ejecutar_hijo(int child_number, int count, char*** progs, int pipes[][2]){
	for(int i=0; i<count-1; i++){
		if ( (i != child_number) && (i != child_number-1) ){
			close(pipes[i][PIPE_READ]);
			close(pipes[i][PIPE_WRITE]);
		}
	}
	if (child_number == 0){
		close(pipes[child_number][PIPE_READ]);
		dup2(pipes[child_number][PIPE_WRITE], STD_OUTPUT);
	} else if (child_number == count -1 ) {
		close(pipes[child_number-1][PIPE_WRITE]);
		dup2(pipes[child_number-1][PIPE_READ], STD_INPUT);
	} else {
		close(pipes[child_number][PIPE_READ]);
		dup2(pipes[child_number][PIPE_WRITE], STD_OUTPUT);
		close(pipes[child_number-1][PIPE_WRITE]);
		dup2(pipes[child_number-1][PIPE_READ], STD_INPUT);
	}

	execvp(progs[child_number][0], progs[child_number]);
}


static int run(char ***progs, size_t count)
{	
	int r, status;

	//Reservo memoria para el arreglo de pids
	//TODO: Guardar el PID de cada proceso hijo creado en children[i]
	pid_t *children = malloc(sizeof(*children) * count);

	//TODO: Pensar cuantos procesos necesito
	//TODO: Pensar cuantos pipes necesito.
	int pipes[count-1][2];
	for (size_t i = 0; i < count-1; i++) {
		pipe(pipes[i]);
	}
	
	for (size_t i = 0; i < count; i++) {
		children[i] = fork();
		if (children[i] == 0){
			ejecutar_hijo(i, count, progs, pipes);
		}
	}
	
	for (size_t i = 0; i < count-1; i++)
	{
		close(pipes[i][PIPE_READ]);
		close(pipes[i][PIPE_WRITE]);
	}
	


	//TODO: Para cada proceso hijo:
			//1. Redireccionar los file descriptors adecuados al proceso
			//2. Ejecutar el programa correspondiente

	//Espero a los hijos y verifico el estado que terminaron
	for (int i = 0; i < count; i++) {
		waitpid(children[i], &status, 0);

		if (!WIFEXITED(status)) {
			fprintf(stderr, "proceso %d, hijo %d, no terminó correctamente [%d]: ",
			    (int)children[i], i, WIFSIGNALED(status));
			perror("");
			return -1;
		}
	}
	r = 0;
	free(children);

	return r;
}


int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("El programa recibe como parametro de entrada un string con la linea de comandos a ejecutar. \n"); 
		printf("Por ejemplo ./mini-shell 'ls -a | grep anillo'\n");
		return 0;
	}
	int programs_count;
	char*** programs_with_parameters = parse_input(argv, &programs_count);

	printf("status: %d\n", run(programs_with_parameters, programs_count));

	fflush(stdout);
	fflush(stderr);

	return 0;
}
