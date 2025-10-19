#include "maquina.h"
void inicioTablaSegmento(infoSegmento tabla[],uint16_t tamanioCod){
    tabla[0].base = 0x0000;
    tabla[0].tamanio = tamanioCod;

    tabla[1].base = tamanioCod;
    tabla[1].tamanio = MEM - tamanioCod;
}


// Las posiciones no inicializadas automáticamente quedan como NULL
void (*instrucciones[32])(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]) =
{SYS,JMP,JZ,JP,JN,JNZ,JNP,JNN,NOT,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,NO_ACCESIBLE,STOP,MOV,ADD,SUB,MUL,DIV,CMP,SHL,SHR,SAR,AND,OR,XOR,SWAP,LDL,LDH,RND};

static void mostrarHexa(uint8_t instruccion[], uint8_t inicio, uint8_t fin) {
    uint8_t i;
    for (i = inicio; i < fin; i++) {
        printf("%02X ", instruccion[i]);
    }
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
                                    base += tamanio;
                                    if(tamanio!=0){
                                        tablaSegmento[*num_segmentos].tamanio = tamanio;
                                        *num_segmentos += 1;
                                        registros[26 + i/2] = (*num_segmentos) << 16; 
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
void operacion_memoria(uint32_t registros[], uint8_t memoria[], uint32_t direccion, int32_t valor, uint8_t tipo_operacion, uint8_t cantBytes, infoSegmento tablasegmento[], uint32_t segmento){
    //acá se ejecutan las escrituras o lecturas del DS
    int i;
    registros[LAR] = segmento | direccion;
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
    else{//tipo_operacion == LECTURA
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
uint32_t get_segmento_registro(uint32_t operando,uint32_t registros[]){
    uint8_t registro = operando & 0x1F;
    uint8_t segmento_registro = (operando >> 6) & 0x3;

    return registros[registro] >> (4-segmento_registro);
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
uint8_t get_segmento(uint8_t cod_reg, uint32_t registros[], infoSegmento tablaSegmentos[]){
    int i = 0;
    if(cod_reg < CS && cod_reg != BP && cod_reg != SP) cod_reg = CS; //El codigo de registro es uno de los de uso general
    while(tablaSegmentos[i].base != registros[cod_reg])
        i++;
    return i;
}
int32_t get(uint32_t operando,uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    // considerar caso de la funcion SYS donde la cantidad de bytes es impredecible
    int tipo_operando = (operando >> 24) & 0x00000003;
    uint8_t cod_reg = (operando >> 16) & 0x0000001F; // en caso de que el operando sea direccion de memoria saco los 5 bits que indicarian un registro
    operando = operando & 0x0000FFFF;
    uint16_t direccion = registros[cod_reg] + (int16_t)(operando & 0x0000FFFF);//le saco el codigo de registro al operando y hago el casteo por si el offset es negativo

    if (tipo_operando == 0x01)
        return (int32_t)get_segmento_registro(operando, registros);
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
        direccion += 4-((operando>>30) & 0x00000003);
        operacion_memoria(registros, memoria, direccion, 0, LECTURA, 4-((operando>>30) & 0x00000003), tablaSegmentos, get_segmento(cod_reg, registros, tablaSegmentos)); //4 bytes porque es el tamaño de cada celda
        return (int32_t)registros[MBR];
    }
}
void set_segmento_registro(uint32_t registros[],uint32_t operando1, int32_t operando2,uint8_t reg){
    uint8_t seg_reg = operando1 >> 6 & 0x03;

    switch (seg_reg){
        case 0:
            registros[reg] = operando2;
            break;
        case 1://Por ejemplo AL
            registros[reg] = (reg & 0xFFFFFF00) & operando2;
            break;
        case 2://Por ejemplo AH
            registros[reg] = (reg & 0xFFFF00FF) & operando2;
            break;
        case 3://Por ejemplo AAX
            registros[reg] = (reg & 0xFFFF0000) & operando2;
            break;
    }
}
void set(uint32_t registros[], uint8_t memoria[], uint32_t operando1, int32_t operando2,infoSegmento tablaSegmentos[]){
    //operando 2 será inmediato siempre en esta función, pues se la llamará con el argumento get()
    int tipo_operando1 = (operando1 >> 24) & 0x00000003;
    uint32_t direccion;
    uint8_t cod_reg = (operando1 >> 16) & 0x0000001F,reg = operando1 & 0x1F;
    operando1 = operando1 & 0x00FFFFFF;

    if (tipo_operando1 == 1)
        set_segmento_registro(registros,operando1, operando2, reg);
    else{
        direccion = registros[cod_reg] +(int16_t) operando1 & 0x0000FFFF;

        operacion_memoria(registros, memoria, direccion, operando2, ESCRITURA, 4,tablaSegmentos, get_segmento(cod_reg, registros, tablaSegmentos));
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
void generar_imagen(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    FILE *archivoVMI;
    uint16_t tamMemoria = MEM; /// --------------------------------MODIFICAR CON VERSION 2 (VARIABLE)------------------------
    int i,tamTabla = 8;
    uint32_t registro,segmento;

    archivoVMI = fopen("Estado_Actual","wb");//------------------MODIFICAR NOMBRE COMO PARAMETRO--------------------------
    if(archivoVMI != NULL){
        fwrite("VMI25",1,5,archivoVMI);
        uint8_t version = 1;
        fwrite(&version,1,1,archivoVMI);
        fwrite(&tamMemoria,2,1,archivoVMI);
        for(i=0; i<REG; i++){
            int registro = registros[i];
            registro = ((registro >> 24) & 0xFF) | ((registro << 8) & 0xFF0000) |  ((registro >> 8) & 0xFF00) |  ((registro << 24) & 0xFF000000);
            fwrite(&registro,4,1,archivoVMI);
        }
        for(i=0; i<tamTabla; i++){
            segmento = (tablaSegmentos[i].base << 16) | tablaSegmentos[i].tamanio;
            segmento = ((segmento >> 24) & 0xFF) | ((segmento << 8) & 0xFF0000) |  ((segmento >> 8) & 0xFF00) |  ((segmento << 24) & 0xFF000000);
            fwrite(&segmento,4,1,archivoVMI);
        }
        fwrite(memoria,1,tamMemoria,archivoVMI);
        fclose(archivoVMI);
    }
    

}