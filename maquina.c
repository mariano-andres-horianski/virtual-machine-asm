#include "maquina.h"
void inicioTablaSegmento(infoSegmento tabla[],uint16_t tamanioCod){
    tabla[0].base = 0x0000;
    tabla[0].tamanio = tamanioCod;

    tabla[1].base = tamanioCod;
    tabla[1].tamanio = MEM - tamanioCod;
}
void inicioRegistro(uint32_t reg[]){
    reg[CS] = 0x00000000;
    reg[DS] = 0x00010000;
    reg[KS] = 0xFFFFFFFF;
    reg[SS] = 0xFFFFFFFF;
    reg[ES] = 0xFFFFFFFF;
    reg[SP] = 0xFFFFFFFF;
    reg[IP] = reg [CS];
}
// Las posiciones no inicializadas automáticamente quedan como NULL
void (*instrucciones[32])(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]) =
{SYS,JMP,JZ,JP,JN,JNZ,JNP,JNN,NOT,NO_ACCESIBLE,NO_ACCESIBLE,PUSH,POP,CALL,RET,STOP,MOV,ADD,SUB,MUL,DIV,CMP,SHL,SHR,SAR,AND,OR,XOR,SWAP,LDL,LDH,RND};

void mostrarHexa(uint8_t instruccion[], uint8_t inicio, uint8_t fin) {
    uint8_t i;
    for (i = inicio; i < fin; i++) {
        printf("%02X ", instruccion[i]);
    }
}

void leerEncabezado(char nombre[], uint32_t registros[REG], infoSegmento tablaSegmento[ENT], uint8_t memoria[], int *resultado, uint8_t *num_segmentos, uint32_t tamano_param_segment){
    FILE *arch;
    char ident[6];
    char version;
    uint16_t tamanio = 0, tamanio_mem_principal=0, base, entry_offset;
    uint8_t byte_count,byte_aux;
    *resultado = 0;
    int i,j;
    arch = fopen(nombre, "rb");
    
    if(arch != NULL){
        printf("Archivo abierto correctamente\n");
        
        // obtener el tamaño del archivo para verificar
        fseek(arch, 0, SEEK_END);
        long tamano_archivo = ftell(arch);
        fseek(arch, 0, SEEK_SET);
        
        
        if(fread(ident, sizeof(char), 5, arch) == 5){
            ident[5]='\0';
            if(strcmp(ident,"VMX25") == 0)
                if(fread(&version, sizeof(char), 1, arch) == 1){
                    if(version == 1){
                        if(fread(&byte_aux, 1, 1, arch) == 1){
                            tamanio = (tamanio << 8) | byte_aux;
                            if(fread(&byte_aux, 1, 1, arch) == 1){
                                tamanio = tamanio | byte_aux;
                            }
                            else{
                                printf("Error al leer el tamaño del archivo\n");
                                fclose(arch);
                                return;
                            }
                            //queda iniciar la tabla de segmentos y los registros para la version 1 de la VM, ver funciones en el github inicioRegistros e inicioTablaSegmento
                            fread(memoria, sizeof(uint8_t), tamanio, arch);
                        }
                        else{
                            printf("Error al leer el tamaño del archivo\n");
                            fclose(arch);
                            return;
                        }
                        inicioTablaSegmento(tablaSegmento,tamanio);
                        inicioRegistro(registros);
                        *resultado = 1;
                    }
                if(version == 2){
                    base = tamano_param_segment;
                    tamanio_mem_principal = tamano_param_segment;
                    for(i=0;i<10;i++){//leo 5 números (son 5 segmentos quitando el param) de 2 bytes cada uno
                        if(fread(&byte_aux, 1, 1, arch) == 1){
                            tamanio = (tamanio << 8) | byte_aux;
                            if(i%2){
                                    //terminé de leer el tamaño de este segmento
                                if(tamanio!=0){
                                    tablaSegmento[*num_segmentos].base = base;
                                    base += tamanio;
                                    tablaSegmento[*num_segmentos].tamanio = tamanio;
                                    registros[26 + i/2] = *num_segmentos << 16;
                                    *num_segmentos += 1;
                                }
                                else{
                                    registros[26 + i/2] = 0xFFFFFFFF;
                                }
                                tamanio_mem_principal += tamanio;
                                if(tamanio_mem_principal > MEM){
                                    printf("ERROR: Memoria insuficiente\n");
                                    return;
                                }
                                tamanio = 0;
                            }
                        }
                        if(fread(&byte_aux, 1, 1, arch) == 1){
                            entry_offset = (entry_offset << 8) | byte_aux;
                            if(read(&byte_aux, 1, 1, arch)){
                                entry_offset = (entry_offset << 8) | byte_aux;
                                registros[IP] = registros[CS] | entry_offset;
                                registros[SP] = tablaSegmento[registros[SS]].base + tablaSegmento[registros[SS]].tamanio;
                            }
                        }
                    }
                    //queda por asignar el param segment
                    for(i=0;i<tablaSegmento[registros[CS]].tamanio;i++){
                        if(fread(memoria + tablaSegmento[registros[CS]].base + i,1,1,arch) == 1){
                        }
                        else{
                            printf("No se pudo leer el segmento codigo%d\n",i);
                        }
                    }
                    for(i=0;i<tablaSegmento[registros[KS]].tamanio;i++){
                        if(fread(memoria + tablaSegmento[registros[KS]].base + i,1,1,arch) == 1){
                        }
                        else{
                            printf("No se pudo leer el segmento constantes%d\n",i);
                        }
                    }
                    *resultado = 1;
                }    
            }
            else{
                if(strcmp(ident,"VMI25") == 0){
                    if(fread(&version, sizeof(char), 1, arch) == 1){
                        if(version == 1){
                            if(fread(&byte_aux, 1, 1, arch) == 1){
                                tamanio_mem_principal = (tamanio_mem_principal << 8) | byte_aux;
                                if(fread(&byte_aux, 1, 1, arch) == 1){
                                    tamanio_mem_principal = tamanio_mem_principal | byte_aux; // tercera linea del header de la vmi es el tamaño de la memoria principal
                                }
                                //toca leer registros uno por uno en el header de la vmi, luego la tabla de descriptores de segmentos
                                //para leer los vectores se leen byte a byte las siguientes 32 celdas de cuatro bytes, cada celda es un registro
                                //los registros vienen dados en el orden del vector de registros (el primero es el registro 0, el ultimo el registro 31)
                                //el primer byte de la celda es el primer (mas a la izquierda) byte del registro, el segundo es el segundo byte del registro y asi
                                for(i = 0; i < 32; i++){
                                    for(j = 0; j < 4; j++){
                                        if(fread(&byte_aux, 1, 1, arch) == 1){
                                            registros[i] = (registros[i] << 8) | byte_aux;
                                        }
                                    }
                                    if (registros[i] != 0xFFFFFFFF) registros[i] = registros[i] | (i << 16);//si guarda el dato normalmente (en el byte más bajo) lo muevo al más alto para usos posteriores
                                }
                                //leer la tabla de descriptores de segmentos
                                //son 8 celdas de 4 byts
                                //los primeros dos bytes indican la base del segmento, el tercer y cuarto byte el tamaño
                                //si el tamaño es cero, el numero de segmentos no se incrementa
                                for(i = 0; i < 8; i++){
                                    if(fread(&byte_aux, 1, 1, arch) == 1){
                                        base = (base << 8) | byte_aux;
                                        if(fread(&byte_aux, 1, 1, arch) == 1){
                                            base = (base << 8) | byte_aux;
                                            if(fread(&byte_aux, 1, 1, arch) == 1){
                                                tamanio = (tamanio << 8) | byte_aux;
                                                if(fread(&byte_aux, 1, 1, arch) == 1){
                                                    tamanio = (tamanio << 8) | byte_aux;
                                                    if(tamanio != 0){
                                                        tablaSegmento[*num_segmentos].base = base;
                                                        tablaSegmento[*num_segmentos].tamanio = tamanio;
                                                        *num_segmentos += 1;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                //luego byte por byte la memoria
                                //los bytes se añaden en el orden que se leen (el primer byte en la posicion 0 del vector memoria, el segundo en la 1 y asi)
                                for(i = 0; i < tamanio_mem_principal; i++){
                                    if(fread(&byte_aux, 1, 1, arch) == 1){
                                        memoria[i] = byte_aux;
                                    }
                                }
                            }
                            
                            *resultado = 1;
                        }
                                
                        else {
                            printf("ERROR: Versión incorrecta (esperado: 1, leído: %d)\n", (int)version);
                        }
                    }
                    else{
                        printf("No se pudo leer la version\n");
                    }
                }
            }
    }
    }
    
    else {
        printf("ERROR: No se pudo abrir el archivo '%s'\n", nombre);
    }
    
    if(*resultado == 0) {
        printf("ERROR: No se pudo leer el encabezado correctamente\n");
    }
    else{
        printf("Encabezado leido correctamente\n");
    }
    
    fclose(arch);
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
    registros[LAR] = segmento| direccion;
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
uint32_t get_segmento_registro(uint32_t operando, uint32_t registros[]) {
    uint8_t registro = operando & 0x1F;
    uint8_t segmento_registro = (operando >> 6) & 0x3;
    uint32_t valor = registros[registro];
    uint32_t resultado=0;

    switch (segmento_registro) {
        case 0: resultado=valor & 0xFFFFFFFF; 
                break;
        case 1: resultado=valor & 0xFF;
                break;
        case 2: resultado=(valor >> 8) & 0xFF;
                break;
        case 3: resultado=valor & 0xFFFF;
                break;
    }

    return resultado;
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
    //printf("DEBUG: LeerInstrucciones: Instruccion byte = %02X\n", instruccion);
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
            //printf("DEBUG: Listo para ejecutar OPC=%X \n", registros[OPC]);
            instrucciones[registros[OPC]](registros,memoria,tablaSegmento);
        }
    }
}
uint32_t get_segmento(uint8_t cod_reg, uint32_t registros[], infoSegmento tablaSegmentos[]){
    
    if(cod_reg < CS && cod_reg != BP && cod_reg != SP) cod_reg = DS; //El codigo de registro es uno de los de uso general
    
    return registros[cod_reg];
}
int32_t get(uint32_t operando,uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    // considerar caso de la funcion SYS donde la cantidad de bytes es impredecible
    int tipo_operando = (operando >> 24) & 0x00000003;
    uint8_t cod_reg = (operando >> 16) & 0x0000001F; // en caso de que el operando sea direccion de memoria saco los 5 bits que indicarian un registro
    uint8_t sub_segmento = (operando>>30) & 0x00000003;
    operando = operando & 0x0000FFFF;
    uint32_t direccion = registros[cod_reg] + (int16_t)(operando & 0x0000FFFF);//le saco el codigo de registro al operando y hago el casteo por si el offset es negativo

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
        direccion += 4-(4-sub_segmento);
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
void inicializar_stack(uint32_t registros[], uint8_t memoria[], infoSegmento tablaSegmentos[], int argc, char* argv[]){
    const int cantBytes = 4;
    uint32_t puntero_args = (argc == 0) ? 0xFFFFFFFF : (uint32_t)*argv;
    uint32_t cant_args = argc;
    int i;
    registros[SP] -= 4;
    // en la posicion más abajo del stack (el fondo de la pila) pongo el puntero al inicio del arreglo de argumentos
    for (i = 0; i < cantBytes; i++) {
        memoria[registros[SP] + i] = (puntero_args >> (8 * (cantBytes - 1 - i))) & 0x000000FF;
    }
    // en la segunda posicion del stack pongo la cantidad de argumentos
    registros[SP] -= 4;
    for (i = 0; i < cantBytes; i++) {
        memoria[registros[SP] + i] = (cant_args >> (8 * (cantBytes - 1 - i))) & 0x000000FF;
    }
    registros[SP] -= 4;
    for (i = 0; i < cantBytes; i++) {
        memoria[registros[SP] + i] = 0xFF;
    }
}
void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[], int argc, char* argv[]){
    //IP ya viene inicializado desde la lectura del encabezado
    
    uint16_t base = tablaSegmento[registros[CS] >> 16].base;
    uint16_t tamanio = tablaSegmento[registros[CS] >> 16].tamanio;
    
    if(registros[SS] != 0xFFFFFFFF) inicializar_stack(registros,memoria,tablaSegmento,argc,argv);
    leerInstrucciones(memoria[registros[IP]], memoria, registros, tablaSegmento);
    //registros[IP] = -1;
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

uint8_t detectarVersion(char *nombre) {
    FILE *arch = fopen(nombre, "rb");
    uint8_t version = 0;
    if (arch) {
        fseek(arch, 5, SEEK_SET);
        fread(&version, sizeof(uint8_t), 1, arch);
    }
    
    fclose(arch);
    return version;
}

void construirParamSegment(uint8_t *memoria, char *argv[], int argc_param, uint32_t *tamano_param_segment) {
    uint32_t offset = 0;    // donde escribir el proximo string
    uint32_t *punteros = NULL;  // punteros de 4 bytes
    int i, len;

    punteros = (uint32_t *)malloc(argc_param * sizeof(uint32_t));
    if (!punteros) {
        printf("Error: no se pudo reservar memoria para punteros de Param Segment\n");
        *tamano_param_segment = 0;
        return;
    }
    else{
        for (i = 0; i < argc_param; i++) {
            len = 0;
            while (argv[i][len] != '\0')
                len++;
            len++; //para el /0
            memcpy(memoria + offset, argv[i], len);
            punteros[i] = offset; //offset del string
            offset += len;
        }

        // agrego el arreglo argv de punteros al final
        for (i = 0; i < argc_param; i++) {
            uint32_t ptr_val = (0x0000 << 16) | (punteros[i] & 0xFFFF); //los primeros 16 bits que son siempre cero | 16 bits menos significativos (offset)
            // escribo los 4 bytes en big-endian
            memoria[offset + 0] = (ptr_val >> 24) & 0xFF; // byte mas significativo
            memoria[offset + 1] = (ptr_val >> 16) & 0xFF;
            memoria[offset + 2] = (ptr_val >> 8) & 0xFF;
            memoria[offset + 3] = ptr_val & 0xFF; // byte menos significativo

            offset += 4;
        }

        *tamano_param_segment = offset;
        free(punteros);
    }
    
}
