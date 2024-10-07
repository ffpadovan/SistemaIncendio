#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "sensor_system.h"

#define RESET_COLOR "\x1b[0m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GRAY "\x1b[90m"
#define COLOR_BLUE "\x1b[34m"

extern char floresta[SIZE][SIZE];
extern pthread_mutex_t floresta_mutex[SIZE][SIZE];
extern pthread_cond_t fogo_cond[SIZE][SIZE];
extern pthread_mutex_t central_mutex;

void exibir_floresta() {
    pthread_mutex_lock(&central_mutex);
    printf("Estado atual da floresta:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char celula = floresta[i][j];
            switch (celula) {
                case VAZIO:
                    printf(COLOR_GRAY "%c " RESET_COLOR, celula);
                    break;
                case NODOSENSOR:
                    printf(COLOR_GREEN "%c " RESET_COLOR, celula);
                    break;
                case FOGO:
                    printf(COLOR_RED "%c " RESET_COLOR, celula);
                    break;
                case QUEIMADO:
                    printf(COLOR_BLUE "%c " RESET_COLOR, celula);
                    break;
                default:
                    printf("%c ", celula);
            }
        }
        printf("\n");
    }
    printf("\n");
    pthread_mutex_unlock(&central_mutex);
}

void* funcao_sensor(void* arg) {
    Sensor* sensor = (Sensor*)arg;

    while (1) {
        pthread_mutex_lock(&floresta_mutex[sensor->posX][sensor->posY]);
        if (floresta[sensor->posX][sensor->posY] == FOGO) {
            printf("Sensor [%d, %d] detectou fogo!\n", sensor->posX, sensor->posY);

            // Propagar o fogo para sensores adjacentes
            if (sensor->posX > 0) {
                pthread_mutex_lock(&floresta_mutex[sensor->posX - 1][sensor->posY]);
                if (floresta[sensor->posX - 1][sensor->posY] == NODOSENSOR) {
                    floresta[sensor->posX - 1][sensor->posY] = FOGO;
                }
                pthread_mutex_unlock(&floresta_mutex[sensor->posX - 1][sensor->posY]);
            }
            if (sensor->posX < SIZE - 1) {
                pthread_mutex_lock(&floresta_mutex[sensor->posX + 1][sensor->posY]);
                if (floresta[sensor->posX + 1][sensor->posY] == NODOSENSOR) {
                    floresta[sensor->posX + 1][sensor->posY] = FOGO;
                }
                pthread_mutex_unlock(&floresta_mutex[sensor->posX + 1][sensor->posY]);
            }
            if (sensor->posY > 0) {
                pthread_mutex_lock(&floresta_mutex[sensor->posX][sensor->posY - 1]);
                if (floresta[sensor->posX][sensor->posY - 1] == NODOSENSOR) {
                    floresta[sensor->posX][sensor->posY - 1] = FOGO;
                }
                pthread_mutex_unlock(&floresta_mutex[sensor->posX][sensor->posY - 1]);
            }
            if (sensor->posY < SIZE - 1) {
                pthread_mutex_lock(&floresta_mutex[sensor->posX][sensor->posY + 1]);
                if (floresta[sensor->posX][sensor->posY + 1] == NODOSENSOR) {
                    floresta[sensor->posX][sensor->posY + 1] = FOGO;
                }
                pthread_mutex_unlock(&floresta_mutex[sensor->posX][sensor->posY + 1]);
            }

            pthread_cond_signal(&fogo_cond[sensor->posX][sensor->posY]);
        }
        pthread_mutex_unlock(&floresta_mutex[sensor->posX][sensor->posY]);
        usleep(500000);
    }

    return NULL;
}

void* gerar_incendio(void* arg) {
    while (1) {
        int x = rand() % SIZE;
        int y = rand() % SIZE;

        pthread_mutex_lock(&floresta_mutex[x][y]);
        if (floresta[x][y] == NODOSENSOR) {
            floresta[x][y] = FOGO;
            printf("Incêndio iniciado em [%d, %d]!\n", x, y);
            exibir_floresta();
        }
        pthread_mutex_unlock(&floresta_mutex[x][y]);

        sleep(3);
    }
    return NULL;
}

void* centro_de_controle(void* arg) {
    while (1) {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                pthread_mutex_lock(&floresta_mutex[i][j]);
                if (floresta[i][j] == FOGO) {
                    printf("Centro de Controle: Combate ao fogo em [%d, %d]\n", i, j);

                    // Apagar o fogo nas células adjacentes
                    if (i > 0 && floresta[i - 1][j] == FOGO) {
                        floresta[i - 1][j] = QUEIMADO;
                    }
                    if (i < SIZE - 1 && floresta[i + 1][j] == FOGO) {
                        floresta[i + 1][j] = QUEIMADO;
                    }
                    if (j > 0 && floresta[i][j - 1] == FOGO) {
                        floresta[i][j - 1] = QUEIMADO;
                    }
                    if (j < SIZE - 1 && floresta[i][j + 1] == FOGO) {
                        floresta[i][j + 1] = QUEIMADO;
                    }

                    floresta[i][j] = QUEIMADO;
                    exibir_floresta();
                }
                pthread_mutex_unlock(&floresta_mutex[i][j]);
            }
        }
        sleep(1);
    }
    return NULL;
}
