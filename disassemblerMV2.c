#include "maquina.h"
const char *nombresRegistros[REG] = {
    [LAR] = "LAR",
    [MAR] = "MAR",
    [MBR] = "MBR",
    [IP]  = "IP",
    [OPC] = "OPC",
    [OP1] = "OP1",
    [OP2] = "OP2",
    [SP]  = "SP",
    [BP]  = "BP",

    [EAX] = "EAX",
    [EBX] = "EBX",
    [ECX] = "ECX",
    [EDX] = "EDX",
    [EEX] = "EEX",
    [EFX] = "EFX",
    [AC]  = "AC",
    [CC]  = "CC",

    [CS]  = "CS",
    [DS]  = "DS",
    [ES]  = "ES",
    [SS]  = "SS",
    [KS]  = "KS",
    [PS]  = "PS"
};

const char *mnemonicos[32] = { //arreglo de string, estan en orden para que coincidan los indices
    "SYS", "JMP", "JZ", "JP", "JN", "JNZ", "JNP", "JNN", "NOT",
    "XXX", "XXX", "PUSH", "POP", "CALL", "RET", //no hay mnemonicos para 0x09 y 0x0A
    "STOP", "MOV", "ADD", "SUB", "MUL", "DIV", "CMP", "SHL",
    "SHR", "SAR", "AND", "OR", "XOR", "SWAP", "LDL", "LDH", "RND"
};

void disassemblerMV2(uint8_t memoria[], infoSegmento tablaSegmentos[], uint32_t registros[]) {
    uint32_t i, j, dirFisica;
    uint32_t baseConst= tablaSegmentos[KS].base;
    uint32_t tamConst= tablaSegmentos[KS].tamanio;
    uint32_t baseCode= tablaSegmentos[CS].base;
    uint32_t tamCode= tablaSegmentos[CS].tamanio;
    uint32_t entry= registros[IP] & 0x0000FFFF;

    //cadenas
    uint8_t hex[8];
    uint8_t ascii[99];
    int len;

    //instrucciones
    uint32_t PC;
    uint8_t opcode;
    uint8_t tipo2;
    uint8_t tipo1;
    uint8_t op;
    uint8_t N; // tamaño total de la instruccion
    uint8_t instruccion[6];
    uint32_t operando1, operando2, sector; 
    char car;

    // Recorrer Const Segment y mostrar las cadenas
    i = baseConst;
    while (i < baseConst + tamConst) {
        printf("[%04X] ", i);
        len = 0;

        // Copiar bytes hasta encontrar '\0' o maximo 7
        while (i + len < baseConst + tamConst && memoria[i + len] != 0x00 && len < 7) {
            hex[len] = memoria[i + len];
            ascii[len] = (memoria[i + len] >= 32 && memoria[i + len] <= 126) ? memoria[i + len] : '.'; //se fija si es imprimible y si no pone un punto
            len++;
        }

        // Mostrar cadena en hexadecimal
        for (j = 0; j < len; j++) 
            printf("%02X ", hex[j]);
        if (i + len < baseConst + tamConst && memoria[i + len] == 0x00)
            printf("00 "); //agrega el cero del final
        else 
            if (len == 7)
                printf(".. "); //si el numero final no es cero, muestra ".." al final (se pasa de los 7 bytes)

        // Espaciado para alinear
        for (j = len; j < 7; j++) 
            printf("   ");

        //sigo copiando la cadena si no se llego a \0
        if (len==7 && (i + len < baseConst + tamConst) && memoria[i + len] != 0x00){
            while (i + len < baseConst + tamConst && memoria[i + len] != 0x00) {
                ascii[len] = (memoria[i + len] >= 32 && memoria[i + len] <= 126) ? memoria[i + len] : '.'; //se fija si es imprimible y si no pone un punto
                len++;
            }
        }
        ascii[len] = '\0'; //me aseguro de no tener errores con %s 

        // Mostrar cadena en ascii
        printf("| \"%s\"\n", ascii); // \" para mostrar las comillas de inicio. \" para mostrar las comillas del final y salta de linea

        // Avanzar hasta despues del '\0'
        while (i < baseConst + tamConst && memoria[i] != 0x00) 
            i++;
        i++; // salta el \0
    }

    // Code Segment, desensamblar instrucciones
    PC = baseCode;
    while (PC < baseCode + tamCode) {
        dirFisica = PC;
        opcode = memoria[PC];
        tipo2 = opcode >> 6;
        tipo1 = (opcode >> 4) & 0x03;
        op = opcode & 0x1F;
        N = 1 + tipo1 + tipo2; // tamaño total de la instruccion
        instruccion[0] = opcode;

        // Leer los bytes de la instruccion
        i=1;
        while (i < N && (PC + i) < baseCode + tamCode){
            instruccion[i] = memoria[PC + i];
            i++;
        }

        // Mostrar direccion y bytes
        if ((dirFisica - baseCode) == entry) 
            printf(">");
        else 
            printf(" ");
        printf("[%04X] ", dirFisica);
        i=0;
        while (i<N){
            printf("%02X ", instruccion[i]);
            i++;
        }
        for (; i < 8; i++) 
            printf("   "); // alineacion

        printf("|  %s", (op < 32) ? mnemonicos[op] : "???");

        // Calcular operandos
        operando1 = 0;
        operando2 = 0;
        for (i = 1; i <= tipo2; i++)
            operando2 = (operando2 << 8) | instruccion[i];
        uint8_t ini1 = 1 + tipo2;
        for (i = ini1; i < N; i++)
            operando1 = (operando1 << 8) | instruccion[i];

        // Mostrar operandos
        if (tipo1) {
            printf("    ");
            sector=operando1;
            operando1=operando1 << 2;
            operando1=operando1 >> 2; //como es unsigned pone cero en los primeros 2 bits
            if (tipo1 == 3){ //memoria
                sector=sector >> 22;
                switch(sector){
                    case 0: printf("l");
                            break;
                    case 2: printf("w");
                            break;
                    case 3: printf("b");
                            break;
                }
                printf("[%s+%u]", nombresRegistros[(operando1 >> 8) & 0x1F], operando1 & 0xFF);
            }
            else 
                if (tipo1 == 1){ //registro
                    if (operando1>=EAX&&operando1<=EFX){
                        sector=sector >> 6;
                        car=nombresRegistros[operando1][1];
                        switch(sector){
                            case 0: printf("%s", nombresRegistros[operando1]);
                                    break;
                            case 1: printf("%cL",car);
                                    break;
                            case 2: printf("%cH",car);
                                    break;
                            case 3: printf("%cX",car);
                                    break;
                        }
                    }
                    else
                        printf("%s", nombresRegistros[operando1]);    
                }
                else 
                    if (tipo1 == 2) //inmediato
                        printf("%d", (int16_t)operando1);
        }

        if (tipo2) {
            if (tipo1)
                printf(", ");
            sector=operando2;
            operando2=operando2 << 2;
            operando2=operando2 >> 2;
            if (tipo2 == 3){ //memoria
                sector=sector >> 22;
                switch(sector){
                    case 0: printf("l");
                            break;
                    case 2: printf("w");
                            break;
                    case 3: printf("b");
                            break;
                }
                printf("[%s+%u]", nombresRegistros[(operando2 >> 8) & 0x1F], operando2 & 0xFF);
            }
            else 
                if (tipo2 == 1){ //registro
                    if (operando2>=EAX&&operando2<=EFX){
                        sector=sector >> 6;
                        car=nombresRegistros[operando2][1];
                        switch(sector){
                            case 0: printf("%s", nombresRegistros[operando2]);
                                    break;
                            case 1: printf("%cL",car);
                                    break;
                            case 2: printf("%cH",car);
                                    break;
                            case 3: printf("%cX",car);
                                    break;
                        }
                    }
                    else
                        printf("%s", nombresRegistros[operando2]);    
                }
                else 
                    if (tipo2 == 2) //inmediato
                        printf("%d", (int16_t)operando2);
        }
        printf("\n");
        PC += N;
    }
        
}
