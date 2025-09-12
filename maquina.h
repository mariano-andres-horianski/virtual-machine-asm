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
void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]);
uint32_t get(uint8_t tipo_operando, uint32_t operando,uint32_t registros[], uint8_t memoria[]);

void leerInstrucciones(uint8_t instruccion, uint8_t memoria[], uint32_t registros[REG]);
void operandos(uint32_t *lectura,uint32_t tipo,uint32_t registros[],uint8_t memoria[],uint32_t *posicion);
void ResultadoOperacion(uint8_t Tipo1,uint32_t registros[],uint8_t memoria[], int resultado,infoSegmento tablaSegmentos[]);
void actualizarCC(uint32_t registros[],uint32_t resultado);
//un parametro
void JMP(uint32_t registros[]);
void JZ(uint32_t registros[]);
void JP(uint32_t registros[]);
void JN(uint32_t registros[]);
void JNZ(uint32_t registros[]);
void JNP(uint32_t registros[]);
void JNN(uint32_t registros[]);
//dos parametros
void ADD(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void SUB(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void MUL(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void DIV(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void CMP(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void SHL(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void SHR(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void SAR(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void AND(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void OR(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void XOR(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void SWAP(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void LDL(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void LDH(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void NOT(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void STOP(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void NO_ACCESIBLE(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);