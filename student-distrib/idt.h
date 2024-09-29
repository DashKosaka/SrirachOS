/* idt.h - Functions dealing with the interrupt descriptor table */

#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"
void init_idt();
void DIVIDE_BY_ZERO();
void DB();
void NMI();
void BP();
void OF();
void BR();
void UD();
void NM();
void DF();
void CSO();
void TS();
void NP();
void SS();
void GP();
void PF();
void MF();
void AC();
void MC();
void XF();
void KEYBOARD();
void DEFAULT();

#endif /* _IDT_H */
