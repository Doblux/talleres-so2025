#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int main() {
	int server_socket;
	struct sockaddr_un server_addr;

	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, "unix_socket");

	server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("Error: no se ha podido conectar al servidor");
		exit(EXIT_FAILURE);
	}
	
	char* formula = NULL;
	size_t buffer_size = 0;
	ssize_t read_size;

	int conectado = 1;
	while (conectado){
		read_size = getline(&formula, &buffer_size, stdin);
		printf("readsize %d\n",read_size);
		printf("%s", formula);
		if (strcmp(formula, "exit") == 0){
			conectado = 0;
		}else {
			int resultado;
			printf("envia la formula: %s\n", formula);
			printf("conectado: %d\n", conectado);
			if (write(server_socket, &read_size, sizeof(int)) == -1){
				printf("no puede escribir el tamaño\n");
			}
			if(write(server_socket, &formula, read_size) == -1){
				printf("no puede escribir la formula\n");
			}
			if(read(server_socket, &resultado, sizeof(int)) == -1){
				printf("no puede leer el resultado\n");
			}
			printf("resultado: %d\n", resultado);
		}
	}
	exit(EXIT_SUCCESS);
}
