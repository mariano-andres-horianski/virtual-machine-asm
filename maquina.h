#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define MEM 16384
#define ENT 8
#define REG 32


typedef enum {
    LAR = 0,MAR,MBR,IP,OPC,OP1,OP2,
    EAX = 10,EBX,ECX,EDX,EEX,EFX,AC,CC,
    CS = 26,DS } registros;

typedef struct {
    unsigned short base;
    unsigned short tamanio; } infoSegmento;              //unsigned short tiene exact. 2 bytes

void inicioRegistro(uint32_t reg[]);
void inicioTablaSegmento(infoSegmento tabla[],uint16_t tamanio);
void leerEncabezado(char nombre[],uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]);
uint32_t calcDirFisica(infoSegmento tablaSegmento[ENT],uint32_t regIP,int cantBytes);
int operando(uint8_t tipo);
void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]);

void operandos(uint32_t *lectura,uint32_t tipo,uint32_t registros[],uint8_t memoria[],uint32_t *posicion);
void resultado(uint8_t Tipo1,uint32_t registros[],uint8_t memoria[], int resultado,infoSegmento tablaSegmentos[]);
void actualizarCC(uint32_t registros[],uint32_t resultado);
void ADD(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
//void (*instrucciones[26])(void) = {SYS,JMP,JZ,JP,JN,JNZ,JNP,JNN,NOT,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,STOP,MOV,ADD,SUB,MUL,DIV,CMP,SHL,SHR,SAR,AND,OR,XOR,SWAP,LDL,LDH,RND};
