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

void leerInstrucciones(){
    char nombre[]="sample.vmx"; //el nombre debe ingresarse por terminal despues
    FILE *arch=fopen(nombre,"rb");
    int salir=1;
    uint32_t OPC,OP1,OP2,lectura;
    uint8_t instruccion; //tipo de entero de 8 bits sin signo
    if (!arch)
        printf("No se pudo abrir el archivo vmx");
    else {
        while (fread(&instruccion,1,1,arch) && salir){
            OPC=instruccion & 0x1F;
            OP2=instruccion >> 6;
            OP1=(instruccion >> 4) & 0x03;
            if (OP1==0 && OP2!=0)
                OP1=OP2;
            if ((OPC!=0x0F && OP1==0)||(OPC<=0x1F && OPC>=0x10 && OP2==0)){ //reviso si es una operacion invalida
                printf("Operacion invalida");
                salir=0;
            }
            else {
                if (fread(&lectura,OP2,1,arch)==1){
                    OP2=OP2 << 24;
                    OP2=OP2 | lectura;
                }
                if (fread(&lectura,OP1,1,arch)==1){
                    OP1=OP1 << 24;
                    OP1=OP1 | lectura;
                }
                //Aca ya tengo OPC, OP1 y OP2 para ejecutar
            }
            
        }
        fclose(arch);

    }
}

int leerEncabezado(){

}

