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
void operacion_memoria(uint32_t registros[], uint8_t memoria[], uint32_t direccion, uint32_t valor, uint8_t tipo_operacion){
    //acá se ejecutan las escrituras o lecturas del DS
    uint8_t cantBytes = (registros[MAR] >> 16) & 0x000000FF;
    registros[LAR] = direccion;
    registros[MBR] = valor;
    registros[MAR] = registros[LAR];


    if (tipo_operacion == ESCRITURA){
        for(int i = 0; i < cantBytes; i++){
            memoria[registros[MAR + i] & 0x0000FFFF] = registros[MBR];
        }
    }
    else{
        registros[MBR] = memoria[registros[MAR] & 0x0000FFFF];
    }
}
void calcDirFisica(infoSegmento tablaSegmento[ENT],uint32_t registros[],int cantBytes){
    
    uint32_t numSegmento = (registros[IP] >> 16) & 0x0000FFFF;
    uint32_t desplaz = registros[IP] & 0x0000FFFF; // obtengo los 2 bytes menos significativos
 
    uint32_t limSegmento = tablaSegmento[numSegmento].base + tablaSegmento[numSegmento].tamanio;
    uint32_t limAcceso = registros[LAR] + cantBytes;
    if(tablaSegmento[numSegmento].base <= registros[LAR] && limSegmento >= limAcceso)
        registros[LAR] = tablaSegmento[numSegmento].base + desplaz;
    else{
        printf("SEGMENTATION  FAULT\n"); // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
        registros[IP] = 0xFFFFFFFF;
    }
}

void operandos(uint32_t *lectura,uint32_t tipo,uint32_t registros[],uint8_t memoria[]){
    //lectura del valor de los operandos
    switch (tipo){
    case 0b01: { //registro
        *lectura = memoria[MAR]; 
        break;
    }
    case 0b10: { //inmediato
        *lectura = (memoria[MAR] << 8)  |  memoria[MAR + 1];
    }
    case 0b11: { //direccion de memoria
        *lectura = (memoria[MAR] << 16) | (memoria[MAR + 1] << 8) | (memoria[MAR + 2]);
    }
    }
}

void leerInstrucciones(uint8_t instruccion, uint8_t memoria[], uint32_t registros[REG], infoSegmento tablaSegmento[],
        void (*un_operando[16])(uint32_t registros[]), void (*dos_operando[16])(uint32_t registros[], uint8_t memoria[])){
    //usar los OP para calcular los bytes que necesitamos leer y poner en lectura
    
    uint32_t lectura;
    uint8_t cantByteA,cantByteB;
    
    registros[OPC]=instruccion & 0x1F;
    registros[OP2]=instruccion >> 6;
    cantByteB = registros[OP2];
    registros[OP1]=(instruccion >> 4) & 0x03;
    cantByteA = registros[OP1];

    if (registros[OP1]==0 && registros[OP2]!=0){
        registros[OP1]=registros[OP2];
        cantByteA = cantByteB;
        cantByteB = 0;
    }
    if ((OPC!=0x0F && registros[OP1]==0)||(OPC<=0x1F && OPC>=0x10 && registros[OP2]==0)){ //reviso si es una operacion invalida
        printf("ERROR: operacion invalida\n");
        registros[IP] = 0xFFFFFFFF;
        registros[LAR] = registros[IP];
    }
    else {
        //aca hay que avanzar el IP en 1 y leer byte a byte OP2 veces
        registros[IP] = registros[IP] + 1;

        calcDirFisica(tablaSegmento, registros, cantByteA);
        registros[MAR] = registros[LAR];
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
        if (cantByteB == 0){
            un_operando[registros[OPC]](registros);
        }
        else{
            dos_operando[registros[OPC]](registros,memoria,tablaSegmento);
        }
    }
}

void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM],void (*un_operando[16])(uint32_t registros[]), void (*dos_operando[16])(uint32_t registros[], uint8_t memoria[])){
    //falta evaluar la cantidad de argumentos para saber cual de los dos vectores de punteros a funciones utilizar
    uint8_t instruccion;
    uint8_t codInstruccion;
    uint8_t Tipo1,Tipo2;
    uint32_t operando2,operando1;
    
    registros[IP] = registros[CS];
    calcDirFisica(tablaSegmento,registros,1);  // 1?? ref de la cantidad de bytes de acceso
    registros[MAR] = registros[LAR]; //esto es solo posible porque en esta parte del cuatrimestre el CS empieza en la posicion 0
    leerInstrucciones(memoria[registros[IP]], memoria, registros, tablaSegmento,un_operando,dos_operando);
    while (registros[IP] != 0xFFFFFFFF){
        registros[MAR] = registros[LAR];
        registros[MBR] = memoria[registros[MAR]];
        codInstruccion = registros[MBR];
        Tipo1 = (registros[OP1] >> 30 ) & 0x3;
        Tipo2 = (registros[OP2] >> 30 ) & 0x3;
        operando1 = registros[OP1] & 0x00FFFFFF;
        operando2 = registros[OP2] & 0x00FFFFFF;
       // instrucciones[codInstruccion](Tipo1,Tipo2,OP1,OP2,registros,memoria);
       leerInstrucciones(memoria[registros[IP]], memoria, registros, tablaSegmento,un_operando,dos_operando);
    }
}
uint32_t get(uint32_t operando,uint32_t registros[], uint8_t memoria[]){
    int tipo_operando = (operando >> 24) & 0x00000003;
    if (tipo_operando == 0x01)
        return registros[operando];
    else if (tipo_operando == 0x02)
        return operando;
    else {
        //el operando es direccion de memoria
        registros[MAR] = (tipo_operando << 2) & 0xFFFF0000;
        operacion_memoria(registros, memoria, operando, 0, LECTURA); //desde el get() solo leo la posición de memoria -> solo leo 1 byte y por eso el 1
        return registros[MBR];
    }
}
void set(uint32_t registros[], uint8_t memoria[], uint32_t operando1, uint16_t operando2){
    //operando 2 será inmediato siempre en esta función, pues se la llamará con el argumento get()
    int tipo_operando = (operando1 >> 24) & 0x00000003;

    if (tipo_operando == 1)
        registros[operando1] = operando2; 
    else{
        registros[MBR] = operando2;
        
        registros[MAR] = (tipo_operando << 16) & 0xFFFF0000;
        operacion_memoria(registros, memoria, operando1, operando2, ESCRITURA);
    }
}
void ResultadoOperacion(uint32_t registros[],uint8_t memoria[],int resultado,infoSegmento tablaSegmentos[]){
    uint32_t codRegistro;
    uint8_t Tipo1 = registros[OP1] >> 24;
    if(Tipo1 == 0b01){ // registro
        codRegistro = registros[OP1] & 0x00FFFFFF;
        registros[codRegistro] = resultado;
    }
    else {
        if(Tipo1 == 0b11) { // memoria
            calcDirFisica(tablaSegmentos,registros,1);
            registros[MAR] = registros[LAR];
            registros[MBR] = resultado;
            memoria[registros[MAR]] =  (registros[MBR] >> 24) & 0x000000FF;
            memoria[registros[MAR] + 1] = (registros[MBR] >> 16) & 0x00FF;
            memoria[registros[MAR] + 2] = (registros[MBR] >> 8) & 0x0000FF;
            memoria[registros[MAR] + 3] = registros[MBR] & 0x000000FF;
        }
    }
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