#include "maquina.h"
#include <stdlib.h>
#include <stdio.h>
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
void (*instrucciones[32])(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]) = 
{SYS,JMP,JZ,JP,JN,JNZ,JNP,JNN,NOT,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,STOP,MOV,ADD,SUB,MUL,DIV,CMP,SHL,SHR,SAR,AND,OR,XOR,SWAP,LDL,LDH,RND};
    
void leerEncabezado(char nombre[],uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]){
    FILE *arch;
    char ident[6];    
    char version;
    uint16_t tamanio;
    int resultado = 0;

    arch = fopen(nombre,"rb");
    
    if(arch!=NULL){
        fseek(arch,0,0); // inicio de archivo
        if( fread(ident,sizeof(char),5,arch) == 5){
            ident[5] = '\0';
            if (strcmp(ident,"VMX25") == 0){ 
                fread(&version,sizeof(char),1,arch);
                if(version == 1){
                    fread(&tamanio,sizeof(uint16_t),1,arch);
                    if(tamanio < MEM){
                        resultado = 1;
                        fread(memoria,tamanio,1,arch);
                    }
              }
          }
        }
    }
    fclose(arch);
    if(resultado){
        inicioRegistro(registros);
        inicioTablaSegmento(tablaSegmento,tamanio);
    }
    else 
        printf("ERROR: no se pudo leer el resultado\n");

}
void operacion_memoria(uint32_t registros[], uint8_t memoria[], uint32_t direccion, uint32_t valor, uint8_t tipo_operacion, uint8_t cantBytes, infoSegmento tablasegmento[]){
    //acá se ejecutan las escrituras o lecturas del DS
    //falta añadir codigo de segmento
    registros[LAR] = direccion;
    registros[MBR] = valor;
    calcDirFisica(tablasegmento,registros,cantBytes);
    registros[MAR] = registros[MAR] | (cantBytes << 16);


    if (tipo_operacion == ESCRITURA){
        cantBytes = (registros[MAR] >> 16) & 0x000000FF;
        for (int i = 0; i < cantBytes; i++) {
            memoria[(registros[MAR] & 0x0000FFFF) + i] = (registros[MBR] >> (8 * (cantBytes - 1 - i))) & 0x000000FF;   
        }
    }
    else{
        //se llama acá con 0 en el argumento 'valor' por lo tanto el MBR tiene 0
        for(int i = 0; i < cantBytes; i++){
            registros[MBR] = registros[MBR] | (memoria[registros[MAR] & 0x0000FFFF]) << (24-8*i);
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

    if(tablaSegmento[numSegmento].base <= dirFisica && limSegmento >= limAcceso)
        registros[MAR] = dirFisica;
    else{
        printf("SEGMENTATION  FAULT\n"); // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
        registros[IP] = 0xFFFFFFFF;
    }
}

void operandos(uint32_t *lectura,uint32_t tipo,uint32_t registros[],uint8_t memoria[]){
    //lectura del valor de los operandos
    switch (tipo){
    case 0b01: { //registro
        *lectura = memoria[IP]; 
        break;
    }
    case 0b10: { //inmediato
        *lectura = (memoria[IP] << 8)  |  memoria[IP + 1];
    }
    case 0b11: { //direccion de memoria
        *lectura = (memoria[IP] << 16) | (memoria[IP + 1] << 8) | (memoria[IP + 2]);
    }
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
        }
        if ((registros[OPC]!=0x0F && registros[OP1]==0)||(registros[OPC]<=0x1F && registros[OPC]>=0x10 && registros[OP2]==0)){ //reviso si es una operacion invalida
            printf("ERROR: operacion invalida\n");
            registros[IP] = 0xFFFFFFFF;
        }
        else {
            //aca hay que avanzar el IP en 1 y leer byte a byte OP2 veces
            registros[IP] = registros[IP] + 1;

            calcDirFisica(tablaSegmento, registros, cantByteA);
            
            operandos(&lectura,registros[OP2],registros,memoria);  /// agregar a .h ----------------------------------------?
            registros[OP2]=registros[OP2] << 24;
            registros[OP2]=registros[OP2] | lectura;
            registros[IP] = registros[IP] + cantByteA;

            calcDirFisica(tablaSegmento, registros, cantByteB);
            registros[MAR] = registros[LAR];
            operandos(&lectura,registros[OP1],registros,memoria);
            registros[OP1]=registros[OP1] << 24;
            registros[OP1]=registros[OP1] | lectura;
            registros[IP] = registros[IP] + cantByteB;

            //Aca ya tengo OPC, OP1 y OP2 para ejecutar
            instrucciones[registros[OPC]](registros,memoria,tablaSegmento);
        }
    }
}
uint32_t get(uint32_t operando,uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    // considerar caso de la funcion SYS donde la cantidad de bytes es impredecible
    int tipo_operando = (operando >> 24) & 0x00000003;
    uint8_t cod_reg = (operando >> 16) & 0x000000FF; // en caso de que el operando sea direccion de memoria saco los 5 bits que indicarian un registro
    operando = operando & 0x0000FFFF;
    uint16_t direccion = registros[cod_reg] + operando & 0x0000FFFF;//le saco el codigo de registro al operando

    if (tipo_operando == 0x01)
        return registros[operando];
    else if (tipo_operando == 0x02)
        return operando;
    else {
        //el operando es direccion de memoria
        operacion_memoria(registros, memoria, direccion, 0, LECTURA, 1, tablaSegmentos); //desde el get() solo leo la posición de memoria -> solo leo 1 byte y por eso el 1
        return registros[MBR];
    }
}
void set(uint32_t registros[], uint8_t memoria[], uint32_t operando1, uint16_t operando2,infoSegmento tablaSegmentos[]){
    //operando 2 será inmediato siempre en esta función, pues se la llamará con el argumento get()
    //no necesito el argumento operando1, corregir
    int tipo_operando1 = (operando1 >> 24) & 0x00000003;
    uint32_t direccion;
    uint16_t offset;
    uint8_t cod_reg = (operando1 >> 16) & 0x000000FF; 
    operando1 = operando1 & 0x00FFFFFF;
    
    if (tipo_operando1 == 1)
        registros[operando1] = operando2; 
    else{
        offset = operando1 & 0x0000FFFF;
        direccion = registros[cod_reg] + offset;
        operacion_memoria(registros, memoria, direccion, operando2, ESCRITURA, 1,tablaSegmentos);
    }
}
void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]){
    //falta evaluar la cantidad de argumentos para saber cual de los dos vectores de punteros a funciones utilizar
    uint8_t instruccion;
    uint8_t codInstruccion;
    uint8_t Tipo1,Tipo2;
    uint32_t operando2,operando1;
    
    registros[IP] = registros[CS];
    
    leerInstrucciones(memoria[registros[IP]], memoria, registros, tablaSegmento,instrucciones);
    while (registros[IP] != 0xFFFFFFFF)
       leerInstrucciones(memoria[registros[IP]], memoria, registros, tablaSegmento);
    
}


void actualizarCC(uint32_t registros[],uint32_t resultado){
    if(resultado < 0)
        registros[CC] = 0x80000000;
    else
        if(resultado == 0)
            registros[CC] = 0x40000000;
        else 
            registros[CC] = 0x0;
}