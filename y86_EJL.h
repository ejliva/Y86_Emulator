/*
Earl John Liva
CEG-4350-01 Spring 2018
Project 3
*/


#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<ctype.h>

void setup();
void error(char*, int);
void setZF();
void clearZF();
int getZF();
void setSF();
void clearSF();
int getSF();
void setOF();
void clearOF();
int getOF();
void setAOK();
void setHLT();
void setADR();
void setINS();
int getStatus();
void clearStatus();
int getVal(char,char,char,char);
int* r1(char);
int* r2(char);
void printRegisters();
char * n1(char a);
char * n2(char a);
void error(char*, int);
void halt();
void nop();
void rrmovl(char);
void cmovle(char);
void cmovl(char);
void cmove(char);
void cmovne(char);
void cmovge(char);
void cmovg(char);
void irmovl(int, char);
void rmmovl(char,int);
void mrmovl(char,int);
void addl(char);
void subl(char);
void andl(char);
void xorl(char);
void jmp(int);
void jle(int);
void jl(int);
void je(int);
void jne(int);
void jge(int);
void jg(int);
void call(int);
void ret();
void pushl(char);
void popl(char);

int printMemory(int);
void parse(FILE*);
char toHex(char);
int decode();
void clearFlags();
char charToHex(char);
int isHex(char);
void setFlags(int a, int b, int c, int op);




