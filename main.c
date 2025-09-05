#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MEM 16384
#define ENT 8
#define REG 32

typedef enum {
    LAR = 0,MAR,MBR,IP,OPC,OP1,OP2,
    EAX = 10,EBX,ECX,EDX,EEX,EFX,AC,CC,
    CS = 26,DS } registro;

typedef struct {
    unsigned short base;
    unsigned short tamano; } infoSegmento;              //unsigned short tiene exact. 2 bytes

int main()
{
    char memoria[MEM];                         // memoria de 16 kib
    infoSegmento tablaSegmento[ENT];                    // tabla de segmentos sin inicializar
    uint32_t registros[REG];                            // cada pos tiene disponible 4 bytes >> con unsigned char tiene 1 byte

    if(leerEncabezado()){
        inicioRegistro(registros);
        inicioTablaSegmento(tablaSegmento);
    }

    return 0;
}

/// FUNCIONES
void inicioRegistro(uint32_t reg[]){
    reg[CS] = 0x00000000;
    reg[DS] = 0x00010000;
    reg[IP] = reg [CS];
}
void inicioTablaSegmento(infoSegmento tabla[]){
    tabla[0].base = 0x0000;
    tabla[0].tamano = 0x0000;

    tabla[1].base = 0x0001;
    tabla[1].tamano = 0x0000;
}
int leerEncabezado(){

}
