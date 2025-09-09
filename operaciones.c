#include "maquina.h"

void JMP(uint32_t OP1, uint32_t registros[]){
    registros[IP] = registros[OP1];
}

void JZ(uint32_t OP1, uint32_t registros[]){
    if((registros[CC] & 0x01) == 1)
        registros[IP] = registros[OP1];
}
void JP(uint32_t OP1, uint32_t registros[]){
    if(registros[CC] == 0)
        registros[IP] = registros[OP1];
}
void JN(uint32_t OP1, uint32_t registros[]){
    if((registros[CC] & 0x02) == 2)
        registros[IP] = registros[OP1];
}
void JNZ(uint32_t OP1, uint32_t registros[]){
    if((registros[CC] & 0x01) == 0)
        registros[IP] = registros[OP1];
}
void JNP(uint32_t OP1, uint32_t registros[]){
    if((registros[CC] & 0x03) == 2 || (registros[CC] & 0x03) == 1)
        registros[IP] = registros[OP1];
}
void JNN(uint32_t OP1, uint32_t registros[]){
    if((registros[CC] & 0x02) < 2)
        registros[IP] = registros[OP1];
}

void NOT(uint32_t OP1, uint32_t registros[]){
    
}