#ifndef SENSOR_SYSTEM_H_INCLUDED
#define SENSOR_SYSTEM_H_INCLUDED

#define SIZE 30

#define VAZIO '-'
#define NODOSENSOR 'S'
#define FOGO '*'
#define QUEIMADO 'X'

typedef struct {
    int posX;
    int posY;
} Sensor;

void exibir_floresta();
void* funcao_sensor(void* arg);
void* gerar_incendio(void* arg);
void* centro_de_controle(void* arg);


#endif // SENSOR_SYSTEM_H_INCLUDED
