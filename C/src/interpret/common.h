#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

const int STACKSIZE=2047,CXMAX=10000,LEVMAX=100;

typedef enum _OPCOD  //������Ķ���
{
	LIT,LIT1,LOD,ILOD,LODA,LODT,LODB,STO,CPYB,JMP,JPC,RED,WRT,CAL,RETP,UDIS,OPAC,ENTP,
	ENDP,ANDS,ORS,NOTS,IMOD,MUS,ADD,ADD1,SUB,MULT,IDIV,EQ,NE,LS,LE,GT,GE
} OPCOD;

typedef struct _INSTRUCTION  //ָ��ṹ����
{
	OPCOD func;
	int level;
	int address;
} INSTRUCTION;

INSTRUCTION * CODE=NULL;  //һ�����INSTRUCTION��¼������


int pc,bp,top;
int oldTop;
INSTRUCTION instruction;  //��ǰ�������ָ��
int S[STACKSIZE];         //����������Ҫ��ջ
int DISPLAY[LEVMAX];      //DISPLAY��
int stop;                 //�ж��Ƿ�������ı�־
int h,hh,hhh;             //������ʱ����
char ch;                  //��ʱ����
int temp=0;               //��ʱ����
