#include "maquina.h"
void inicioRegistro(uint32_t reg[]){
    reg[CS] = 0x00000000;
    reg[DS] = 0x00010000;
    reg[IP] = reg [CS];
}
void inicioTablaSegmento(infoSegmento tabla[],uint16_t tamanioCod){
    tabla[0].base = 0x0000;
    tabla[0].tamanio = tamanioCod;

    tabla[1].base = tamanioCod;
    tabla[1].tamanio = MEM - tamanioCod;
}
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
void (*instrucciones[32])(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]) =
{SYS,JMP,JZ,JP,JN,JNZ,JNP,JNN,NOT,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,STOP,MOV,ADD,SUB,MUL,DIV,CMP,SHL,SHR,SAR,AND,OR,XOR,SWAP,LDL,LDH,RND};

static void mostrarHexa(uint8_t instruccion[], uint8_t inicio, uint8_t fin) {
    uint8_t i;
    for (i = inicio; i < fin; i++) {
        printf("%02X ", instruccion[i]);
    }
}

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

        //calcular direccion fisica?
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

void SYS(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    
    uint16_t cantBytes = (registros[ECX] >> 16) & 0x0000FFFF;
    uint16_t cantCeldas = (registros[ECX]) & 0x0000FFFF, direccion;
    uint8_t modo_lectura = registros[EAX],byteActual;
    int64_t valor;
    int i,b,j,caracter;
    if(get(registros[OP1],registros,memoria,tablaSegmentos) == 0x1){ //lectura
        //guarda en memoria
        for(i = 0; i<cantCeldas; i++){
            if(scanf("%lld", &valor) != 1) {
                printf("ERROR: Entrada inválida\n");
                STOP(registros, memoria, tablaSegmentos);
                return;
            }
            for(j = 0; j < cantBytes; j++){
                byteActual =  (valor >> ((cantBytes - 1 - j) * 8)) & 0xFF;
                //llamamos a operacion memoria y no a set para no escribir exclusivamente 4 bytes de golpe
                operacion_memoria(registros,memoria,registros[EDX]+i*cantBytes+j, byteActual, ESCRITURA, 1, tablaSegmentos);
            }
            printf("[%04x]: ",(registros[MAR] & 0x0000FFFF) - cantBytes + 1);
            if((modo_lectura & 0x10) != 0){
                // binario
                for(b = 0; b < cantBytes; b++){
                    printf("%d ",valor >> ((cantBytes - 1 - b) * 8) & 0x01);
                }
                printf(" ");
            }
            if((modo_lectura & 0x08) != 0){
                //hexa
                printf("%x",valor);
                printf(" ");
            }
            if((modo_lectura & 0x04) != 0){
                //octal
                printf("%llo",valor);
                printf(" ");
            }
            if((modo_lectura & 0x02) != 0){
                //caracter
                printf("%lc",valor);
                for(caracter = 0; caracter < cantBytes; caracter++){
                    printf("%c",valor >> ((cantBytes - 1 - caracter) * 8) & 0xFF);
                }
                printf(" ");
            }
            if((modo_lectura & 0x01) != 0){
                //decimal
                printf("%lld",valor);
            }
        printf("\n");
        }
        printf("\n");
    }
    else if(get(registros[OP1],registros,memoria,tablaSegmentos) == 0x2){ //escribe en pantalla
        for(i = 0; i<cantCeldas; i++){
            valor = 0;
            for(j = 0; j < cantBytes; j++){
                //escribimos byte a byte, pasamos 0 en el valor para asegurarnos que el MBR va a estar limpio
                operacion_memoria(registros,memoria,registros[EDX]+i*cantBytes+j, 0, LECTURA, 1, tablaSegmentos);
                byteActual = registros[MBR];
                valor = valor | (byteActual << ((cantBytes - 1 - j) * 8));
            }
            printf("[%04x]: ",(registros[MAR] & 0x0000FFFF) - cantBytes + 1);
            if((modo_lectura & 0x10) != 0){
                for(b = 0; b < cantBytes; b++){
                    printf("%d ",valor >> ((cantBytes - 1 - b) * 8) & 0x01);//muestro el valor en binario
                }
                printf(" ");
            }
            if((modo_lectura & 0x08) != 0){
                printf("0x%x",valor);
                printf(" ");
            }
            if((modo_lectura & 0x04) != 0){
                printf("%o",valor);
                printf(" ");
            }
            if((modo_lectura & 0x02) != 0){
                printf("%c",valor);
                for(caracter = 0; caracter < cantBytes; caracter++){
                    printf("%c",valor >> ((cantBytes - 1 - caracter) * 8) & 0xFF);
                }
                printf(" ");
            }
            if((modo_lectura & 0x01) != 0){
                printf("%d",valor);
            }
        printf("\n");
        }
        printf("\n");
    }
}
void RND(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    srand(time(NULL));

    set(registros,memoria,registros[OP1], rand() % (get(registros[OP2], registros, memoria,tablaSegmentos) + 1),tablaSegmentos);
}
void JMP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    registros[IP] = registros[OP1] & 0x0000FFFF;
}

void JZ(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x01<<30)) == (1<<30))
        registros[IP] = registros[OP1] & 0x0000FFFF;
}
void JP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if(registros[CC] == 0)
        registros[IP] = registros[OP1] & 0x0000FFFF;
}
void JN(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x02<<30)) == (2<<30))
        registros[IP] = registros[OP1] & 0x0000FFFF;
}
void JNZ(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x01<<30)) == 0)
        registros[IP] = registros[OP1] & 0x0000FFFF;
}
void JNP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x03<<30)) == (2<<30) || (registros[CC] & 0x03) == (1<<30))
        registros[IP] = registros[OP1] & 0x0000FFFF;
}
void JNN(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x02<<30)) < (2<<30))
        registros[IP] = registros[OP1] & 0x0000FFFF;
}

void ADD(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (int32_t)get(registros[OP1],registros,memoria,tablaSegmentos) + (int32_t)get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SUB(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int32_t resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) - get(registros[OP2],registros,memoria,tablaSegmentos);
    //printf("operacion SUB: %d = %d - %d\n",resultado,get(registros[OP1],registros,memoria,tablaSegmentos),get(registros[OP2],registros,memoria,tablaSegmentos));
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void MUL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) * get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void DIV(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if(get(registros[OP2],registros,memoria,tablaSegmentos) != 0){
        resultado = get(registros[OP1],registros,memoria,tablaSegmentos) / get(registros[OP2],registros,memoria,tablaSegmentos);
        actualizarCC(registros,resultado);
        registros[AC] = get(registros[OP1],registros,memoria,tablaSegmentos) % get(registros[OP2],registros,memoria,tablaSegmentos);
        set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
    }
    else{
        printf("ERROR: division por cero\n"); // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
        STOP(registros,memoria,tablaSegmentos);
    }
}
/*
//version alternativa del div con otra division
//la anterior tenia un problema en el codigo de la pregunta 5 del cuestionario, donde almacenaba 0 pero debia almacenar -1 que era el cociente
void DIV(uint32_t registros[], uint8_t memoria[], infoSegmento tablaSegmentos[]) {
    int32_t dividendo = get(registros[OP1], registros, memoria, tablaSegmentos);
    int32_t divisor = get(registros[OP2], registros, memoria, tablaSegmentos);
    
    if (divisor != 0) {
        int32_t cociente = dividendo / divisor;
        int32_t resto = dividendo % divisor;

        //chequeo que se hace si el signo del divisor y dividendo difieren
        if ((resto != 0) && ((dividendo < 0) != (divisor < 0))) {
            cociente--;
        }
        resto = dividendo - (cociente * divisor);

        actualizarCC(registros, cociente);
        registros[AC] = resto;
        set(registros, memoria, registros[OP1], cociente, tablaSegmentos);
    } else {
        printf("ERROR: division por cero\n");
        STOP(registros, memoria, tablaSegmentos);
    }
}
*/
void CMP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) - get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
}
//Los shifts en C serán lógicos porque siempre trabajamos con unsigned
void SHL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ((uint32_t)(get(registros[OP1],registros,memoria,tablaSegmentos)) << get(registros[OP2],registros,memoria,tablaSegmentos));
    //printf("SHL: %d = %d << %d\n",resultado,get(registros[OP1],registros,memoria,tablaSegmentos),get(registros[OP2],registros,memoria,tablaSegmentos));
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SHR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ((uint32_t)(get(registros[OP1],registros,memoria,tablaSegmentos)) >> get(registros[OP2],registros,memoria,tablaSegmentos));
    //printf("SHR: %d = %d >> %d\n",resultado,get(registros[OP1],registros,memoria,tablaSegmentos),get(registros[OP2],registros,memoria,tablaSegmentos));
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SAR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if((get(registros[OP1],registros,memoria,tablaSegmentos) & 0x80000000) != 0) // si es neg, el resultado tambien lo sera: agrego unos
        resultado = get(registros[OP1],registros,memoria,tablaSegmentos) >> get(registros[OP2],registros,memoria,tablaSegmentos) | ~(0xFFFFFFFF >> get(registros[OP2],registros,memoria,tablaSegmentos));
    else
        resultado = (get(registros[OP1],registros,memoria,tablaSegmentos) >> get(registros[OP2],registros,memoria,tablaSegmentos)); // si es positivo: agrego ceros
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void AND(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) & get(registros[OP2],registros,memoria,tablaSegmentos);
    printf("operacion AND: %d = %d & %d\n",resultado,get(registros[OP1],registros,memoria,tablaSegmentos),get(registros[OP2],registros,memoria,tablaSegmentos));
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void OR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) | get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void XOR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) ^ get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SWAP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int aux = get(registros[OP1],registros,memoria,tablaSegmentos);
    set(registros,memoria, registros[OP1],get(registros[OP2],registros,memoria,tablaSegmentos),tablaSegmentos);
    set(registros,memoria, registros[OP2],aux,tablaSegmentos);
}
void LDL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(registros[OP2],registros,memoria,tablaSegmentos) & 0x0000FFFF) | (get(registros[OP1],registros,memoria,tablaSegmentos) & 0xFFFF0000);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void LDH(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ((get(registros[OP2],registros,memoria,tablaSegmentos) << 16) & 0xFFFF0000) | (get(registros[OP1],registros,memoria,tablaSegmentos) & 0x0000FFFF);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void NOT(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ~get(registros[OP1],registros,memoria,tablaSegmentos);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void STOP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    registros[IP] = 0xFFFFFFFF;
}
void MOV(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    set(registros,memoria,registros[OP1],get(registros[OP2], registros, memoria,tablaSegmentos),tablaSegmentos);
}

void NO_ACCESIBLE(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    printf("INSTRUCCION INVALIDA: codigo de operacion de la instruccion a ejecutar no existe\n");  // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
}

void leerEncabezado(char nombre[], uint32_t registros[REG], infoSegmento tablaSegmento[ENT], uint8_t memoria[MEM], int *resultado, uint8_t *num_segmentos){
    FILE *arch;
    char ident[6];
    char version;
    uint16_t tamanio = 0, base, entry_offset;
    uint8_t byte_count,byte_aux;
    *num_segmentos =0;
    *resultado = 0;
    int i;
    arch = fopen(nombre, "rb");
    
    if(arch != NULL){
        printf("Archivo abierto correctamente\n");
        
        // obtener el tamaño del archivo para verificar
        fseek(arch, 0, SEEK_END);
        long tamano_archivo = ftell(arch);
        fseek(arch, 0, SEEK_SET);
        
        
        if(fread(ident, sizeof(char), 5, arch) == 5 && strcmp(ident,"VMX25") == 0){
            ident[5]='\0';
            if(strcmp(ident,"VMX25") == 0)
                if(fread(&version, sizeof(char), 1, arch) == 1){
                    if(version == 2){
                        base = 0;
                        for(i=0;i<10;i++){//leo 5 números (son 5 segmentos quitando el param) de 2 bytes cada uno
                            if(fread(&byte_aux, 1, 1, arch) == 1){
                                tamanio = (tamanio << 8) | byte_aux;
                                if(i%2){
                                    //terminé de leer el tamaño de este segmento
                                    tablaSegmento[*num_segmentos].base = base;
                                    tablaSegmento[*num_segmentos].tamanio = tamanio;
                                    base += tamanio;
                                    if(tamanio!=0){
                                        *num_segmentos += 1;
                                        registros[26 + i/2] = base;
                                    }
                                    else{
                                        registros[26 + i/2] = 0xFFFFFFFF;
                                    }
                                    tamanio = 0;
                                }
                            }
                            fread(&byte_aux, 1, 1, arch);
                            entry_offset = (entry_offset << 8) | byte_aux;
                            fread(&byte_aux, 1, 1, arch);
                            entry_offset = (entry_offset << 8) | byte_aux;
                            registros[IP] = (registros[CS] << 16) | entry_offset;
                        }
                        //queda por asignar el param segment
                    }
                        
                }
                else {
                    printf("ERROR: Versión incorrecta (esperado: 1, leído: %d)\n", (int)version);
                }
        }
        
        fclose(arch);
    }
    else {
        printf("ERROR: No se pudo abrir el archivo '%s'\n", nombre);
    }
    
    if(*resultado == 0) {
        printf("ERROR: No se pudo leer el encabezado correctamente\n");
    }
}
void debug_memoria(uint8_t memoria[], uint32_t direccion, int cantBytes) {
    printf("Memoria en [%04x]: ", direccion);
    int i;
    for (i = 0; i < cantBytes; i++) {
        printf("%02x ", memoria[direccion + i]);
    }
    printf("\n");
}

void debug_registros(uint32_t registros[]) {
    printf("Registros clave: IP=%08x, MBR=%08x, MAR=%08x, LAR=%08x\n", 
           registros[IP], registros[MBR], registros[MAR], registros[LAR]);
}
void operacion_memoria(uint32_t registros[], uint8_t memoria[], uint32_t direccion, int32_t valor, uint8_t tipo_operacion, uint8_t cantBytes, infoSegmento tablasegmento[]){
    //acá se ejecutan las escrituras o lecturas del DS
    int i;
    registros[LAR] = registros[DS] | direccion;
    registros[MBR] = valor;
    calcDirFisica(tablasegmento,registros,cantBytes);
    if(registros[IP] == 0xFFFFFFFF) {
        return;
    }
    registros[MAR] = registros[MAR] | (cantBytes << 16);
    if (tipo_operacion == ESCRITURA){
        cantBytes = (registros[MAR] >> 16) & 0x000000FF;
        for (i = 0; i < cantBytes; i++) {
            memoria[(registros[MAR] & 0x0000FFFF) + i] = (registros[MBR] >> (8 * (cantBytes - 1 - i))) & 0x000000FF;
        }
    }
    else{
        //se llama acá con 0 en el argumento 'valor' por lo tanto el MBR tiene 0
        for(i = 0; i < cantBytes; i++){
            registros[MBR] = registros[MBR] | (memoria[(registros[MAR] & 0x0000FFFF) + i] << (8 * (cantBytes - 1 - i)));
        }
    }
    
}

void calcDirFisica(infoSegmento tablaSegmento[ENT],uint32_t registros[],int cantBytes){
    // Deja en los bytes menos significativos del MAR la direccion fisica
    uint32_t numSegmento = (registros[LAR] >> 16) & 0x0000FFFF;
    uint32_t desplaz = registros[LAR] & 0x0000FFFF; // obtengo los 2 bytes menos significativos
    uint32_t dirFisica = tablaSegmento[numSegmento].base + desplaz;
    uint32_t limSegmento = tablaSegmento[numSegmento].base + tablaSegmento[numSegmento].tamanio;
    uint32_t limAcceso = dirFisica + cantBytes;

    if(numSegmento < ENT && tablaSegmento[numSegmento].base <= dirFisica && limSegmento >= limAcceso)
        registros[MAR] = dirFisica;
    else{
        printf("SEGMENTATION  FAULT\n"); // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
        registros[IP] = 0xFFFFFFFF;
        return;
    }
}

void operandos(uint32_t *lectura,uint32_t tipo,uint32_t registros[],uint8_t memoria[]){
    //lectura del valor de los operandos
    *lectura = 0;
    switch (tipo){
        case 0b01: { //registro
            *lectura = memoria[registros[IP]];
            break;
        }
        case 0b10: { //inmediato
            *lectura = (memoria[registros[IP]] << 8)  |  memoria[registros[IP] + 1] ;
            break;
        }
        case 0b11: { //direccion de memoria
            *lectura = (memoria[registros[IP]] << 16) | ((memoria[registros[IP] + 1]  ) << 8) | (memoria[registros[IP]+ 2]);
            break;
        }
        default:
            //pongo esto por las dudas, si no sirve le pregunto al profe
            *lectura = 0;
            break;
    }
}

void leerInstrucciones(uint8_t instruccion, uint8_t memoria[], uint32_t registros[REG], infoSegmento tablaSegmento[]){
    //usar los OP para calcular los bytes que necesitamos leer y poner en lectura

    uint32_t lectura;
    uint8_t cantByteA,cantByteB;

    registros[OPC]=instruccion & 0x1F;
    registros[OP2]=instruccion >> 6;
    cantByteB = registros[OP2];
    registros[OP1]=(instruccion >> 4) & 0x03;
    cantByteA = registros[OP1];
    if ((registros[OPC] > 0x08 && registros[OPC] < 0x0F) || (registros[OPC] > 0x0F && registros[OPC] < 0x10) || (registros[OPC] > 0x1F)){
            printf("ERROR: operacion invalida\n");
            registros[IP] = 0xFFFFFFFF;
    }
    else{
        if (registros[OP1]==0 && registros[OP2]!=0){
            registros[OP1]=registros[OP2];
            cantByteA = cantByteB;
            cantByteB = 0;
            registros[OP2]=0;
        }
        if ((registros[OPC]!=0x0F && registros[OP1]==0)||(registros[OPC]<=0x1F && registros[OPC]>=0x10 && registros[OP2]==0)){ //reviso si es una operacion invalida
            printf("ERROR: operacion invalida\n");
            registros[IP] = 0xFFFFFFFF;
        }
        else {
            if(registros[OPC] != 0x0F){
                registros[IP] = registros[IP] + 1;

                if(registros[IP] + cantByteB > tablaSegmento[0].base + tablaSegmento[0].tamanio) {
                    printf("SEGMENTATION FAULT\n");
                    registros[IP] = 0xFFFFFFFF;
                    return;
                }
                operandos(&lectura,registros[OP2],registros,memoria);
                registros[OP2]=registros[OP2] << 24;
                registros[OP2]=registros[OP2] | lectura;
                registros[IP] = registros[IP] + cantByteB;

                if(registros[IP] + cantByteA > tablaSegmento[0].base + tablaSegmento[0].tamanio) {
                    printf("ERROR: Lectura de operando 1 fuera de límites\n");
                    registros[IP] = 0xFFFFFFFF;
                    return;
                } 
                operandos(&lectura,registros[OP1],registros,memoria);
                registros[OP1]=registros[OP1] << 24;
                registros[OP1]=registros[OP1] | lectura;
                registros[IP] = registros[IP] + cantByteA;
            }
            //Aca ya tengo OPC, OP1 y OP2 para ejecutar
            instrucciones[registros[OPC]](registros,memoria,tablaSegmento);
        }
    }
}
int32_t get(uint32_t operando,uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    // considerar caso de la funcion SYS donde la cantidad de bytes es impredecible
    int tipo_operando = (operando >> 24) & 0x00000003;
    uint8_t cod_reg = (operando >> 16) & 0x0000001F; // en caso de que el operando sea direccion de memoria saco los 5 bits que indicarian un registro
    operando = operando & 0x0000FFFF;
    uint16_t direccion = registros[cod_reg] + (int16_t)(operando & 0x0000FFFF);//le saco el codigo de registro al operando y hago el casteo por si el offset es negativo

    if (tipo_operando == 0x01)
        return (int32_t)registros[operando];
    else if (tipo_operando == 0x02) {//inmediato, puede ser negativo
            if (operando & 0x8000) {
                // es un número negativo
                // extender el signo a 32 bits para que sea compatible con el retorno sin alterar el valor original
                return operando | 0xFFFF0000;
            }
            return (int32_t)operando;
        }
    else {
        //el operando es direccion de memoria

        operacion_memoria(registros, memoria, direccion, 0, LECTURA, 4, tablaSegmentos); //4 bytes porque es el tamaño de cada celda
        return (int32_t)registros[MBR];
    }
}
void set(uint32_t registros[], uint8_t memoria[], uint32_t operando1, int32_t operando2,infoSegmento tablaSegmentos[]){
    //operando 2 será inmediato siempre en esta función, pues se la llamará con el argumento get()
    int tipo_operando1 = (operando1 >> 24) & 0x00000003;
    uint32_t direccion;
    uint8_t cod_reg = (operando1 >> 16) & 0x0000001F;
    operando1 = operando1 & 0x00FFFFFF;

    if (tipo_operando1 == 1)
        registros[operando1] = operando2;
    else{
        direccion = registros[cod_reg] +(int16_t) operando1 & 0x0000FFFF;
        operacion_memoria(registros, memoria, direccion, operando2, ESCRITURA, 4,tablaSegmentos);
    }
}

void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]){

    registros[IP] = registros[CS];
    uint16_t base = tablaSegmento[registros[CS]].base;
    uint16_t tamanio = tablaSegmento[registros[CS]].tamanio;
    
    leerInstrucciones(memoria[registros[IP]], memoria, registros, tablaSegmento);
    while (registros[IP] != 0xFFFFFFFF && registros[IP] < base+tamanio ){
       leerInstrucciones(memoria[registros[IP]], memoria, registros, tablaSegmento);
    }

}

void actualizarCC(uint32_t registros[],int32_t resultado){
    registros[CC] = 0;
    
    if(resultado < 0)
        registros[CC] = registros[CC] | (0x02 << 30);
    if(resultado == 0)
        registros[CC] = registros[CC] | (0x01 << 30);
    //printf("CC actualizado: %d\n",registros[CC]);
}