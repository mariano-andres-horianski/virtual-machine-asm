#include <stdio.h>
#include <stdint.h>
#include "maquina.h"

const char *mnemonicos[32] = { //arreglo de string, estan en orden para que coincidan los indices
    "SYS", "JMP", "JZ", "JP", "JN", "JNZ", "JNP", "JNN", "NOT",
    "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", //no hay mnemonicos para 0x09, 0x0A, ... 0x0F
    "STOP", "MOV", "ADD", "SUB", "MUL", "DIV", "CMP", "SHL",
    "SHR", "SAR", "AND", "OR", "XOR", "SWAP", "LDL", "LDH", "RND"
};

const char *nombresRegistros[REG] = {
    [LAR] = "LAR",
    [MAR] = "MAR",
    [MBR] = "MBR",
    [IP]  = "IP",
    [OPC] = "OPC",
    [OP1] = "OP1",
    [OP2] = "OP2",

    [EAX] = "EAX",
    [EBX] = "EBX",
    [ECX] = "ECX",
    [EDX] = "EDX",
    [EEX] = "EEX",
    [EFX] = "EFX",
    [AC]  = "AC",
    [CC]  = "CC",

    [CS]  = "CS",
    [DS]  = "DS"
};

// Las posiciones no inicializadas automáticamente quedan como NULL

static void mostrarHexa(uint8_t instruccion[], uint8_t inicio, uint8_t fin) {
    uint8_t i;
    for (i = inicio; i < fin; i++) {
        printf("%02X ", instruccion[i]);
    }
}

// recorrer memoria
void disassembler(uint8_t memoria[], infoSegmento tablaSegmentos[], uint32_t tamMemoria) {
    uint32_t dirFisica, operando1, operando2, PC = tablaSegmentos[CS].base;
    uint8_t instruccion[6],i,N,tipo1,tipo2,ini1;
    
    while (PC < tablaSegmentos[CS].base + tablaSegmentos[CS].tamanio) {
        dirFisica = PC;
        i=0;
        N=0;
        instruccion[i] = memoria[PC];
        tipo1=(instruccion[i]>>4)&0x03;
        tipo2=instruccion[i]>>6;
        N=1+tipo1+tipo2; //size de la instruccion
        for (i=1;i<N;i++){
            PC++;
            instruccion[i]=memoria[PC];
        }

        //calcular direccion fisica?
        printf("[%04X] ", dirFisica);
        mostrarHexa(instruccion, 0, N);

        // Completar con espacios para alinear
        for (int i = N; i<8; i++) 
            printf("    ");

        // Mostrar mnemonico
        instruccion[0]=instruccion[0]&0x1F;
        printf(" |  %s", mnemonicos[instruccion[0]]);

        operando2=0;
        operando1=0;
        for (i=1; i<=tipo2; i++){
            operando2=operando2<<8;
            operando2=operando2|instruccion[i];
        }
        ini1=tipo2+1;
        for (i=ini1; i<N; i++){
            operando1=operando1<<8;
            operando1=operando1|instruccion[i];
        }

        // Mostrar operandos
        if (tipo1==3)
            printf("    [%s + %u]",nombresRegistros[(operando1&0x00FF0000)>>16],operando1&0x0000FFFF);
        else
            if (tipo1==1)
                printf("    %s",nombresRegistros[operando1]); //el tipo del operando 1 nunca puede ser 2 (inmediato)
        
        if (tipo2==3)
            printf("    [%s + %u]",nombresRegistros[(operando2&0x00FF0000)>>16],operando2&0x0000FFFF);
        else
            if (tipo2==2)
                printf("    %d",operando2);
            else
                if (tipo2==1)
                    printf("    %s",nombresRegistros[operando2]);


        printf("\n");
        PC++;
    }
}
