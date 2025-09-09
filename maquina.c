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
void leerEncabezado(char nombre[],uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]){
    FILE *arch;
    char ident[5];    
    char version;
    uint16_t tamanio;
    int resultado = 0;

    arch = fopen(nombre,"rb");
    
    if(arch!=NULL){
        fseek(arch,0,0); // inicio de archivo
        if( fread(ident,sizeof(char),5,arch) == 5){
            if (strcmp(ident,"VMX25")){ 
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
        printf("ERROR: no se pudo leer el resultado");

}
uint32_t calcDirFisica(infoSegmento tablaSegmento[ENT],uint32_t regIP,int cantBytes){
    uint32_t direcFisica;
    uint32_t numSegmento = (regIP >> 16) & 0x0000FFFF;
    uint32_t desplaz = regIP & 0x0000FFFF; // obtengo los 2 bytes menos significativos
    direcFisica = tablaSegmento[numSegmento].base + desplaz;
 
    uint32_t limSegmento = tablaSegmento[numSegmento].base + tablaSegmento[numSegmento].tamanio;
    uint32_t limAcceso = direcFisica + cantBytes ;
    if(tablaSegmento[numSegmento].base <= direcFisica && limSegmento >= limAcceso)
        return direcFisica;
    else{
        printf("segmentation fault\n"); 
        return 0xFFFFFFFF;
    }
}

void leerInstrucciones(uint8_t instruccion, uint8_t memoria[], uint32_t registros[REG], uint32_t *dirFisica){
    //usar los OP para calcular los bytes que necesitamos leer y poner en lectura
    
    uint32_t lectura;
    uint8_t cantByteA,cantByteB;
    
    registros[OPC]=instruccion & 0x1F;
    registros[OP2]=instruccion >> 6;
    cantByteB = registros[OP2];
    registros[OP1]=(instruccion >> 4) & 0x03;
    cantByteA = registros[OP1];

    if (registros[OP1]==0 && registros[OP2]!=0)
        registros[OP1]=registros[OP2];
    if ((OPC!=0x0F && registros[OP1]==0)||(OPC<=0x1F && OPC>=0x10 && registros[OP2]==0)){ //reviso si es una operacion invalida
        printf("Operacion invalida");
        *dirFisica = 0xFFFFFFFF;
    }
    else {
        //aca hay que avanzar el IP en 1 y leer byte a byte OP2 veces
        registros[OP2]=registros[OP2] << 24;
        registros[OP2]=registros[OP2] | lectura;
        
        
        //aca hay que avanzar el IP en 1 y leer byte a byte OP1 veces
        registros[OP1]=registros[OP1] << 24;
        registros[OP1]=registros[OP1] | lectura;

        //Aca ya tengo OPC, OP1 y OP2 para ejecutar

    }
    registros[IP] = registros[IP] + cantByteA + cantByteB + 1; // IP apunta a nueva direccion en memoria
}

void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]){
    uint8_t instruccion;
    uint32_t dirFisica;
    
    registros[IP] = registros[CS];
    dirFisica = calcDirFisica(tablaSegmento,registros[IP],1);  // 1?? ref de la cantidad de bytes de acceso
    while (dirFisica != 0xFFFFFFFF){
        leerInstrucciones(memoria[registros[IP]], memoria, registros, &dirFisica); //la dirFisica es una posicion del vector memoria que tenemos que chequear constantemente
        //aca iria una funcion que ejecute las instrucciones en base a lo que hay en el vector registros en las posiciones OP1, OP2 y OPC
        //lo haria accediendo a posiciones de un vector de punteros a funciones donde cada una es una instruccion
    }
}
