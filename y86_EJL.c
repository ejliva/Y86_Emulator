/*
Earl John Liva
CEG-4350-01 Spring 2018
Project 3
*/

#include"y86_EJL.h"
#define FALSE 0
#define TRUE 1

const int programLength = 0x500;
char p[programLength]; /* This is memory */

int littleEndian = TRUE;
int num_words;
int *eax, *ecx, *edx, *ebx, *esi, *edi, *esp, *ebp, pc;
char codes; /* status, condition codes[3]; control and status flags */
// TODO: Implement setting error code for MEM ERROR





/* Program start */
int main(int argc, char ** argv)
{   
    setup();
    char *input = argv[1];
    FILE * f;
    f = fopen(input, "r");
    printf("\nOpened file %s\n", input);
    //parse(f);
    printf("\nParsed %s\n", input);
    decode();
    return 0;
}

void setup()
{
        /* get the file from arc */
    eax = malloc(sizeof(int*));
    ecx = malloc(sizeof(int*));
    edx = malloc(sizeof(int*));
    ebx = malloc(sizeof(int*));
    esi = malloc(sizeof(int*));
    edi = malloc(sizeof(int*));
    esp = malloc(sizeof(int*));
    ebp = malloc(sizeof(int*));
    *eax = *ecx = *edx = *ebx = *esi = *edi = *esp = *ebp = pc = 0;
}

/* Parses the file into bytes ***************************
*********************************************************/
void parse(FILE * f)
{
    char * line = NULL;
    size_t length = 0;
    ssize_t read;
    if (f == NULL)
    {
        perror("Cannot open input file\n");
        exit(1);
    }
    while ((read = getline(&line, &length, f)) != -1) 
    {
        //printf("%s", line);
        int i; 
        int address; 
        char opcodes[6];
        int count = 0;
        for (i=0; i<length; i++) 
        {
            if (line[i] == '|') break;  
            if (line[i] == ' ') continue; 
            if (line[i] == '0' && line[i+1] == 'x')
            {
                address = (0x100 * charToHex(line[i+2])) + 
                          (0x010 * charToHex(line[i+3])) + 
                          (0x001 * charToHex(line[i+4])); 
                //printf("...address = 0x%2x ", address & 0xff);
            }
            if (line[i] == ':' && line[i+1] == ' ') 
            {
                //printf("...opcodes = ");
                i += 2; // start of the opcode
                int j=0;
                while (j<6 && isHex(line[i]))
                {
                    char n1 = charToHex(line[i]);
                    char n2 = charToHex(line[i+1]);
                    opcodes[j] = n1<<4 | n2;
                    //printf("%02x ", opcodes[j] & 0xff);
                    i+=2;
                    j++;
                } 
                count = j;
                //printf(" ... opcode length: %d\n", count);
            }
        }
        int j=0;
        int t = address;
        for (j=0; j<count; j++, t++)
            p[t] = opcodes[j];
    }
    int j=0; 
    printf("Memory:\n---------------------------\n");
    for (j=0; j<programLength; j++)
    {
        if (j % 8 == 0)
            printf("\n0x%02x\t: %02x", j, p[j] & 0xff);
        else
            printf(" %02x", p[j] & 0xff);
    }
    fclose(f);
    if (line)
        free(line);
}




/* Decodes the string of byte-sized characters and executes em **********
*************************************************************************/
int decode()
{
    /*
    take the first char
        - determine how many operands
        - call function with right operands
    */
    printf("Instructions\n---------------------------\n");
    for (pc=0; pc<programLength; )
    {
        printf("\n0x%02x:\t ", pc&0xff);
        char a = p[pc] & 0xf0;
        char b = p[pc] & 0x0f;
        switch (p[pc] & 0xf0)
        {
            case 0x00:
            {
                /* l=1, halt */
                if ((p[pc] & 0x0f) == 0x0)
                {
                    printf("%02x\t\t\t", p[pc] & 0xff);
                    halt();
                } else
                {
                    error("Error interpreting halt at pc=%02x\n", pc & 0xff);
                }
                break;
            }
            case 0x10:
            {
                /* l=1, nop */
                if ((p[pc] & 0x0f) == 0x0)
                {
                    printf("%02x\t\t", p[pc]);
                    nop();
                } else
                {
                    error("Error interpreting nop at pc=%02x", pc);
                }
                break;
            }
            case 0x20:
            {
                printf("%02x %02x \t\t\t", p[pc]&0xff, p[pc+1]&0xff);
                char reg = p[pc+1];
                /* l=2, mov 
                    rrmovl rA, rb     20 rArB
                    cmovle rA, rb     21 rArB
                    cmovl rA, rb      22 rArB
                    cmove rA, rB      23 rArB
                    cmovne rA, rB     24 rArB
                    cmovge rA, rB     25 rArB
                    cmovg rA, rB      26 rArB
                    */
                if ((p[pc]&0x0f) == 0x0)
                {
                    rrmovl(reg);
                }
                else if ((p[pc]&0x0f) == 0x1)
                {
                    cmovle(reg);
                }
                else if ((p[pc]&0x0f) == 0x2)
                {
                    cmovl(reg);
                }
                else if ((p[pc]&0x0f) == 0x3)
                {
                    cmove(reg);
                }
                else if ((p[pc]&0x0f) == 0x4)
                {
                    cmovne(reg);
                }
                else if ((p[pc]&0x0f) == 0x5)
                {
                    cmovge(reg);
                }
                else if ((p[pc]&0x0f) == 0x6)
                {
                    cmovg(reg);
                } else 
                {
                    setINS();
                    error("Error interpreting mov at pc=%02x", pc);
                }
                break;
            }
            case 0x30:
            {
                /* l=6, irmovl */
                /* irmovl V, rb      30 FrB V[4] */
               if ((p[pc] & 0x0f) == 0x0)
               {
                printf("%02x %02x %02x %02x %02x %02x\t", p[pc]&0xff, p[pc+1]&0xff, 
                    p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff, p[pc+5]&0xff);
                int val = getVal(p[pc+2], p[pc+3], p[pc+4], p[pc+5]);
                irmovl(val, p[pc+1]);
               } else
               {
                    setINS();
                    error("Error interpreting irmovl at pc=%02x", pc);
               }
               break;
            }
            case 0x40:
            {
                /* l=6, rmmovl */
                /* rmmovl rA, D(rB)  40 rArB D[4] */
                if ((p[pc]&0x0f) == 0x0)
                {
                    printf("%02x %02x %02x %02x %02x %02x\t\t", p[pc]&0xff, p[pc+1]&0xff, 
                        p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff, p[pc+5]&0xff);                    
                    int data = getVal(p[pc+2], p[pc+3], p[pc+4], p[pc+5]);
                    rmmovl(p[pc+1], data);
                } else
                {
                    setINS();
                    error("Error interpreting rmmovl at pc=%02x\n", pc);
                }
                break;
            }
            case 0x50:
            {
                /* l=6, mrmovl */
                /*     mrmovl D(rB), rA     50 rArB D[4] */
                if ((p[pc]&0x0f) == 0x0)
                {
                    printf("%02x %02x %02x %02x %02x %02x\t", p[pc]&0xff, p[pc+1]&0xff, 
                        p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff, p[pc+5]&0xff);                    
                    int data = getVal(p[pc+2], p[pc+3], p[pc+4], p[pc+5]);
                    mrmovl(p[pc+1], data);
                } else
                {
                    setINS();
                    error("Error interpreting mrmovl at pc=%02x", pc & 0xff);
                }
               break;
            }
            case 0x60:
            {
                /* l=2 */
                /* Arithmetic operations */
                printf("%02x %02x \t\t\t", p[pc]&0xff, p[pc+1]&0xff);
                char reg = p[pc+1];
                /* 
                addl rA, rB       60 rArB
                subl rA, rB       61 rArB
                andl rA, rB       62 rArB
                xorl rA, rB       63 rArB
                */
                if ((p[pc]&0x0f) == 0x0)
                {
                    addl(reg);
                } else if ((p[pc]&0x0f) == 0x1)
                {
                    subl(reg);
                } else if ((p[pc]&0x0f) == 0x2)
                {
                    andl(reg);
                } else if ((p[pc]&0x0f) == 0x3)
                {
                    xorl(reg);
                } else
                {
                    setINS();
                    error("Problem parsing op at pc=%02x\n", pc);
                }
                break;
            }
            case 0x70:
            {
                /* l=5, jmps */
                printf("%02x %02x %02x %02x %02x\t\t", p[pc]&0xff, p[pc+1]&0xff, 
                    p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff);                    
                int dest = getVal(p[pc+1], p[pc+2], p[pc+3], p[pc+4]);
                /*
                jmp Dest          70 Dest[4]
                jle Dest          71 Dest[4]
                jl Dest           72 Dest[4]
                je Dest           73 Dest[4]
                jne Dest          74 Dest[4]
                jge Dest          75 Dest[4]
                jg Dest           76 Dest[4]
                */
                if ((p[pc]&0x0f) == 0x0)
                {
                    jmp(dest);
                } else if ((p[pc]&0x0f) == 0x1)
                {
                    jle(dest);
                } else if ((p[pc]&0x0f) == 0x2)
                {
                    jl(dest);
                } else if ((p[pc]&0x0f) == 0x3)
                {
                    je(dest);
                } else if ((p[pc]&0x0f) == 0x4)
                {
                    jne(dest);
                } else if ((p[pc]&0x0f) == 0x5)
                {
                    jge(dest);
                } else if ((p[pc]&0x0f) == 0x6)
                {
                    jg(dest);
                } else 
                {
                    setINS();
                    error("Error interpreting jump at pc=%02x", pc);
                }
                break;
            }
            case 0x80:
            {
                /* l=5, call */
                /*     call  80 Dest[4] */
                if ((p[pc]&0x0f) == 0x0)
                {
                    printf("%02x %02x %02x %02x %02x\t\t", p[pc]&0xff, p[pc+1]&0xff, 
                        p[pc+2]&0xff, p[pc+3]&0xff, p[pc+4]&0xff);                    
                    int dest = getVal(p[pc+1], p[pc+2], p[pc+3], p[pc+4]);
                    call(dest);                
                } else 
                {
                    setINS();
                    error("Error interpreting call at pc=%02x\n", pc);
                }
                break;
            }
            case 0x90:
            {
                /* l=1, ret */
                /*     ret   90 */
               if ((p[pc]&0x0f) == 0x0)
               {
                    printf("%02x  \t\t\t", p[pc]&0xff);
                    ret();
               } else
               {
                    setINS();
                    error("Error interpreting ret at pc=%02x\n", pc);
               }
               break;
            }
            case 0xa0:
            {
                /* l=2, pushl */
                /* pushl rA          A0 rAF */
                if ((p[pc]&0x0f) == 0x0)
                {
                    printf("%02x %02x \t\t\t", p[pc]&0xff, p[pc+1]&0xff);
                    pushl(p[pc+1]&0xf0);
                } else 
                {   
                    setINS();
                    error("Error interpreting pushl at pc=%02x\n", pc);
                }
               break;
            }
            case 0xb0:
            {
                /* l=2, popl */
                /* popl rA           B0 rAF */
                if ((p[pc]&0x0f) == 0x0)
                {
                    printf("%02x %02x \t\t\t", p[pc]&0xff, p[pc+1]&0xff);
                    popl(p[pc+1]&0xf0);
                } else
                {
                    setINS();
                    error("Error interpreting pushl at pc=%02x\n", pc);
                }
                break;
            }
            default:
            {
                setINS();
                error("Error interpreting instruction at pc=%02x\n", pc);
            }
        }
    }
    printRegisters();
    return 0;
}




/*
    x | ZF | SF | OF |  1 | 1 | 1 | 1
    1 AOK  Normal operation
    2 HLT  halt instruction encountered
    3 ADR  Invalid address encountered
    4 INS  Invalid instruction encountered
    *************************************
    ZF  zero flag
    SF  sign flag
    OF  overflow flag
*/

/* Zero flag operations */
void setZF()
{  
    codes = codes | 64; /* sets the 01000000 flag */
    printf("(ZF)");
}

void clearZF()
{
    codes = codes & 64; /* clears 10111111 flag */
    // if set, clear
    // if clear, clear
}

int getZF()
{
    if ((codes & 64) == 64)
        return TRUE;
    return FALSE;
}

void clearFlags()
{
    clearSF();
    clearZF();
    clearOF();
}


/* Sign Flag operations */
void setSF()
{
    codes = codes | 32; /* sets the 00100000 flag */
    printf("(SF)");
}

void clearSF()
{
    codes = codes & 32;
}

int getSF()
{
    if ((codes & 32) == 32)
        return TRUE;
    return FALSE;
}


/* Overflow flag operations */
void setOF()
{
    codes = codes | 16; /* sets the 00010000 flag */
    printf("(OF)");
}

void clearOF()
{
    codes = codes & 16;
}

int getOF()
{
    if ((codes & 16) == 16)
        return TRUE;
    return FALSE;
}




/* Status code operations (last four bits) */
void setAOK()
{
    clearStatus();
    codes = codes | 1; /* sets 00000001 */
    printf("(set status = AOK)");
}


void setHLT()
{
    clearStatus();
    codes = codes | 2; /* sets 00000010 */
    printf("(set status = HLT)");
}

void setADR()
{
    clearStatus();
    codes = codes | 3; /* sets 00000011 */
    printf("(set status = ADR)");
}

void setINS()
{
    clearStatus();
    codes = codes | 4; /* sets 00000010 */
    printf("(set status = INS)");  
}

int getStatus()
{
    return codes & 0xf0;
}

void clearStatus()
{
    codes = codes & (255 - 15); /*11110000 - clears last four bits */
}



/* Generates an error then exits the program 
*/ 
void error(char * words, int pc)
{
    printf("%s", words);
    exit(1);
}





/*************************************************/
/*********FUNCTIONS IMPLEMENTING ASSEMBLY*********/




/**     
*    halt              00 */
void halt()
{
    printf("halt");
    printRegisters();
    setHLT();
    pc+=1;
    exit(0);
}



/**  nop               10  */
void nop()
{
    printf("nop");
    pc+=1;
}


/**  rrmovl rA, rb     20 rArB   */
void rrmovl(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    *dst = *src;
    printf("rrmovl %02x, %02x", *src, *dst);
    pc+=2;
}



/**     cmovle rA, rb     21 rArB  */
void cmovle(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    if (getZF() || getSF() != getOF())
    {
        *dst = *src;
        printf("cmovle %x, %x (moved)", *src, *dst);
    } else 
        printf("cmovle %x, %x (not moved)", *src, *dst);
    pc+=2;
}


/**      cmovl rA, rb      22 rArB   */
void cmovl(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    if (getSF() != getOF())
    {
        *dst = *src;
        printf("cmovl %02x, %02x (moved)", *src, *dst);
    } else
        printf("cmovl %02x, %02x (not moved)", *src, *dst);
    pc+=2;
}



/**     cmove rA, rB      23 rArB  */
void cmove(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    if (getZF() == 1)
    {
        *dst = *src;   
        printf("cmove %02x, %02x (moved)", *src, *dst);    
    } else
        printf("cmove %02x, %02x (not moved)", *src, *dst);    
    pc+=2;
}







/**     irmovl V, rb      30 FrB Va Vb Vc Vd  */
void irmovl(int val, char reg)
{
    int *rB = r2(reg);
    *rB = val;
    printf("irmovl rB, %02x", *rB);
    pc+=6;
}



/**     rmmovl rA, D(rB)     40 rArB Da Db Dc Dd  */
void rmmovl(char reg, int offset)
{
    int * rA = r1(reg);
    int * rB = r2(reg);
    p[*rB + offset] = *rA;
    printf("rmmovl rA, %02x(%02x)", offset, *rB);
    pc+=6;
}





/**     addl rA, rB          60 rArB  */
void addl(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    int tmp = *dst;
    *dst = *dst + *src;
    printf("addl %s, %s: (%02x)", n1(reg), n2(reg), *dst);
    setFlags(tmp, *src, *dst, 0);
    pc+=2;
}





/**     jmp Dest          70 Da Db Dc Dd  */
void jmp(int dest)
{
    printf("jmp %02x", dest);
    pc = dest;
    printf(" (pc=%02x)", dest);
}



/**     je Dest           73 Da Db Dc Dd  */
void je(int dest)
{
    printf("je %02x", dest);
    if (getZF() == 1)
    {
        pc = dest;
        printf(" (pc=%02x)", dest);
    } else
    {
        printf(" (not taken)");
        pc+=5;
    }
}


/**     jne Dest          74 Da Db Dc Dd  */
void jne(int dest)
{
    printf("jne %02x", dest);
    if (getZF() == 0)
    {
        pc = dest;
        printf(" (pc=%02x)", pc&0xff);
    } else
    {
        printf(" (not taken)");
        pc+=5;
    }
}



/*     call              80 Da Db Dc Dd  */
void call(int addr)
{
    printf("call %02x", addr);
    *esp = *esp - 0x4; /* make entry on stack */
    p[*esp] = pc;     /* push old pc onto stack */
    pc = addr;
}



/*     ret               90   */
void ret()
{
    printf("ret");
    pc = p[*esp];     /* restore pc from stack */
    *esp = *esp + 0x4; /* move esp back down */
    pc += 5;  /* for how many bytes it took to call */
}


/*     pushl rA          A0 rAF   */
void pushl(char reg)
{
    int * rA = r1(reg);
    *esp = *esp - 4;    /* make entry on the stack */
    p[*esp] = *rA;      /* put rA into it */
    printf("pushl %02x", *rA);
    pc+=2;
}


/*      popl rA           B0 rAF   */
void popl(char reg)
{
    int * rA = r1(reg);
    *rA = p[*esp];      /* pull top stack value into rA */
    *esp = *esp + 0x4;  /* remove entry from the stack */
    printf("popl %02x", *rA);
    pc+=2;
}



/**************************************/
/******UTILITY FUNCTIONS***************/

char charToHex(char x)
{
//    printf("x:%02x", x);
    if (x >= '0' && x <= '9') /* chars 0-9 */
        x = (x - ('0' - 0x0)); 
    else if (x >= 'a' && x <= 'f') /* a-f */
        x = (x - ('a' - 0xA));     
    else if (x >- 'A' && x <= 'F') /* A-F */
        x = (x - ('A' - 0xA));   
    else
    {
        printf("There was an error translating a char to hex: %c", x);
        exit(1);
    }
//    printf("->%02x\n", x);
    return x;
}


int isHex(char x)
{
    if ((x >= '0' && x <= '9' )|| 
        (x >= 'a' && x <= 'f' )||
        (x >= 'A' && x <= 'F'))
        return 1;
    return 0;
}

int printMemory(int start)
{
    int words_on_screen = 1000;

    for (int i=start; i<start+words_on_screen; i++)
    {
        if (i==(0%4))
            printf("");
        printf("%02x\t", p[i]);
    }
    return 0;
}


/**
**  Gets an integer value from four bytes (based on big 
**   endian or little endian encoding)
**/
int getVal(char a, char b, char c, char d)
{
    int val;
    if (littleEndian)
    {
//        printf("      getVal: %02x %02x %02x %02x \n ",a,b,c,d);
        val = a;
        val = val | b<<8;
        val = val | c<<12;
        val = val | d<<16;
//        printf(": %02x ", val);
    } else /* big endian */
    {
        val = a;
        val = val | b<<4;
        val = val | c<<8;
        val = val | d<<12;
    }
    return val;
}

/* 
    Computes the register from the first part of the byte 
    passed in. Returns a pointer to the register given 
    character code as input.
    0  eax; 1 ecx; 2 edx; 3  ebx
    4  esp; 5 ebp; 6 esi; 7  edi
    F  No register
*/
int * r1(char a)
{
//    printf("r2: %02x, r2 & 0x0f: %02x", a, (a & 0xf0)); 
    switch (a & 0xf0)
    {
        case 0x00:  return eax;
        case 0x10:  return ecx;
        case 0x20:  return edx;
        case 0x30:  return ebx;
        case 0x40:  return esp;
        case 0x50:  return ebp;
        case 0x60:  return esi;
        case 0x70:  return edi;
        case 0xf0:  return 0; 
        default:
            error("Error determining register value. PC=0x%02x", pc & 0xff);
    }
    return eax; /* shouldn't be hit */
}

/* 
    Computes the register from the second part of the byte 
    passed in. Returns a pointer to the register given 
    character code as input.
    0  eax; 1 ecx; 2 edx; 3  ebx
    4  esp; 5 ebp; 6 esi; 7  edi
    F  No register
*/
int * r2(char a)
{
//    printf("r2: %02x, r2 & 0x0f %02x", a, (a & 0x0f));
    switch (a & 0x0f)
    {
        case 0x00:  return eax;
        case 0x01:  return ecx;
        case 0x02:  return edx;
        case 0x03:  return ebx;
        case 0x04:  return esp;
        case 0x05:  return ebp;
        case 0x06:  return esi;
        case 0x07:  return edi;
        case 0x0f:  return 0; 
        default:
            error("Error determining register value. PC=0x%02x", pc & 0xff);
    }
    return eax; /* shouldn't be hit */
}

char * n1(char a)
{
//    printf("r2: %02x, r2 & 0x0f: %02x", a, (a & 0xf0)); 
    switch (a & 0xf0)
    {
        case 0x00:  return "\%eax";
        case 0x10:  return "\%ecx";
        case 0x20:  return "\%edx";
        case 0x30:  return "\%ebx";
        case 0x40:  return "\%esp";
        case 0x50:  return "\%ebp";
        case 0x60:  return "\%esi";
        case 0x70:  return "\%edi";
        case 0xf0:  return "0"; 
        default:
            error("Error determining register value for n1. reg=%c\n", a);
    }
    return "err"; /* shouldn't be hit */
}

char * n2(char a)
{
//    printf("r2: %02x, r2 & 0x0f %02x", a, (a & 0x0f));
    switch (a & 0x0f)
    {
        case 0x00:  return "\%eax";
        case 0x01:  return "\%ecx";
        case 0x02:  return "\%edx";
        case 0x03:  return "\%ebx";
        case 0x04:  return "\%esp";
        case 0x05:  return "\%ebp";
        case 0x06:  return "\%esi";
        case 0x07:  return "\%edi";
        case 0x0f:  return "0"; 
        default:
            error("Error determining register value for n1. reg=%c\n", a);
    }
    return "err"; /* shouldn't be hit */
}

/*
*  Prints out the values of the registers */
void printRegisters()
{
    printf("\nEAX:0x%08x \t " , *eax & 0xffffffff);
    printf("ECX:%08x \t  " , *eax & 0xffffffff);
    printf("EDX:%08x \t ", *eax & 0xffffffff);
    printf("EBX:%08x \n ", *eax  & 0xffffffff);
    printf("ESP:%08x \t ", *eax  & 0xffffffff);
    printf("EBP:%08x \t ", *eax & 0xffffffff);
    printf("ESI:%08x \t ", *eax & 0xffffffff);
    printf("EDI:%08x \t ", *eax & 0xffffffff);
    printf("PC:0x%03x \n", pc & 0xffffffff);
}


/**************************************/

/* TODO 0: Verify existing instructions work correctly with very small assembly programs */


/**     cmovne rA, rB     24 rArB  */
void cmovne(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    if (getZF() == 0)
    {
        *dst = *src;
        printf("cmovne %02x, %02x (moved)", *src, *dst);
    } else
        printf("cmovne %02x, %02x (not moved)", *src, *dst);
    pc+=2;
}


/**     cmovge rA, rB     25 rArB  */
void cmovge(char reg)
{
    /* TODO 2: Implement the cmovge instruction */
    int * src = r1(reg);
    int * dst = r2(reg);
    if (getSF() == getZF())
    {
        *dst = *src;   
        printf("cmovge %02x, %02x (moved)", *src, *dst);
    } else
        printf("cmovge %02x, %02x (not moved)", *src, *dst);
    pc+=2;
}


/*     cmovg rA, rB      26 rArB   */
void cmovg(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    if (getSF() == 0 && getZF() == 0)
    {
        *dst = *src;   
        printf("cmovg %02x, %02x (moved)", *src, *dst);
    } else
        printf("cmovg %02x, %02x (not moved)", *src, *dst);
    pc+=2;
}


/**     mrmovl D(rB), rA      50 rArB Da Db Dc Dd  */
void mrmovl(char reg, int offset)
{
    int * rA = r1(reg);
    int * rB = r2(reg);
    *rA = p[*rB + offset];
    printf("mrmovl %02x(%02x), %02x", offset, *rB, *rA);
    pc+=6;
}

/** Sets flags based on the last result
See rules for setting the overflow and carry flags:
http://teaching.idallen.com/dat2343/10f/notes/040_overflow.txt
op reflects the type of operation that was performed last: 
op == 0 for add
op == 1 for subtract
op == 2 for all others
 */
void setFlags(int a, int b, int c, int op)
{
    clearFlags();        // start with fresh flag values
    if (c == 0) setZF(); // the result was 0 so ZF is 1
    if (c < 0) setSF();  // result is negative, so SF is 1
    // now set overflow flag:
    a = a & 0x80000000; // to check carry out - has only bit 31 set */
    b = b & 0x80000000; // same as pow(2, 31)
    c = c & 0x80000000;
    if ((op == 0) && (a == b) && (a != c)) setOF();
    if ((op == 1) && (a != b) && (a != c)) setOF();
}


/**      subl rA, rB        61 rArB  */
void subl(char reg)
{
    
    int * src = r1(reg);
    int * dst = r2(reg);
    int tmp = *dst;
    *dst = *dst - *src;
    printf("subl %s, %s: (%02x)", n1(reg), n2(reg), *dst);
    setFlags(tmp, *src, *dst, 1);
    pc+=2;
}


/**     andl rA, rB       62 rArB  */
void andl(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    int tmp = *dst;
    *dst = *dst & *src;
    printf("andl %s, %s: (%02x)", n1(reg), n2(reg), *dst);
    setFlags(tmp, *src, *dst, 2);
    pc+=2;
}


/**     xorl rA, rB       63 rArB  */
void xorl(char reg)
{
    int * src = r1(reg);
    int * dst = r2(reg);
    int tmp = *dst;
    *dst = *dst ^ *src;
    printf("xorl %s, %s: (%02x)", n1(reg), n2(reg), *dst);
    setFlags(tmp, *src, *dst, 2);
    pc+=2;
}

/**     jle Dest          71 Da Db Dc Dd  */
void jle(int dest)
{
    printf("jle %02x", dest);
    if (getZF() || getSF() != getOF())
    {
        pc = dest;
        printf(" (pc=%02x)", dest);
    } else
    {
        printf(" (not taken)");
        pc+=5;
    }
}


/**     jl Dest           72 Da Db Dc Dd  */
void jl(int dest)
{
    printf("jl %02x", dest);
    if (getZF() != getOF())
    {
        pc = dest;
        printf(" (pc=%02x)", dest);
    } else
    {
        printf(" (not taken)");
        pc+=5;
    }
}


/**     jge Dest          75 Da Db Dc Dd  */
void jge(int dest)
{
    printf("jge %02x", dest);
    if (getZF() == getSF())
    {
        pc = dest;
        printf(" (pc=%02x)", dest);
    } else
    {
        printf(" (not taken)");
        pc+=5;
    }
}



/**     jg Dest           76 Da Db Dc Dd  */
void jg(int dest)
{
    printf("jg %02x", dest);
    if (getZF() == 0 && getSF() == 0)
    {
        pc = dest;
        printf(" (pc=%02x)", dest);
    } else
    {
        printf(" (not taken)");
        pc+=5;
    }
}




