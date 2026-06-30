// PL.cpp : Defines the entry point for the console application.
//
#include "common.h"

// iscsymf / iscsym 是 Microsoft VC 的 ctype 扩展，Linux <ctype.h> 没有。
// 这里用标准 ctype 给出等价实现，使 getSymbols 无需改动：
//   iscsymf(c) = 标识符首字符（字母或下划线）
//   iscsym(c)  = 标识符后续字符（字母、数字、下划线）
static inline int iscsymf(int c) { return (isalpha(c) || c == '_'); }
static inline int iscsym(int c)  { return (isalnum(c) || c == '_'); }

////////////////////////////////////////////////////////////
////////////////////////����ͷ��������//////////////////////
////////////////////////////////////////////////////////////

SYMLIST * listsAdd(SYMLIST * list1,SYMLIST * list2);
SYMLIST * listAddSym(SYMLIST * list,SYMBOL sym);
int SYMINLIST(SYMBOL sym,SYMLIST * list);
void COPYLIST(SYMLIST * list1,SYMLIST * list2);

void error(int);
int nError;

void INITIAL();
void ENTERID();
void ENTERPREID();

void getSymbols(FILE *);
void getASymbol();
void destroySymbols();
SymbolItem *Symbols=NULL;
SymbolItem *CurSymbol=NULL;

void GEN(OPCOD func,int level,int address);
void WriteObjCode(char *);
void WriteCodeList(char *);

void ENTERARRAY(SYMLIST * list,TYPES type,int low,int high);
void ENTERBLOCK();
void ENTER(OBJECT object);
int GETPOSITION(char * id);
void CONSTANT(SYMLIST * list,CONSTREC & constRec);
void ARRAYTYP(SYMLIST * list,int & aref,int & arsz);
void TYP(SYMLIST * list,TYPES & tp,int & rf,int & sz);
void PARAMENTERLIST(SYMLIST * list);

void CONSTDECLARATION(SYMLIST * list);
void TYPEDECLARATION(SYMLIST * list);
void VARDECLARATION(SYMLIST * list);
void PROCDECLARATION(SYMLIST * list);

void FACTOR(SYMLIST * list,TYPEITEM & typeItem);
void TERM(SYMLIST * list,TYPEITEM & typeItem);
void SIMPLEEXPRESSION(SYMLIST * list,TYPEITEM & typeItem);
void EXPRESSION(SYMLIST * list,TYPEITEM & typeItem);
void ARRAYELEMENT(SYMLIST * list,TYPEITEM & typeItem);

void ASSIGNMENT(SYMLIST * list);
void IFSTATEMENT(SYMLIST * list);
void WHILESTATEMENT(SYMLIST * list);
void COMPOUND(SYMLIST * list);
void STANDPROC(SYMLIST * list,int i);
void CALL(SYMLIST * list);
void STATEMENT(SYMLIST * list);
void BLOCK(SYMLIST * list,int level);

////////////////////////////////////////////////////////////
////////////////////�������̶�������////////////////////////
////////////////////////////////////////////////////////////

void GEN(OPCOD func,int level,int address) //�������Ĵ��뱣�浽��������CODE����
{										   //������������CX���� 1	
	static int lineNumber=0;
	if(CX>MAXNUMOFCODEADDRESS)
	{
		printf("PROGRAM TOO LONG!");
		exit(0);
	}

	printf("%d\t",lineNumber);                 //����������������ڱ���Ĺ�������ʾ����
	printf(ObjCodeScript[func],level,address); //ע�⣺��ʵ�Ĵ��벻����ȫ�ģ����ǿ��԰�������
	printf("\n");

	CODE[CX].lineNumber=lineNumber++;
	CODE[CX].func=func;
	CODE[CX].level=level;
	CODE[CX].address=address;
	CX++;
}



void WriteObjCode(char *filename)  //�������Ĵ���д��*.pld�ļ����棨��������ʽ��
{
	FILE *fcode;
	
	fcode=fopen(filename,"wb");
	if(!fcode)
		error(40);  //���ܴ�.pld�ļ�
	for(int i=0;i<CX;i++)
	{
		fwrite(&CODE[i].func,sizeof(OPCOD),1,fcode);
		fwrite(&CODE[i].level,sizeof(int),1,fcode);
		fwrite(&CODE[i].address,sizeof(int),1,fcode);
	}
	fclose(fcode);	
}

void WriteCodeList(char *filename)  //�������Ĵ���д��*.lst�ļ����ɼ��ַ���ʽ��
{
	FILE *flist;
	
	flist=fopen(filename,"wb");
	if(!flist)
		error(39);  //���ܴ�.lst�ļ�
	for(int i=0;i<CX;i++)
	{
		fprintf(flist,"%d\t",i);
		fprintf(flist,ObjCodeScript[CODE[i].func],CODE[i].level,CODE[i].address);
		fprintf(flist,"\n");
	}
	fclose(flist);
}

void WriteLabelCode(char *filename)  //�������Ĵ���д��*.lab�ļ����ɼ��ַ���ʽ��
{
	FILE *flabel;
	
	flabel=fopen(filename,"wb");
	if(!flabel)
		error(41);  //���ܴ�.lst�ļ�
	for(int i=0;i<JX;i++)
	{
		fprintf(flabel,"��%d��:\t",i+1);
		fprintf(flabel,"%d",JUMADRTAB[i]);
		fprintf(flabel,"\n");
	}
	fclose(flabel);
}

void ENTERARRAY(TYPES type,int low,int high)  //��������Ϣ������һ������
{
	if(low>high)
	{
		error(19); //"�������´�С��ϵ�����"
	}
	if (AX==MAXNUMOFARRAYTABLE)
	{
		error(24);  //��������
		printf("TOO LONG ARRAYS IN PROGRAM!");
	}
	else
	{
		AX++;
		ATAB[AX].intType=type;
		ATAB[AX].low=low;
		ATAB[AX].high=high;
	}
}

void ENTERBLOCK()           //ÿ������һ�����̿�ʼʱ����֤���øú���һ��
{							//����������������һ�������ԵǼǴ˹�����
	if(BX==MAXNUMOFBLOCKTABLE)
	{
		error(26);  //����������
		printf("TOO MANY PROCEDURE IN PROGRAM!");
	}
	else
	{
		BX++;
		displayLevel++;  //���뵽��һ�����̣���μ�һ
		BTAB[BX].last=0;
		BTAB[BX].lastPar=0;
	}
}

void QUITBLOCK()        //ÿ��һ�����̱�����ʱ����֤���øú���һ��
{
	displayLevel--;     //��μ�һ
}


void ENTER(OBJECT kind) //�ڱ���Ĺ����н������ķ���������ű���
{						//�����ͬһ����������ζ������ʵĳ��֣��򱨴�
	int j,l;
	if(TX==MAXNUMOFNAMETABLE)
	{
		error(25);  //���ֱ����
		printf("PROGRAM TOO LONG!");
	}
	else
	{
		strcpy(NAMETAB[0].name,CurSymbol->value.lpValue);    //��ǰ��ʶ����ע�⣺ֻ���Ǳ�ʶ����������������ű��ĵ�һ�������������������
		j=BTAB[DISPLAY[displayLevel]].last;   //����ʱ����j��ɵ�ǰDISPLAY������ָ��ĳ���������last��
		l=j;
		while(strcasecmp(NAMETAB[j].name,CurSymbol->value.lpValue))  
			j=NAMETAB[j].link;               //˳������
		if(j>0)
			error(31); //��������ˣ��򱾳������ڷ����ظ�����
	    else
		{             //����������Ӧ����Ϣ
			TX++;
			strcpy(NAMETAB[TX].name,CurSymbol->value.lpValue);
			NAMETAB[TX].link=l;
			NAMETAB[TX].kind=kind;
			NAMETAB[TX].type=NOTYP;
			NAMETAB[TX].ref=0;
			NAMETAB[TX].level=displayLevel;
			NAMETAB[TX].normal=0;
			switch(kind)
			{
			case VARIABLE:
			case PROCEDURE:
				NAMETAB[TX].unite.address=0;break;
			case KONSTANT:
				NAMETAB[TX].unite.value=0;break;
			case TYPEL:
				NAMETAB[TX].unite.size=0;break;
			}
			BTAB[DISPLAY[displayLevel]].last=TX;
		}		
	}
}

int GETPOSITION(char * id)  //ͨ����ʶ�������������ֱ��������������
{
	int i=0,j=displayLevel;
	strcpy(NAMETAB[0].name,id);
	//j=displayLevel;

	do              //�����еĻ��¼��ע�⣺�ǻ�ģ�����˳������
	{
		i=BTAB[DISPLAY[j]].last;
		while (strcasecmp(NAMETAB[i].name,id))
			i=NAMETAB[i].link;
		j--;        //һ����¼����û������������һ������ȥ
	}while( j>=0 && i==0);   
	if(i==0)      // ��ʾû������
		error(33);  //û�ж������
	return (int)i;
}

void CONSTANT(SYMLIST * list,CONSTREC & constRec)  //��Դ���������ȡһ���������͵�����
{												   //�����ò���constRec ������¼��ȡ�Ľ��
	int x,sign;

	constRec.type=NOTYP;
	constRec.value=0;
	if(SYMINLIST(CurSymbol->type,&CONSTBEGSYS))
	{
		if(CurSymbol->type==CHARCON)
		{
			constRec.type=CHARS;
			constRec.value=CurSymbol->value.iValue;
			getASymbol();
		}
		else
		{
			sign=1;       //�������ķ���Ĭ����Ϊ��
			if(CurSymbol->type==PLUS || CurSymbol->type==MINUS)
			{
				if(CurSymbol->type==MINUS)
					sign=-1;  //��Ϊ��
				getASymbol();
			}

			if(CurSymbol->type==IDENT)
			{
				x=GETPOSITION(CurSymbol->value.lpValue);  //���
				if(x!=0)
				{
					if(NAMETAB[x].kind!=KONSTANT)
						error(17);// Ӧ���ǳ������߳�����ʶ��
					else
					{
						constRec.type=NAMETAB[x].type;
						constRec.value=sign*NAMETAB[x].unite.value;
					}
				}
				getASymbol();
			}
			else if(CurSymbol->type==INTCON)
			{
				constRec.type=INTS;
				constRec.value=sign*CurSymbol->value.iValue;
				getASymbol();
			}
		}
	}
}

void ARRAYTYP(SYMLIST * list,int & arrayRef,int & arraySize)  //��ȡһ���������͵���Ϣ�������ò���arrayRef��arraySize
{																  //������¼�������ȡ��������Ϣ�ʹ�С��Ϣ	
	TYPES eleType;
	CONSTREC low,high;
	int eleRef,eleSize;

	//////////////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	tempList->AddHead(COLON);	tempList->AddHead(RBRACK);
	tempList->AddHead(RPAREN);	tempList->AddHead(OFSYM);
	CONSTANT(tempList,low);         //��������±������
	delete tempList;
	//////////////////////////////////////////////////////////

	if(low.type!=INTS && low.type!=CHARS)
		error(20);  //�±�Ԫ�����ʹ���
    if(CurSymbol->type==DPOINT)
		getASymbol();
	else
		error(42);//Ӧ����'..'

	//////////////////////////////////////////////////////////
    SYMLIST * tempList1=new SYMLIST;
	tempList1->AddHead(COMMA);	tempList1->AddHead(RBRACK);
	tempList1->AddHead(RPAREN);	tempList1->AddHead(OFSYM);
	CONSTANT(tempList1,high);        //��������±������
	delete tempList1;
	//////////////////////////////////////////////////////////

	if(low.type!=high.type)
	{
		error(21);  //���½�����Ͳ�һ��
		high.value=low.value;
	}

	ENTERARRAY(low.type,low.value,high.value);   //��������Ϣ���Ǽ�

	arrayRef=AX;
	if(CurSymbol->type==COMMA)   //�����ǰ�����Ƕ��ţ���ʾ�����Ƕ�ά��
	{
		getASymbol();
		eleType=ARRAYS;    //����ġ���ά��������������
		ARRAYTYP(list,eleRef,eleSize);   //��ȡ�ϵ�ά���������Ϣ
	}
	else
	{
		if(CurSymbol->type==RBRACK)
			getASymbol();
		else
		{
			error(5);  //Ӧ����']'
			if(CurSymbol->type==RPAREN)
				getASymbol();
		}

		if(CurSymbol->type==OFSYM)
			getASymbol();
		else
			error(10);  //Ӧ����'of'
		TYP(list,eleType,eleRef,eleSize);   //��ȡ����Ԫ�ص�����
	}
	arraySize=(high.value-low.value+1)*eleSize;   //���㱾�������͵Ĵ�С
	ATAB[arrayRef].size=arraySize;
	ATAB[arrayRef].eleType=eleType;          //����Ϣ
	ATAB[arrayRef].elRef=eleRef;
	ATAB[arrayRef].elSize=eleSize;
}


void TYP(SYMLIST * list,TYPES & type,int & ref,int & size)//��ȡ�������ֱ�������ң���ǰ�ı�ʶ����������Ϣ
{	
	int x;		

	type=NOTYP;
	ref=0;
	size=0;
	if(SYMINLIST(CurSymbol->type,&TYPEBEGSYS))
	{
		if(CurSymbol->type==IDENT)
		{
			x=GETPOSITION(CurSymbol->value.lpValue);   //���
			if(x!=0)
			{
				if(NAMETAB[x].kind!=TYPEL)
				{
					error(15);  //Ӧ�������ͱ�ʶ��
				}
				else
				{
					type=NAMETAB[x].type;
					ref=NAMETAB[x].ref;
					size=NAMETAB[x].unite.size;
					if(type==NOTYP)
						error(36);  //���Ͷ������
				}
				getASymbol();
			}
		}
		else if (CurSymbol->type==ARRAYSYM)  //�������������
		{
			getASymbol();
			if(CurSymbol->type==LBRACK)
				getASymbol();
			else
			{
				error(4);  //Ӧ����'['
				if(CurSymbol->type==LPAREN)
					getASymbol();
			}
			type=ARRAYS;
			ARRAYTYP(list,ref,size);  //�����������͵���Ϣ
		}
	}
}


void PARAMENTERLIST(SYMLIST * list)  //����ĳ�����̵Ĳ����б�
{
	TYPES type;	
	int ref,size,x,helper;

	type=NOTYP;
	ref=0;
	size=0;

	getASymbol();	
	while(CurSymbol->type==IDENT || CurSymbol->type==VARSYM)
	{
		int valuePar=0;  //Ĭ���Ǳ�������
		if(CurSymbol->type!=VARSYM)
			valuePar=1;  //�����ǰ���Ų���var�������ֵ����
		else
			getASymbol();
			
		helper=TX;
		if(CurSymbol->type==IDENT)
		{
			ENTER(VARIABLE);  //���뵽һ��������������Ϣ������ű�
			getASymbol();
		}
		else 
			error(14);  //Ӧ���Ǳ�ʶ��

		while(CurSymbol->type==COMMA)  //�����ǰ�����Ƕ��ţ���˵����һ�������б�������������һ����ʶ��
		{
			getASymbol();
			if(CurSymbol->type==IDENT)
			{
				ENTER(VARIABLE);
				getASymbol();
			}
			else
				error(14);  //Ӧ���Ǳ�ʶ��
		}

		if(CurSymbol->type==COLON)  //�����':'�������Ӧ�������ͱ�ʶ����������������
		{
			getASymbol();
			if(CurSymbol->type!=IDENT)
				error(14);  //Ӧ���Ǳ�ʶ��
			else
			{
				x=GETPOSITION(CurSymbol->value.lpValue);
				getASymbol();
				if(x!=0)
				{
					if(NAMETAB[x].kind!=TYPEL)
						error(15);  //Ӧ�������ͱ�ʶ��
					else
					{
						type=NAMETAB[x].type;
						ref=NAMETAB[x].ref;
						if(valuePar)
							size=NAMETAB[x].unite.size;
						else
							size=1;
					}
				}
			}
		}
		else
			error(0);  //Ӧ����':'

		while(helper<TX)  //������ʶ������ı�������Ϣ������ű�
		{
			helper++;
			NAMETAB[helper].type=type;
			NAMETAB[helper].ref=ref;
			NAMETAB[helper].unite.address=DX;
			NAMETAB[helper].level=displayLevel;
			NAMETAB[helper].normal=valuePar;
			DX+=size;   //DX������¼�Ѿ�����ľֲ��ռ�Ĵ�С
		}

		if(CurSymbol->type!=RPAREN)
		{
			if(CurSymbol->type==SEMICOLON)
				getASymbol();
			else
			{	
				error(1);  //Ӧ����';'
				if(CurSymbol->type==COMMA)
					getASymbol();
			}
		}
	}
	
	if(CurSymbol->type==RPAREN)
	{
		getASymbol();
	}
	else
		error(2);  //Ӧ����')'
}

void CONSTDECLARATION(SYMLIST * list)  //��������
{
	CONSTREC constRec;

	if(CurSymbol->type==IDENT)
	{
		ENTER(KONSTANT);   //�������ĳ�����ʶ��
		getASymbol();
		if(CurSymbol->type==EQL)
			getASymbol();
		else
		{
			error(6);  //Ӧ����'='
			if(CurSymbol->type==BECOMES)
				getASymbol();
		}

		////////////////////////////////////////////////////////////////
		SYMLIST * tempList1=new SYMLIST;
		COPYLIST(tempList1,listAddSym(listAddSym(listAddSym(list,SEMICOLON),COMMA),IDENT));
		CONSTANT(tempList1,constRec);   //��ȡ��һ����������Ϣ
		delete tempList1;
		////////////////////////////////////////////////////////////////

		NAMETAB[TX].type=constRec.type;    //���
		NAMETAB[TX].ref=0;
		NAMETAB[TX].unite.value=constRec.value;
		if(CurSymbol->type==SEMICOLON)
			getASymbol();
		else
			error(1);//Ӧ����';'
	}
	else
		error(14);//Ӧ���Ǳ�ʶ��
}

void TYPEDECLARATION(SYMLIST * list)  //��������
{
	TYPES type;
	int ref,size,helper;

	if(CurSymbol->type==IDENT)
	{
		ENTER(TYPEL);  //�����������ͱ�ʶ��
		helper=TX;	
		getASymbol();

		if(CurSymbol->type==EQL)
			getASymbol();
		else
		{
			error(6);//Ӧ����'='
			if(CurSymbol->type==SEMICOLON)
				getASymbol();
		}

		//////////////////////////////////////////////////////////////
		SYMLIST * tempList=new SYMLIST;
		COPYLIST(tempList,listAddSym(listAddSym(listAddSym(list,SEMICOLON),COMMA),IDENT));
		TYP(tempList,type,ref,size);    //��ȡ������Ϣ
		delete tempList;
		//////////////////////////////////////////////////////////////

		NAMETAB[TX].type=type;         //���
		NAMETAB[TX].ref=ref;
		NAMETAB[TX].unite.size=size;

		if(CurSymbol->type==SEMICOLON)
			getASymbol();
		else
			error(1);//Ӧ����';'
	}
	else
		error(14);//Ӧ���Ǳ�ʶ��
}

void VARDECLARATION(SYMLIST * list)   //��������
{
	TYPES type;
	int ref,size,helper1,helper2;

	if(CurSymbol->type==IDENT)
	{
		helper1=TX;
		ENTER(VARIABLE);   //�������ı���
		getASymbol();
		while(CurSymbol->type==COMMA)  //����Ƕ��ţ������ٵ���ͬ���͵�һ�������б�
		{
			getASymbol();
			if(CurSymbol->type==IDENT)
			{
				ENTER(VARIABLE);  //�������ı���
				getASymbol();
			}
			else
				error(14);//Ӧ���Ǳ�ʶ��
		}

		if(CurSymbol->type==COLON)
			getASymbol();
		else
			error(0);//Ӧ����':'

		helper2=TX;

	    //////////////////////////////////////////////////////////
		SYMLIST * tempList=new SYMLIST;
		COPYLIST(tempList,listAddSym(listAddSym(listAddSym(list,SEMICOLON),COMMA),IDENT));
		TYP(tempList,type,ref,size);  //��ȡ������������Ϣ
		delete tempList;
		//////////////////////////////////////////////////////////

		while(helper1<helper2)  //��һ����������Ϣ���
		{
			helper1++;
			NAMETAB[helper1].type=type;
			NAMETAB[helper1].ref=ref;
			NAMETAB[helper1].level=displayLevel;
			NAMETAB[helper1].unite.address=DX;
			NAMETAB[helper1].normal=1;
			DX+=size;
		}

		if(CurSymbol->type==SEMICOLON)
			getASymbol();
		else
			error(1);//Ӧ����';'
	}
	else
		error(14);//Ӧ���Ǳ�ʶ��
}

void PROCDECLARATION(SYMLIST * list)  //��������
{
	getASymbol();
	if(CurSymbol->type!=IDENT)
	{
		error(14);  //Ӧ���Ǳ�ʶ��
		strcpy(CurSymbol->value.lpValue,"");
	}

	ENTER(PROCEDURE);  //�������Ĺ�����
	NAMETAB[TX].normal=1;
	getASymbol();

	///////////////////////////////////////////////////////////
	SYMLIST * tempList1=new SYMLIST;
	COPYLIST(tempList1,listAddSym(list,SEMICOLON));
	BLOCK(tempList1,displayLevel);   //���������
	delete tempList1;
	///////////////////////////////////////////////////////////

	if(CurSymbol->type==SEMICOLON)
		getASymbol();
	else
		error(1);//Ӧ����';'
}

void error(int errCode)   //������ʾ������Ϣ�ĺ���
{
	char errorScript[][100]=
	{
		"Ӧ����\':\'",//0
        "Ӧ����\';\'",//1
		"Ӧ����\')\'",//2
		"Ӧ����\'(\'",//3
		"Ӧ����\'[\'",//4
		"Ӧ����\']\'",//5
		"Ӧ����\'=\'",//6
		"Ӧ����\':=\'",//7
		"Ӧ����\'.\'",//8
		"Ӧ����\'do\'",//9
		"Ӧ����\'of\'",//10
		"Ӧ����\'then\'",//11
		"Ӧ����\'end\'",//12
		"Ӧ����\'program\'",//13
		"Ӧ���Ǳ�ʶ��",//14
		"Ӧ�������ͱ�ʶ��",//15
		"Ӧ���Ǳ���",//16
		"Ӧ���ǳ���������ʶ��",//17
		"Ӧ���ǹ�����",//18
		"�������½��С��ϵ����",//19
		"���鶨��ʱ���±�Ԫ�����ʹ���",//20
		"���鶨��ʱ�����½����Ͳ�һ��",//21
		"����ʱ������Ԫ���±�Ԫ�����ͳ���",//22
		"�±��������ȷ",//23
		"��������",//24
		"���ֱ����",//25
		"����������",//26
		"ϵͳΪ������������ĶѲ�����",//27
		"ʵ�����βθ�������",//28
		"ʵ�����β����Ͳ�һ��",//29
		"ʵ�θ�������",//30
		"�������ڷ����ض���",//31
		"if��while�������ʽ����Ϊ��������",//32
		"��ʶ��û�ж���",//33
		"�����������������ܳ����ڱ���ʽ��",//34
		"���������ʹ���",//35
		"���Ͷ������",//36
		"���Ͳ�һ��",//37
		"����ʶ���ַ�������Դ����",//38
		"���ܴ�.lst�ļ�",//39
		"���ܴ�.pld�ļ�",//40
		"���ܴ�.lab�ļ�",//41
		"Ӧ����\'..\'",//42
		"����ʱȱ�ٱ�ʶ��",//43
};

	if(CurSymbol && CurSymbol->lineNumber>0)
		printf("\n<<<<<<  Line number %d ",CurSymbol->lineNumber);
	printf("found error %d : %s !  >>>>>>\n\n",errCode,errorScript[errCode]);
	nError++;
	
}


void FACTOR(SYMLIST * list,TYPEITEM & typeItem)  //��ȡ��һ�������ӡ�����Ϣ�������ò���typeItem����������Ϣ
{
	int i;

	typeItem.typ=NOTYP;
	typeItem.ref=0;

	////////////////////////////////////////////////////////
	SYMLIST * tempList2=new SYMLIST;
	COPYLIST(tempList2,listAddSym(list,RPAREN));	
	////////////////////////////////////////////////////////

	while(SYMINLIST(CurSymbol->type,&FACBEGSYS))
	{
		switch(CurSymbol->type)
		{
		case IDENT:
			i=GETPOSITION(CurSymbol->value.lpValue);  //����Ǳ�ʶ��������
			getASymbol();
			if(i!=0)
			{
				switch(NAMETAB[i].kind)
				{
				case KONSTANT:
					typeItem.typ=NAMETAB[i].type;
					typeItem.ref=0;
					GEN(LIT,0,NAMETAB[i].unite.value);
					break;
				case VARIABLE:
					typeItem.typ=NAMETAB[i].type;
					typeItem.ref=NAMETAB[i].ref;
					if(NAMETAB[i].type==INTS || NAMETAB[i].type==BOOLS || NAMETAB[i].type==CHARS)
					{
						if(NAMETAB[i].normal)
							GEN(LOD,NAMETAB[i].level,NAMETAB[i].unite.address);
						else
							GEN(ILOD,NAMETAB[i].level,NAMETAB[i].unite.address);
					}
					else
					{
						if(NAMETAB[i].type==ARRAYS)
						{
							if(NAMETAB[i].normal)
								GEN(LODA,NAMETAB[i].level,NAMETAB[i].unite.address);
							else
								GEN(LOD,NAMETAB[i].level,NAMETAB[i].unite.address);
							if(CurSymbol->type==LBRACK)
								ARRAYELEMENT(list,typeItem);
							if(typeItem.typ!=ARRAYS)
								GEN(LODT,0,0);
						}
					}
					break;
				case PROCEDURE:
				case TYPEL:
					error(34);  //�����������������ܳ����ڱ���ʼ��
					break;
				}
			}
			break;

		case INTCON:
		case CHARCON:
			if(CurSymbol->type==INTCON)
				typeItem.typ=INTS;
			else
				typeItem.typ=CHARS;
			typeItem.ref=0;
			GEN(LIT,0,CurSymbol->value.iValue);
			getASymbol();
			break;
		case LPAREN:  //�����������
			getASymbol();
			EXPRESSION(tempList2,typeItem);  //��ȡһ��������ʽ������Ϣ
			if(CurSymbol->type==RPAREN)
				getASymbol();
			else 
				error(2); //Ӧ����')'
			break;
		case NOTSYM:   //����ǡ��ǡ�
			getASymbol();
			FACTOR(list,typeItem);   //��ȡһ�������ӡ�����Ϣ
			if(typeItem.typ==BOOLS)
				GEN(NOTS,0,0);
			else
				error(35);  //���������ʹ���
			break;
		}

	}
	delete tempList2;
}


void TERM(SYMLIST * list,TYPEITEM & typeItem)  //��ȡһ���������Ϣ
{
	SYMBOL mulop;
	TYPEITEM helpTypeItem;

	///////////////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	SYMLIST * tempList1=new SYMLIST;
	tempList1->AddHead(TIMES);	tempList1->AddHead(DIVSYM);	tempList1->AddHead(MODSYM);	tempList1->AddHead(ANDSYM);
	COPYLIST(tempList,listsAdd(tempList1,list));
	FACTOR(tempList,typeItem);    //��ȡһ�������ӡ�����Ϣ
	///////////////////////////////////////////////////////////

	while(SYMINLIST(CurSymbol->type,tempList1))
	{
		mulop=CurSymbol->type;  
		getASymbol();
		FACTOR(tempList,helpTypeItem);   //��ȡһ�������ӡ�����Ϣ
		if(typeItem.typ!=helpTypeItem.typ)
		{
			error(37);//���Ͳ�һ��
			typeItem.typ=NOTYP;
			typeItem.ref=0;
		}
		else
		{
			switch(mulop)
			{
			case TIMES:
				if(typeItem.typ==INTS)
					GEN(MULT,0,0);
				else
					error(35);//���������ʹ���
				break;
			case DIVSYM:
				if(typeItem.typ==INTS)
					GEN(IDIV,0,0);
				else
					error(35);//���������ʹ���
				break;
			case MODSYM:
				if(typeItem.typ==INTS)
					GEN(IMOD,0,0);
				else
					error(35);//���������ʹ���
				break;
			case ANDSYM:
				if(typeItem.typ==INTS)
					GEN(ANDS,0,0);
				else
					error(35);//���������ʹ���
				break;
			}
		}
	}
	delete tempList;delete tempList1;
}

void SIMPLEEXPRESSION(SYMLIST * list,TYPEITEM & typeItem)  //��ȡһ�����򵥱���ʽ������Ϣ
{
	SYMBOL addop;
	TYPEITEM helpTypeItem;

	/////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	SYMLIST * tempList1=new SYMLIST;
	tempList->AddHead(PLUS);	tempList->AddHead(MINUS);	tempList->AddHead(ORSYM);
	COPYLIST(tempList1,listsAdd(tempList,list));
	/////////////////////////////////////////////////

	if(CurSymbol->type==PLUS || CurSymbol->type==MINUS)
	{
		addop=CurSymbol->type;
		getASymbol();
		TERM(tempList1,typeItem);  //��ȡһ���������Ϣ
		if(addop==MINUS)
			GEN(MUS,0,0);
	}
	else
		TERM(tempList1,typeItem);   //��ȡһ���������Ϣ

	while(SYMINLIST(CurSymbol->type,tempList))
	{
		addop=CurSymbol->type;
		getASymbol();
		TERM(tempList1,helpTypeItem);   //��ȡһ���������Ϣ
		if(typeItem.typ!=helpTypeItem.typ)  //������������Ϣ������ͻ
		{
			error(37);//���Ͳ�һ��
			typeItem.typ=NOTYP;
			typeItem.ref=0;
		}
		else
		{
			switch(addop)
			{
			case PLUS:
				if(typeItem.typ==INTS)
					GEN(ADD,0,0);
				else
					error(35);//���������ʹ���
				break;
			case MINUS:
				if(typeItem.typ==INTS)
					GEN(SUB,0,0);
				else
					error(35);//���������ʹ���
				break;
			case ORSYM:
				if(typeItem.typ==BOOLS)
					GEN(ORS,0,0);
				else
					error(35);//���������ʹ���
				break;
			}
		}
	}
	delete tempList;delete tempList1;
}

void EXPRESSION(SYMLIST * list,TYPEITEM & typeItem)   //��ȡһ��������ʽ������Ϣ
{
	SYMBOL relationop;
	TYPEITEM helpTypeItem;

	/////////////////////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	SYMLIST * tempList1=new SYMLIST;
	tempList->AddHead(EQL);	tempList->AddHead(NEQ);	tempList->AddHead(LSS);
	tempList->AddHead(GTR);	tempList->AddHead(LEQ);	tempList->AddHead(GEQ);
	COPYLIST(tempList1,listsAdd(tempList,list));
	SIMPLEEXPRESSION(tempList1,typeItem);  //��ȡһ�����򵥱���ʽ������Ϣ
	/////////////////////////////////////////////////////////////////

	while(SYMINLIST(CurSymbol->type,tempList))
	{
		relationop=CurSymbol->type;
		getASymbol();
		SIMPLEEXPRESSION(list,helpTypeItem);   ///��ȡһ�����򵥱���ʽ������Ϣ
		if(typeItem.typ!=helpTypeItem.typ)
			error(37);//���Ͳ�һ��
		else
		{
			switch(relationop)
			{
			case EQL: GEN(EQ,0,0);break;
			case NEQ: GEN(NE,0,0);break;
			case LSS: GEN(LS,0,0);break;
			case GEQ: GEN(GE,0,0);break;
			case GTR: GEN(GT,0,0);break;
			case LEQ: GEN(LE,0,0);break;
			}
		}
		typeItem.typ=BOOLS;
	}
	delete tempList;delete tempList1;
}



void ARRAYELEMENT(SYMLIST * list,TYPEITEM & typeItem)  //��ȡһ������Ԫ�ص���Ϣ
{
	int p;
	TYPEITEM helpTypeItem;

	p=typeItem.ref;
	if(CurSymbol->type==LBRACK)
	{
		SYMLIST * tempList=new SYMLIST;
		COPYLIST(tempList,listAddSym(list,COMMA));
		do
		{
			getASymbol();
			EXPRESSION(tempList,helpTypeItem);   //��������Ԫ��ʱ��ʹ�õ������±����Ϣ
			if(typeItem.typ!=ARRAYS)
				error(23);  //�±��������ȷ
			else
			{
				if(helpTypeItem.typ!=ATAB[p].intType)
					error(22);  //����Ԫ������ʱ���±�Ԫ�����ͳ���
				GEN(LIT,0,ATAB[p].low);    //�����ǲ����ļ�������Ԫ�ص�ַ�Ĵ���
				GEN(SUB,0,0);
				GEN(LIT1,0,ATAB[p].elSize);
				GEN(MULT,0,0);
				GEN(ADD1,0,0);
				typeItem.typ=ATAB[p].eleType;
				typeItem.ref=ATAB[p].elRef;
				p=ATAB[p].elRef;
			}
		}
		while(CurSymbol->type==COMMA);  //��ά����

		if(CurSymbol->type==RBRACK)
			getASymbol();
		else
			error(5);//Ӧ����']'
	}
	else
		error(4);//Ӧ����'['	
}


void INITIAL()  //��������ʼ��������һЩ����
{
	nError=0;
	displayLevel=0;
	DISPLAY[0]=0;

	DX=0;
	CX=0;
	BX=1;
	TX=-1;
	JX=0;

	// FIVE SYMLISTS' INITIALIZATION

	DECLBEGSYS.AddHead(CONSTSYM);	DECLBEGSYS.AddHead(VARSYM);
	DECLBEGSYS.AddHead(TYPESYM);	DECLBEGSYS.AddHead(PROCSYM);

	STATBEGSYS.AddHead(BEGINSYM);	STATBEGSYS.AddHead(CALLSYM);
	STATBEGSYS.AddHead(IFSYM);	STATBEGSYS.AddHead(WHILESYM);

	FACBEGSYS.AddHead(IDENT);	FACBEGSYS.AddHead(INTCON);	FACBEGSYS.AddHead(LPAREN);
	FACBEGSYS.AddHead(NOTSYM);	FACBEGSYS.AddHead(CHARCON);

	TYPEBEGSYS.AddHead(IDENT);	TYPEBEGSYS.AddHead(ARRAYSYM);

	CONSTBEGSYS.AddHead(PLUS);	CONSTBEGSYS.AddHead(MINUS);	CONSTBEGSYS.AddHead(INTCON);
	CONSTBEGSYS.AddHead(CHARCON);	CONSTBEGSYS.AddHead(IDENT);
}


void ENTERID(char * name,OBJECT kind,TYPES type,int value)  //����ű���������ϢҪ�õĺ���
{
	TX++;
	strcpy(NAMETAB[TX].name,name);
	NAMETAB[TX].link=TX-1;
	NAMETAB[TX].kind=kind;
	NAMETAB[TX].type=type;
	NAMETAB[TX].ref=0;
	NAMETAB[TX].normal=1;
	NAMETAB[TX].level=0;
	switch(kind)
	{
	case VARIABLE:
	case PROCEDURE:
		NAMETAB[TX].unite.address=value;break;
	case KONSTANT:
		NAMETAB[TX].unite.value=value;break;
	case TYPEL:
		NAMETAB[TX].unite.size=value;break;
	}
	return;
}

void ENTERPREID()  //Ԥ��������ű�����Ϣ������Щ��Ϣ��Ϊ�������㡱���������
{
	ENTERID("",VARIABLE,NOTYP,0);
	ENTERID("char",TYPEL,CHARS,1);
	ENTERID("integer",TYPEL,INTS,1);
	ENTERID("boolean",TYPEL,BOOLS,1);
	ENTERID("false",KONSTANT,BOOLS,0);
	ENTERID("true",KONSTANT,BOOLS,1);
	ENTERID("read",PROCEDURE,NOTYP,1);
	ENTERID("write",PROCEDURE,NOTYP,2);

	BTAB[0].last=TX;
	BTAB[0].lastPar=1;
	BTAB[0].pSize=0;
	BTAB[0].vSize=0;
}

void ASSIGNMENT(SYMLIST * list)  //������ֵ���
{
	TYPEITEM typeItem1,typeItem2;
	int i;

	i=GETPOSITION(CurSymbol->value.lpValue);
	if(i==0)
		error(33);  //��ʶ��û�ж���
	else
	{
		if (NAMETAB[i].kind!=VARIABLE)  //':='���Ӧ���Ǳ���
		{
			error(16);  //Ӧ���Ǳ���
            i=0;
		}
        getASymbol();
        typeItem1.typ=NAMETAB[i].type;
        typeItem1.ref=NAMETAB[i].ref;
        if(NAMETAB[i].normal)
			GEN(LODA,NAMETAB[i].level,NAMETAB[i].unite.address);
		else
			GEN(LOD,NAMETAB[i].level,NAMETAB[i].unite.address);

		if(CurSymbol->type==LBRACK)  //�����ǰ��ʶ����������ţ���ô����Ϊһ�������Ԫ�ظ�ֵ
		{
			//////////////////////////////////////////////////////////
			SYMLIST * tempList=new SYMLIST;
			COPYLIST(tempList,listAddSym(list,BECOMES));
            ARRAYELEMENT(tempList,typeItem1);  //��ô�����Ԫ�ص���Ϣ
			delete tempList;
            //////////////////////////////////////////////////////////
		}
			
		if(CurSymbol->type==BECOMES)
			getASymbol();
		else
		{
			error(7);  //Ӧ����':='
            if(CurSymbol->type==EQL)
				getASymbol();
		}
		EXPRESSION(list,typeItem2);  //':='�ұ��Ǳ���ʽ
		if(typeItem1.typ!=typeItem2.typ)
			error(37);//���Ͳ�һ��
		else
		{
			if(typeItem1.typ==ARRAYS)
			{
				if(typeItem1.ref==typeItem2.ref)
					GEN(CPYB,0,ATAB[typeItem1.ref].size);
				else
					error(35);//���������ͳ���
			}
			else
				GEN(STO,0,0);
		}
	}
}


void IFSTATEMENT(SYMLIST * list)  //����if���
{
	TYPEITEM typeItem;
	int fillBackFalse,fillBackTrue;

	getASymbol();
	
	/////////////////////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	COPYLIST(tempList,listAddSym(listAddSym(list,DOSYM),THENSYM));
	EXPRESSION(tempList,typeItem);  //��ȡ��������ʽ����Ϣ
	delete tempList;
	/////////////////////////////////////////////////////////////////

	if(typeItem.typ!=BOOLS)
		error(32);  //if��while����ı���ʽ���ͱ���Ϊ��������
	if(CurSymbol->type==THENSYM)
		getASymbol();
	else 
		error(11);  //Ӧ����'then'

	fillBackFalse=CX;  //����
	GEN(JPC,0,0);

	/////////////////////////////////////////////////////////////////
	SYMLIST * tempList1=new SYMLIST;
	COPYLIST(tempList1,listAddSym(list,ELSESYM));
	STATEMENT(tempList1);  //������
	delete tempList1;
	/////////////////////////////////////////////////////////////////

	if(CurSymbol->type==ELSESYM)
	{
		getASymbol();
		fillBackTrue=CX;  //����
		GEN(JMP,0,0);
		CODE[fillBackFalse].address=CX;
		JUMADRTAB[JX]=CX;
		JX++;
		STATEMENT(list);
		CODE[fillBackTrue].address=CX;
		JUMADRTAB[JX]=CX;
		JX++;
	}
	else
	{
		CODE[fillBackFalse].address=CX;
		JUMADRTAB[JX]=CX;
		JX++;
	}
}


void WHILESTATEMENT(SYMLIST * list)  //while ���ķ���
{
	TYPEITEM typeItem;
	int jumpback,fillBackFalse;

	getASymbol();
	JUMADRTAB[JX]=CX;
	JX++;
	jumpback=CX;  //��¼whileѭ��ִ��ʱ��Ҫ��ǰ���صĵ�ַ

	/////////////////////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	COPYLIST(tempList,listAddSym(list,DOSYM));
	EXPRESSION(tempList,typeItem);
	delete tempList;
	/////////////////////////////////////////////////////////////////

	if(typeItem.typ!=BOOLS)
		error(32);  //if��while����ı���ʽ����Ӧ���ǲ�������
	fillBackFalse=CX;  //��������ʧ�ܺ��������ѭ�����⡱��
	GEN(JPC,0,0);
	if(CurSymbol->type==DOSYM)
		getASymbol();
	else
		error(9);//Ӧ����'do'

	STATEMENT(list);  //�������
	GEN(JMP,0,jumpback);
	CODE[fillBackFalse].address=CX;  //����
	JUMADRTAB[JX]=CX;
	JX++;
}

void COMPOUND(SYMLIST * list)  //begin ��ͷ��������ķ���
{
	getASymbol();

    ////////////////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	COPYLIST(tempList,listAddSym(listAddSym(list,ENDSYM),SEMICOLON));
	STATEMENT(tempList);
	
	////////////////////////////////////////////////////////////
	SYMLIST * tempList1=new SYMLIST;
	COPYLIST(tempList1,listAddSym(&STATBEGSYS,SEMICOLON));
	///////////////////////////////////////////////////////////

	while(SYMINLIST(CurSymbol->type,tempList1))  //һ�����ķ������
	{
		if(CurSymbol->type==SEMICOLON)
			getASymbol();
		else
			error(1);//Ӧ����';'
	    STATEMENT(tempList);  
	}
	if(CurSymbol->type==ENDSYM)
		getASymbol();
	else
		error(12);  //Ӧ����'end'
	delete tempList;delete tempList1;
}

void STANDPROC(SYMLIST * list,int i)  //������������read��write�ĵ��ã������ڷ��ű�����ֱ��ڵ�����͵�����
{
	int helper;
	TYPEITEM typeItem;

	if(i==6)//����ǵ����read();
	{
		getASymbol();
		if(CurSymbol->type==LPAREN)
		{
			do
			{
				getASymbol();
				if(CurSymbol->type==IDENT)
				{
					helper=GETPOSITION(CurSymbol->value.lpValue);
					getASymbol();
					if(helper==0)
					{
						error(33);//��ʶ��û�ж���
					}
					else
					{
						if(NAMETAB[helper].kind!=VARIABLE)
						{
							error(16);//Ӧ���Ǳ���
							helper=0;
						}
						else
						{
							typeItem.typ=NAMETAB[helper].type;
							typeItem.ref=NAMETAB[helper].ref;

							if(NAMETAB[helper].normal)
								GEN(LODA,NAMETAB[helper].level,NAMETAB[helper].unite.address);
							else
								GEN(LOD,NAMETAB[helper].level,NAMETAB[helper].unite.address);
							if(CurSymbol->type==LBRACK)
							{
								//////////////////////////////////////////////////////
								SYMLIST * tempList1=new SYMLIST;
								COPYLIST(tempList1,listAddSym(list,COMMA));
								ARRAYELEMENT(tempList1,typeItem);
								delete tempList1;
								//////////////////////////////////////////////////////
							}
							if(typeItem.typ==INTS)
								GEN(RED,0,0);
							else if(typeItem.typ==CHARS)
								GEN(RED,0,1);
							else
								error(35);//���������ʹ���
						}
					}
				}
				else
					error(14);//Ӧ���Ǳ�ʶ��
			}while(CurSymbol->type==COMMA);

			if(CurSymbol->type!=RPAREN)
			{	
				error(2);//Ӧ����')'
			}
			else 
				getASymbol();
		}
		else
			error(3);//Ӧ����'('
	}
	else if(i==7)//����ǵ����write();
	{
		getASymbol();
		if(CurSymbol->type==LPAREN)
		{
			do
			{
				getASymbol();
				///////////////////////////////////////////////////////
				SYMLIST * tempList=new SYMLIST;
				COPYLIST(tempList,listAddSym(listAddSym(list,RPAREN),COMMA));
				EXPRESSION(tempList,typeItem);
				delete tempList;
				////////////////////////////////////////////////////////
				
				if(typeItem.typ==INTS)
					GEN(WRT,0,0);
				else if(typeItem.typ==CHARS)
					GEN(WRT,0,1);
				else
					error(35);//���������ͳ���
			}
			while(CurSymbol->type==COMMA);
			
			if(CurSymbol->type!=RPAREN)
				error(2);//Ӧ����')'
			else
				getASymbol();
		}
		else
			error(3);//Ӧ����'('
	}
}

void CALL(SYMLIST * list)  //�������ķ���
{
	int i,lastPar,cp,k;
	TYPEITEM typeItem;

	/////////////////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	COPYLIST(tempList,listAddSym(listAddSym(list,RPAREN),COMMA));
    /////////////////////////////////////////////////////////////

	getASymbol();
	if(CurSymbol->type==IDENT)
	{
		i=GETPOSITION(CurSymbol->value.lpValue);
		if(NAMETAB[i].kind==PROCEDURE)
		{
			if(NAMETAB[i].level==0)  //����ڡ������㡱����ô�ض���read ���� write
				STANDPROC(list,i);
			else
			{
				getASymbol();
				GEN(OPAC,0,0);//�򿪻��¼
				lastPar=BTAB[NAMETAB[i].ref].lastPar;
				cp=i;
				if(CurSymbol->type==LPAREN)
				{//ʵ�ڲ����б�
					/////////////////////////////////////////////////////////
					SYMLIST * tempList1=new SYMLIST;
					COPYLIST(tempList1,listAddSym(listAddSym(listAddSym(list,RPAREN),COLON),COMMA));					
					/////////////////////////////////////////////////////////
					do
					{
						getASymbol();
						if(cp>=lastPar)
							error(28);//ʵ�����βθ�������
						else
						{
							cp++;
							if(NAMETAB[cp].normal)
							{//ֵ����

								EXPRESSION(tempList1,typeItem);
								if(typeItem.typ==NAMETAB[cp].type)
								{
									if(typeItem.ref!=NAMETAB[cp].ref)
										error(29);//ʵ�����β����Ͳ�һ��
									else if(typeItem.typ==ARRAYS)
										GEN(LODB,0,ATAB[typeItem.ref].size);
								}
								else
									error(29);//ʵ�����β����Ͳ�һ��
							}
							else
							{//��ʽ����
								if(CurSymbol->type!=IDENT)
									error(14);//Ӧ���Ǳ�ʶ��
								else
								{
									k=GETPOSITION(CurSymbol->value.lpValue);
									getASymbol();
									if(k!=0)
									{
										if(NAMETAB[k].kind!=VARIABLE)
											error(16);//Ӧ���Ǳ���
										typeItem.typ=NAMETAB[k].type;
										typeItem.ref=NAMETAB[k].ref;
										if(NAMETAB[k].normal)
											GEN(LODA,NAMETAB[k].level,NAMETAB[k].unite.address);
										else
											GEN(LOD,NAMETAB[k].level,NAMETAB[k].unite.address);
										if(CurSymbol->type==LBRACK)
										{   
											ARRAYELEMENT(tempList,typeItem);
										}
										if(NAMETAB[cp].type!=typeItem.typ || NAMETAB[cp].ref!=typeItem.ref)
											error(29);//ʵ�����β����Ͳ�һ��
									}
								}
							}
						}
					}
					while(CurSymbol->type==COMMA);
					delete tempList1;
					if(CurSymbol->type==RPAREN)
						getASymbol();
					else
						error(2);//Ӧ����')'
				}
				if(cp<lastPar)
					error(30);//ʵ�ڲ�����������
				GEN(CAL,NAMETAB[i].level,NAMETAB[i].unite.address);
				if(NAMETAB[i].level<displayLevel)
					GEN(UDIS,NAMETAB[i].level,displayLevel);
			}
		}
		else
			error(18);//Ӧ���ǹ�����
	}
	else
		error(14);//Ӧ���Ǳ�ʶ��
	delete tempList;
}

void STATEMENT(SYMLIST * list)  //��ͨ���ķ�������Щ����м�����ʽ���ֱ��Բ�ͬ�ı�ʶ����ͷ
{
	////////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	COPYLIST(tempList,listAddSym(&STATBEGSYS,IDENT));
	////////////////////////////////////////////////////

	if(SYMINLIST(CurSymbol->type,tempList))  //ͨ����ͬ�ı�ʶ��������������
	{
		switch(CurSymbol->type)
		{
		case IDENT: ASSIGNMENT(list);break;
		case CALLSYM: CALL(list);break;
		case IFSYM: IFSTATEMENT(list);break;
		case WHILESYM: WHILESTATEMENT(list);break;
		case BEGINSYM: COMPOUND(list);break;
		}
	}
	delete tempList;
}


void BLOCK(SYMLIST * list,int level)  //������ķ���
{
	int cx,tx,programBlock;
    int dx;
	dx=DX;//��¼��̬�ϲ�ֲ���������С
	DX=3;//����������Ļ��¼����������Ԫ������������������
	tx=TX;
	NAMETAB[tx].unite.address=CX;

	if(displayLevel>MAXLEVELDEPTH)
		error(26);//����������
	ENTERBLOCK();  //�Ǽǹ�����
	programBlock=BX;
	DISPLAY[displayLevel]=BX;
	NAMETAB[tx].type=NOTYP;
	NAMETAB[tx].ref=programBlock;

	if(CurSymbol->type==LPAREN && displayLevel>1)
	{
		PARAMENTERLIST(list);  //��������б�
		if(CurSymbol->type==SEMICOLON)
			getASymbol();
		else
			error(1);//Ӧ����';'
	}
	else if(displayLevel>1)
	{
		if(CurSymbol->type==SEMICOLON)
			getASymbol();
		else
			error(1);//Ӧ����';'
	}
	BTAB[programBlock].lastPar=TX;
	BTAB[programBlock].pSize=DX;
	GEN(JMP,0,0);
	do
	{
		switch(CurSymbol->type)  //��Ե�ǰ��ͬ�Ĳ��ý��в�ͬ������
		{			
		case CONSTSYM:
			getASymbol();
			do
			{
				CONSTDECLARATION(list);  //����������һ�ο��������
			}
			while(CurSymbol->type==IDENT);
			break;
		case TYPESYM:
			getASymbol();
			do
			{
				TYPEDECLARATION(list);  //����������һ�ο��������
			}
			while(CurSymbol->type==IDENT);
			break;
		case VARSYM:
			getASymbol();
			do
			{
				VARDECLARATION(list);  //����������һ�ο��������
			}
			while(CurSymbol->type==IDENT);				
			break;
		}
		while(CurSymbol->type==PROCSYM)
			PROCDECLARATION(list);    //����������ÿ��ֻ������һ��
	}while(SYMINLIST(CurSymbol->type,&DECLBEGSYS));
	CODE[NAMETAB[tx].unite.address].address=CX;//��ִ�����Ŀ�ʼ����ַ����
	JUMADRTAB[JX]=CX;
	JX++;
	NAMETAB[tx].unite.address=CX;//���뿪ʼ��ַ
	cx=CX;
	GEN(ENTP,displayLevel,DX);

	////////////////////////////////////////////////////
	SYMLIST * tempList=new SYMLIST;
	COPYLIST(tempList,listAddSym(listAddSym(list,ENDSYM),SEMICOLON));
	STATEMENT(tempList);
	delete tempList;
	////////////////////////////////////////////////////
    CODE[cx].address=DX;//������������С
	if(displayLevel>1)
		GEN(RETP,0,0);//�ӳ����巵��
	else
		GEN(ENDP,0,0);//�������
	QUITBLOCK();
	DX=dx;//�ָ���̬�ϲ�ֲ���������С
}

int Feof(FILE *fp)//�ж��Ƿ���Դ�ļ�β
{
	int getChar;
	getChar=fgetc(fp);
	if(getChar==-1)
	{
		if(feof(fp))
			return 1;//����ǣ����ء��桱
	}
	else
		fseek(fp,-1,SEEK_CUR);//���򣬽�ָ���ļ�����ָ������ƶ�һ���ַ�
	return 0;
}


SYMBOL GetReserveWord(char *nameValue)//�жϵõ��ķ����Ƿ��Ǳ�����
{
	int i;

	char reserveWord[NUMOFWORD][20]=
	{
		"and","begin","const","else","if","not","or","program","type","while",
		"array","call","do","end","mod","of","procedure","then","var"
	};
	SYMBOL reserveType[NUMOFWORD]=
	{
		ANDSYM,BEGINSYM,CONSTSYM,ELSESYM,IFSYM,NOTSYM,ORSYM,PROGRAMSYM,TYPESYM,WHILESYM,
		ARRAYSYM,CALLSYM,DOSYM,ENDSYM,MODSYM,OFSYM,PROCSYM,THENSYM,VARSYM
	};

	for(i=0;i<NUMOFWORD;i++)  //��������˱���������������Ч�ʲ��ߣ����Կ��ǲ��ö�����ҷ�
		if(!strcasecmp(reserveWord[i],nameValue))
			return reserveType[i];
	return (SYMBOL)0;
}

void AddSymbolNode(SymbolItem **current,int lineNumber,SYMBOL type,int iValue)  //�ڴʷ�������ʱ��������б��м���һ�����������ķ���
{
		(*current)->next=new SymbolItem;
		if(!(*current)->next)
		{
			error(27);//ϵͳΪ������������ĶѲ�����
			exit(4);
		}
		(*current)=(*current)->next;
		(*current)->lineNumber=lineNumber;
		(*current)->type=type;
		(*current)->value.iValue=iValue; 
		(*current)->next=NULL; 
}

void getSymbols(FILE *srcFile)  //��Դ�ļ������ַ�����÷�������
{
	int lineNumber=1;
	char nameValue[MAXSYMNAMESIZE];
	int nameValueint;
	char readChar;
	SymbolItem head,*current=&head;

	printf("\n���дʷ�����  -->-->-->-->-->-->-->-->  ");

	while(!Feof(srcFile))
	{
		readChar=fgetc(srcFile);

		if(iscsymf(readChar))
		{
			nameValueint=0;
			do
			{
				nameValue[nameValueint++]=readChar;
				readChar=fgetc(srcFile);
				if(Feof(srcFile)) 
					break;
			}while(iscsym(readChar) || isdigit(readChar)); 
			nameValue[nameValueint]=0;
			fseek(srcFile,-1,SEEK_CUR);
			current->next=new SymbolItem;
			current=current->next;
			current->lineNumber=lineNumber;
			if(!(current->type=GetReserveWord(nameValue)))
			{
				current->type=IDENT;
				current->value.lpValue=new char[nameValueint]; 
				strcpy(current->value.lpValue,nameValue);
			}
			current->next=NULL; 
		}
		else if(isdigit(readChar))
		{
			nameValueint=0;
			do
			{
				nameValue[nameValueint++]=readChar;
				readChar=fgetc(srcFile);
				if(Feof(srcFile)) 
					break;
			}while(isdigit(readChar)); 
			nameValue[nameValueint]=0;
			fseek(srcFile,-1,SEEK_CUR);
			AddSymbolNode(&current,lineNumber,INTCON,atoi(nameValue));
		}
		else switch(readChar)
		{
			case '	':				//字符 'tab'
			case ' ':
				break;
			case '\r':
				// 回车：CRLF 换行的前半，忽略，行号交给 '\n' 统一累加（兼容 LF/CRLF）。
				break;
			case '\n':
				lineNumber++;
				break;
			case ':':
				if(Feof(srcFile))
					break;
				readChar=fgetc(srcFile);
				if(readChar=='=')
					AddSymbolNode(&current,lineNumber,BECOMES,0);
				else
				{
					fseek(srcFile,-1,SEEK_CUR);
					AddSymbolNode(&current,lineNumber,COLON,0);
				}
				break;
			case '<':
				if(Feof(srcFile))
					break;
				readChar=fgetc(srcFile);
				if(readChar=='=')
					AddSymbolNode(&current,lineNumber,LEQ,0);
				else if(readChar=='>')
					AddSymbolNode(&current,lineNumber,NEQ,0);
				else
				{
					fseek(srcFile,-1,SEEK_CUR);
					AddSymbolNode(&current,lineNumber,LSS,0);
				}
				break;
			case '>':
				if(Feof(srcFile))
					break;
				readChar=fgetc(srcFile);
				if(readChar=='=')
					AddSymbolNode(&current,lineNumber,GEQ,0);
				else
				{
					fseek(srcFile,-1,SEEK_CUR);
					AddSymbolNode(&current,lineNumber,GTR,0);
				}
				break;
			case '.':
				if(Feof(srcFile))
				{
					AddSymbolNode(&current,lineNumber,PERIOD,0);
					break;
				}
				readChar=fgetc(srcFile);
				if(readChar=='.')
					AddSymbolNode(&current,lineNumber,DPOINT,0);
				else
				{
					fseek(srcFile,-1,SEEK_CUR);
					AddSymbolNode(&current,lineNumber,PERIOD,0);
				}
				break;
			case '\'':
				readChar=fgetc(srcFile);
				if(Feof(srcFile))
					break;
				if(fgetc(srcFile)=='\'')
					AddSymbolNode(&current,lineNumber,CHARCON,readChar);
				else
					error(1);//////////////////////////
				break;
			case '+':
				AddSymbolNode(&current,lineNumber,PLUS,0);
				break;
			case '-':
				AddSymbolNode(&current,lineNumber,MINUS,0);
				break;
			case '*':
				AddSymbolNode(&current,lineNumber,TIMES,0);
				break;
			case '/':
				AddSymbolNode(&current,lineNumber,DIVSYM,0);
				break;
			case '(':
				AddSymbolNode(&current,lineNumber,LPAREN,0);
				break;
			case ')':
				AddSymbolNode(&current,lineNumber,RPAREN,0);
				break;
			case '=':
				AddSymbolNode(&current,lineNumber,EQL,0);
				break;
			case '[':
				AddSymbolNode(&current,lineNumber,LBRACK,0);
				break;
			case ']':
				AddSymbolNode(&current,lineNumber,RBRACK,0);
				break;
			case ';':
				AddSymbolNode(&current,lineNumber,SEMICOLON,0);
				break;
			case ',':
				AddSymbolNode(&current,lineNumber,COMMA,0);
				break;
			default:
				error(38);
		}   //switch end
	}		//while end
	Symbols=head.next; 
	CurSymbol=Symbols;
	if(nError)
	{
		printf("\n%d errors found.",nError);
		exit(2);
	}
	else
		printf("�ʷ������ɹ���\n\n");
}

void getASymbol()  //���õݹ��½����﷨����������Ļ�ȡһ�����ʷ���
{
	if(CurSymbol->next)
		CurSymbol=CurSymbol->next;
	else
	{
		error(43);  //�﷨����û����ϣ���Ҫ��ʶ��
		exit(3);
	}

}

void destroySymbols()  //������ϣ������������ͷ�
{
	SymbolItem *current,*needDel;
	current=Symbols;
	while(current)
	{
		needDel=current;
		current=current->next;
		delete needDel;
	}
}


/////////////����������������Ϊ��ģ��pascalԴ�����е�set���Ͷ�������////////////

SYMLIST * listsAdd(SYMLIST * list1,SYMLIST * list2)  //���������ϡ���ӣ�����һ�������ϡ�
{
	SYMLIST * temp=new SYMLIST;
	COPYLIST(temp,list1);
	temp->AddTail(list2);
	return temp;
}
		
SYMLIST * listAddSym(SYMLIST * list,SYMBOL sym)  //һ�������ϡ�����һ����Ԫ�ء�������һ�������ϡ�
{
	SYMLIST * temp=new SYMLIST;
	COPYLIST(temp,list);
	temp->AddTail(sym);
	return temp;
}

int SYMINLIST(SYMBOL sym,SYMLIST * list)  //�ж�һ����Ԫ�ء��Ƿ��ڡ����ϡ�����
{
	for(POSITION pos=list->GetHeadPosition();pos;)
	{
		SYMBOL temp;
		temp=list->GetNext(pos);
		if(temp==sym)
			return 1;  //����ڣ����ط���
	}
	return 0;  //���ڣ��򷵻���
}

void COPYLIST(SYMLIST * list1,SYMLIST * list2)  //�����ϡ�֮��Ŀ���
{
	for(POSITION pos=list2->GetHeadPosition();pos;)
	{
		SYMBOL temp;
		temp=list2->GetNext(pos);
		list1->AddTail(temp);
	}
}


/////////////////////  ������  ///////////////////////
int main(int argc, char* argv[])  
{
	char srcFilename[FILENAMESIZE];
	FILE *srcFile;
	char *srcFileNamePoint;

	if(argc>1)
		strcpy(srcFilename,argv[1]);
	else
	{
		printf("Please input the source file name : ");
		scanf("%s",srcFilename);
	}
	if(!(srcFile=fopen(srcFilename,"rb")))
	{
		printf("Error : source file %s not found\n",srcFilename);
		exit(1);
	}

	printf("\n��һ�飺�ʷ�����");
	getSymbols(srcFile);//��һ�飬ȡ�����еķ��ţ��ڶ���ſ�ʼ�﷨�����ʹ�������

	printf("�ڶ��飺�﷨�����ʹ�������\n");
	INITIAL();  //��ʼ��
	ENTERPREID();  //Ԥ����ű�

	printf("\n**************   �����ǲ��ֵ����ɴ���   ***************\n\n");
	if (CurSymbol->type!=PROGRAMSYM)
		error(13);  //Ӧ����'program'
	getASymbol();
	if(CurSymbol->type!=IDENT)
		error(14);  //Ӧ���Ǳ�ʶ��
	getASymbol();
	if(CurSymbol->type!=SEMICOLON)
		error(1);  //Ӧ����';'
	else 
		getASymbol();

	//////////////////////////////////////////////////////////
	SYMLIST * tempList3=new SYMLIST;
	COPYLIST(tempList3,listsAdd(listAddSym(&DECLBEGSYS,PERIOD),&STATBEGSYS));
	BLOCK(tempList3,0);
	delete tempList3;
	//////////////////////////////////////////////////////////

	if(CurSymbol->type!=PERIOD)
		error(8);  //Ӧ����'.'
	if(nError==0)
	{
		for(srcFileNamePoint=&srcFilename[strlen(srcFilename)];*srcFileNamePoint!='.' && srcFileNamePoint!=srcFilename;srcFileNamePoint--)
		;
	    *srcFileNamePoint=0;  //ɾ���������չ��
		WriteCodeList(strcat(srcFilename,".lst"));
		for(srcFileNamePoint=&srcFilename[strlen(srcFilename)];*srcFileNamePoint!='.' && srcFileNamePoint!=srcFilename;srcFileNamePoint--)
			;
		*srcFileNamePoint=0;  //ɾ���������չ��
		WriteObjCode(strcat(srcFilename,".pld"));
		for(srcFileNamePoint=&srcFilename[strlen(srcFilename)];*srcFileNamePoint!='.' && srcFileNamePoint!=srcFilename;srcFileNamePoint--)
			;
		*srcFileNamePoint=0;  //ɾ���������չ��
		WriteLabelCode(strcat(srcFilename,".lab"));
		destroySymbols();
		fclose(srcFile);
		
		//printf("\n����ɹ����������κ��ַ��˳���");
		printf("\n����ɹ���");
		
		//int a;
		//scanf("%d",a);       //һ���򵥵Ĺؿ�������ִ����Ϻ����ͣ�£������Թۿ����ɵĴ��룬ע�⣬�����ǲ��ֵģ���Щ������û������ȥ
		return 0;
	}
	destroySymbols();
	
	//int b;
	//scanf("%d",b);     //һ���򵥵Ĺؿ�������ִ����Ϻ����ͣ�£������Թۿ����ɵĴ��룬ע�⣬�����ǲ��ֵģ���Щ������û������ȥ
	return 0;
}

