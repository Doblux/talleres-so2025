#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/wait.h>


void sigchld_handler(int sig){
    wait(NULL);
}

void sigint_handler(int sig){
    printf("\n\t[+] Saliendo ... \n");
    exit(EXIT_SUCCESS);
}

int calcular(const char *expresion) {
    int num1, num2, resultado;
    char operador;

    // Usamos sscanf para extraer los dos números y el operador de la expresión
    if (sscanf(expresion, "%d%c%d", &num1, &operador, &num2) != 3) {
        printf("%s\n", expresion);
        printf("Formato incorrecto\n");
        return 0;  // En caso de error, retornamos 0.
    }

    // Realizamos la operación según el operador
    switch (operador) {
        case '+':
            resultado = num1 + num2;
            break;
        case '-':
            resultado = num1 - num2;
            break;
        case '*':
            resultado = num1 * num2;
            break;
        case '/':
            if (num2 != 0) {
                resultado = num1 / num2;
            } else {
                printf("Error: División por cero\n");
                return 0;  // Si hay división por cero, retornamos 0.
            }
            break;
        default:
            printf("Operador no reconocido\n");
            return 0;  // Si el operador no es válido, retornamos 0.
    }

    return resultado;
}

void atender_cliente(int client_socket){
    char* formula;
    char salir[5] = "exit";
    int mantener_comunicacion = 1;
    int tamaño;
    while (mantener_comunicacion) {
        printf("\nesperando a que manden mensajes\n");
        recv(client_socket, &tamaño, sizeof(int), 0);
        printf("tamaño: %d\n", tamaño);
        formula = (char*)malloc((tamaño+1) * sizeof(char));
        recv(client_socket, formula, tamaño, 0);
        printf("la fomula recibida es: %s\n", formula);
        sleep(1);
        if (strcmp(formula, salir) == 0){
            printf("la comunicacion se ha cerrado\n");
            mantener_comunicacion = 0;
        } else {
            int result = calcular(formula);
            send(client_socket, &result, sizeof(result), 0);
            free(formula);
        }
        sleep(1);
    }
    close(client_socket);
    exit(EXIT_SUCCESS);
        
}

int main() {
    int server_socket;
    int client_socket;
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;
    uint slen = sizeof(server_addr);
    uint clen = sizeof(client_addr);
    // COMPLETAR. Este es un ejemplo de funcionamiento básico.
    // La expresión debe ser recibida como un mensaje del cliente hacia el servidor.

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, "unix_socket");
    unlink(server_addr.sun_path);

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    bind(server_socket, (struct sockaddr *) &server_addr, slen);
    listen(server_socket, 1);

    printf("Servidor: esperando conexion del cliente\n");
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &clen);
        printf("esperando conectar\n");
        if (fork() == 0){
            atender_cliente(client_socket);
        }

        close(client_socket);
    }
    

    const char *expresion = "10+5";
    int resultado = calcular(expresion);
    printf("El resultado de la operación es: %d\n", resultado);
    exit(0);
}

