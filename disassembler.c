#include "maquina.h"
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

const char *mnemonicos[32] = { //arreglo de string, estan en orden para que coincidan los indices
    "SYS", "JMP", "JZ", "JP", "JN", "JNZ", "JNP", "JNN", "NOT",
    "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", //no hay mnemonicos para 0x09, 0x0A, ... 0x0F
    "STOP", "MOV", "ADD", "SUB", "MUL", "DIV", "CMP", "SHL",
    "SHR", "SAR", "AND", "OR", "XOR", "SWAP", "LDL", "LDH", "RND"
};
// recorrer memoria
void disassembler(uint8_t memoria[], infoSegmento tablaSegmentos[], uint32_t tamMemoria, uint32_t registros[]) {
    uint32_t dirFisica, operando1, operando2, PC = tablaSegmentos[registros[CS]].base;
    uint8_t instruccion[6],N,tipo1,tipo2,ini1;
    int i;
    while (PC < tablaSegmentos[registros[CS]].base + tablaSegmentos[registros[CS]].tamanio) {
        dirFisica = PC;
        printf("dirFisica: %d",dirFisica);
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

        printf("[%04X] ", dirFisica);
        mostrarHexa(instruccion, 0, N);

        // Completar con espacios para alinear
        for (i = N; i<8; i++)
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
                printf("    %d",(int16_t)operando2);
            else
                if (tipo2==1)
                    printf("    %s",nombresRegistros[operando2]);


        printf("\n");
        PC++;
    }
}
