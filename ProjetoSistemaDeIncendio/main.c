#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <locale.h>
#include "sensor_system.h"

char floresta[SIZE][SIZE];
pthread_mutex_t floresta_mutex[SIZE][SIZE];
pthread_cond_t fogo_cond[SIZE][SIZE];
pthread_mutex_t central_mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {

    setlocale(LC_ALL, "Portuguese");
    srand(time(NULL));

    // Inicializar a floresta
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            floresta[i][j] = (rand() % 4 == 0) ? NODOSENSOR : VAZIO;
            pthread_mutex_init(&floresta_mutex[i][j], NULL);
            pthread_cond_init(&fogo_cond[i][j], NULL);
        }
    }

    exibir_floresta();

    // Criar threads dos sensores
    pthread_t sensores_threads[SIZE][SIZE];
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (floresta[i][j] == NODOSENSOR) {
                Sensor* novo_sensor = (Sensor*)malloc(sizeof(Sensor));
                novo_sensor->posX = i;
                novo_sensor->posY = j;
                pthread_create(&sensores_threads[i][j], NULL, funcao_sensor, (void*)novo_sensor);
            }
        }
    }

    // Criar thread para geração de incêndios
    pthread_t thread_gerador_incendio;
    pthread_create(&thread_gerador_incendio, NULL, gerar_incendio, NULL);

    // Criar thread para o centro de controle
    pthread_t thread_centro_controle;
    pthread_create(&thread_centro_controle, NULL, centro_de_controle, NULL);

    // Aguardar que as threads terminem (o programa continuará em execução até ser encerrado manualmente)
    pthread_join(thread_gerador_incendio, NULL);
    pthread_join(thread_centro_controle, NULL);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (floresta[i][j] == NODOSENSOR) {
                pthread_join(sensores_threads[i][j], NULL);
            }
        }
    }

    return 0;
}
