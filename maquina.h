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
void calcDirFisica(infoSegmento tablaSegmento[ENT],uint32_t regIP,int cantBytes);
void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]);
void set(uint32_t registros[], uint8_t memoria[], uint32_t operando1, uint32_t operando2);
uint32_t get(uint32_t operando,uint32_t registros[], uint8_t memoria[]);

void leerInstrucciones(uint8_t instruccion, uint8_t memoria[], uint32_t registros[REG]);
void operandos(uint32_t *lectura,uint32_t tipo,uint32_t registros[],uint8_t memoria[]);
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
void ADD(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void MOV(uint32_t registros[], uint8_t memoria[], uint32_t operando1, uint32_t operando2);
void SUB(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void MUL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void DIV(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void CMP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void SHL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void SHR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void SAR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void AND(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void OR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void XOR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void SWAP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void LDL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void LDH(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void NOT(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void STOP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);
void NO_ACCESIBLE(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]);