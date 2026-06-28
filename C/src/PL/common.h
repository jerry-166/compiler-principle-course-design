#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>



#define FILENAMESIZE 512    //�ļ�������󳤶�
#define NUMOFWORD 19        //�����ֵĸ���
#define MAXNUMOFNAMETABLE 1000  //���ֱ����������
#define MAXNUMOFBLOCKTABLE 100 //����������������
#define MAXNUMOFARRAYTABLE  100//������Ϣ�����������
#define MAXSYMNAMESIZE 255  //���ŵ�����ַ���
#define MAXVARNAMESIZE 255  //������������ַ���
#define MAXLEVELDEPTH 100   //����Ƕ�׵�������
#define MAXNUMOFCODEADDRESS 10000  //���ĵ�ַ�ռ�


typedef enum _SYMBOL  //���ŵĶ���
{
	NUL,//0
	IDENT,//1
    INTCON,//2
	CHARCON,//3
	PLUS,//4
	MINUS,//5
	TIMES,//6
	DIVSYM,//7
    EQL,//8
	NEQ,//9
	LSS,//10
	LEQ,//11
	GTR,//12
	GEQ,//13
	OFSYM,//14
	ARRAYSYM,//15
	PROGRAMSYM,//16
	MODSYM,//17
    ANDSYM,//18
	ORSYM,//19
	NOTSYM,//20
	LBRACK,//21
	RBRACK,//22
	LPAREN,//23
	RPAREN,//24
	COMMA,//25
    SEMICOLON,//26
	PERIOD,//27
	BECOMES,//28
	COLON,//29
	BEGINSYM,//30
	ENDSYM,//31
	IFSYM,//32
	THENSYM,//33
    ELSESYM,//34
	WHILESYM,//35
	DOSYM,//36
	CALLSYM,//37
	CONSTSYM,//38
	TYPESYM,//39
    VARSYM,//40
	PROCSYM,//41
	DPOINT//42
} SYMBOL;


class SYMLIST:public CList<SYMBOL,SYMBOL>
{
};


SYMLIST DECLBEGSYS,STATBEGSYS,FACBEGSYS,CONSTBEGSYS,TYPEBEGSYS;

//typedef signed short int int;


typedef enum NAMEKIND  //��ʶ��������Ķ���
{
	KONSTANT,  //����
	TYPEL,  //������
	VARIABLE,  //����
	PROCEDURE  //������
} OBJECT;


typedef enum VARTYPES  //���������͵Ķ���
{
	NOTYP,  //����ʱ��û������
	INTS,  //��������
	CHARS,  //�ַ�����
	BOOLS,  //��������
	ARRAYS  //��������
} TYPES;


typedef enum _OPCOD  //������Ķ���
{
	LIT,LIT1,LOD,ILOD,LODA,LODT,LODB,STO,CPYB,JMP,JPC,RED,WRT,CAL,RETP,UDIS,OPAC,ENTP,
	ENDP,ANDS,ORS,NOTS,IMOD,MUS,ADD,ADD1,SUB,MULT,IDIV,EQ,NE,LS,LE,GT,GE
} OPCOD;



typedef struct _TYPEITEM  //�����ڵ㶨��
{
	TYPES typ;
	int ref;
} TYPEITEM;

typedef struct _CONSTREC
{
	TYPES type;
	int value;
} CONSTREC;



typedef struct SYMBOL_ITEM  //��Դ�ļ������ķ�����������Ľڵ�Ķ���
{
	int lineNumber;  //��������������
	SYMBOL type;  //���ŵ�����
	union SYMBOL_VALUE  //���ݲ�ͬ�ķ������ͷ��ſ����в�ͬ�����͵�ֵ
	{
		int iValue;  //������������������ַ�������������ֵ
		char * lpValue;  //����Ƿ��ţ����������ַ��ĵ�ַ
	}value;
	struct SYMBOL_ITEM *next;  //ָ����һ���ڵ�
}SymbolItem;



typedef struct _INSTRUCTION  //ָ��ṹ����
{
	int lineNumber;
	OPCOD func;
	int level;
	int address;
} INSTRUCTION;
INSTRUCTION CODE[MAXNUMOFCODEADDRESS];  //ָ�����鶨��
int CX;  //ָ����������



typedef struct _NAMETABITEM  //���ֱ�����ļ�¼�Ķ���
{
	char name[MAXSYMNAMESIZE];
	OBJECT kind;
	TYPES type;
	int level;
	int normal;
	int ref;
	union SYMBOL_VALUE  //���ݲ�ͬ�ķ������ͷ��ſ����в�ͬ�����͵�ֵ
	{
		int value;  //������������������ַ�������������ֵ
		int size;
		int address;
	}unite;
	int link;
	//struct NAMETAB_ITEM *Next;
}NAMETABITEM;
NAMETABITEM NAMETAB[MAXNUMOFNAMETABLE];  //���ֱ�����
int TX; //���ֱ�����



typedef struct PROGRAM_BLOCK_ITEM  //���������ļ�¼�ĵĶ���
{
	int lastPar;
	int last;
	int pSize;
	int vSize;
	//struct PROGRAM_BLOCK_ITEM *Next;
}BLOCKITEM;
BLOCKITEM BTAB[MAXNUMOFBLOCKTABLE];  //���������
int BX;  //�����������





typedef struct ARRAY_INFORMATION_ITEM  //������Ϣ������
{
	TYPES intType;
	TYPES eleType;
	int low;
	int high;
	int elSize;
	int size;
	int elRef;
	//struct ARRAY_INFORMATION_ITEM *Next;
}ARRAYITEM;
ARRAYITEM  ATAB[MAXNUMOFARRAYTABLE];  //������Ϣ������
int AX;  //������Ϣ������



int JUMADRTAB[300];  //��ת���Ķ���
int JX;  //��ת������



int DISPLAY[MAXLEVELDEPTH];  //DISPLAY������
int displayLevel;  //DISPLAY��������

int DX;//���ݿռ�����ָʾ��,ÿ�������嶼Ҫ�õ�һ��

//#include "global.h"
//#include "symlist.h"
char ObjCodeScript[GE+1][1000]=
{
	"LIT    %4d ,%4d      ------>  װ�볣��",
	"LIT1   %4d ,%4d      ------>  װ�볣�������ڼ�������Ԫ�ص�ַ��",
	"LOD    %4d ,%4d      ------>  װ�����ֵ",
	"ILOD   %4d ,%4d      ------>  ���װ��",
	"LODA   %4d ,%4d      ------>  װ�������ַ",
	"LODT   %4d ,%4d      ------>  װ��ջ��ֵΪ��ַ������",
	"LODB   %4d ,%4d      ------>  װ�볤��ΪA�Ŀ�",
	"STO    %4d ,%4d      ------>  ��ջ��ֵ����ջ����ֵ��ָ��Ԫ",
	"CPYB   %4d ,%4d      ------>  ���ͳ���ΪA�Ŀ�",
	"JMP    %4d ,%4d      ------>  ��������ת",
	"JPC    %4d ,%4d      ------>  ջ��ֵΪ0ʱ��ת",
	"READ   %4d ,%4d      ------>  ��ָ��",
	"WRITE  %4d ,%4d      ------>  дָ��",
	"CALL   %4d ,%4d      ------>  ת��",
	"RETP   %4d ,%4d      ------>  ���̷���",
	"UDIS   %4d ,%4d      ------>  ����Display",
	"OPAC   %4d ,%4d      ------>  �򿪻��¼",
	"ENTP   %4d ,%4d      ------>  �������",
	"ENDP   %4d ,%4d      ------>  �������",
	"AND    %4d ,%4d      ------>  ��",
	"OR     %4d ,%4d      ------>  ��",
	"NOT    %4d ,%4d      ------>  ��",
	"IMOD   %4d ,%4d      ------>  ģ",
	"MUS    %4d ,%4d      ------>  ��",
	"ADD    %4d ,%4d      ------>  ��",
    "ADD1   %4d ,%4d      ------>  �ӣ����ڼ�������Ԫ�ص�ַ��",
	"SUB    %4d ,%4d      ------>  ��",
	"MULT   %4d ,%4d      ------>  ��",
	"IDIV   %4d ,%4d      ------>  ��",
	"EQ     %4d ,%4d      ------>  ==",
	"NEQ    %4d ,%4d      ------>  !=",
	"LSS    %4d ,%4d      ------>  <",
	"LEQ    %4d ,%4d      ------>  <=",
	"GTR    %4d ,%4d      ------>  >",
	"GEQ    %4d ,%4d      ------>  >="
};