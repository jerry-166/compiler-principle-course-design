# include "common.h"

void INTERPRET()  //�����������
{
	//////////  ��ʼ������  /////////
	oldTop=0;     
	stop=0;
	top=0;
	bp=0;
	pc=0;
	DISPLAY[1]=0;
	S[1]=0;
	S[2]=0;
	S[3]=0;
	/////////  ��ʼ������  //////////

	printf("���뿪ʼ\n");
	do
	{
		instruction=CODE[pc];  //ȡָ��
		pc++;                  //PC��һ

		switch(instruction.func)  //����ִ��
		{
		case LIT:
		case LIT1:
			top++;
			S[top]=instruction.address;
			break;
		case LOD:
			top++;
			S[top]=S[DISPLAY[instruction.level]+instruction.address];
			break;
		case LODA:
			top++;
			S[top]=DISPLAY[instruction.level]+instruction.address;
			break;		
		case ILOD:
			top++;
			S[top]=S[S[DISPLAY[instruction.level]+instruction.address]];
			break;
		case LODT:
			S[top]=S[S[top]];
			break;
		case LODB:
			h=S[top];
			top--;
			hh=instruction.address+top;
			while(top<hh)
			{
				top++;
				S[top]=S[h];
				h++;
			}
			break;
		case CPYB:
			h=S[top-1];
			hh=S[top];
			hhh=h+instruction.address;
			while(h<hhh)
			{
				S[h]=S[hh];
				h++;
				hh++;
			}
			top-=2;
			break;
		case STO:
			S[S[top-1]]=S[top];
			top-=2;
			break;
		case OPAC:
			oldTop=top;
			top+=3;
			break;
		case CAL:
			S[oldTop+1]=pc;
			S[oldTop+2]=DISPLAY[instruction.level];
			S[oldTop+3]=bp;
			pc=instruction.address;
			break;
		case ENTP:
			bp=oldTop+1;
			DISPLAY[instruction.level]=bp;
			top=oldTop+instruction.address;
			break;
		case UDIS:
			h=instruction.address;
			hh=instruction.level;
			hhh=bp;
			do
			{
				DISPLAY[h]=hhh;
				h--;
				hhh=S[hhh+1];
			}while(h!=hh);
			break;
		case JMP:
			pc=instruction.address;
			break;
		case JPC:
			if(S[top]==0)
			{
				pc=instruction.address;
			}
			top--;//����
			break;
		case RETP:
			top=bp-1;
			pc=S[top+1];
			bp=S[top+3];
			break;
		case ENDP:
			stop=1;
			break;
		case RED:
			if(instruction.address==0)
			{
				printf("Your Input:");
				scanf("%d",&temp);
			}
			else
				getch();
			S[S[top]]=temp;
			break;
		case WRT:
			if(instruction.address==0)
				printf("Your Output:%d\n",S[top]);		
			else
			{
				ch=(char)S[top];
				printf("Your Output%c\n",ch);
			}
			top--;
			break;
		case MUS:
			S[top]=-S[top];
		case ADD:
		case ADD1:
			top--;
			S[top]=S[top]+S[top+1];
			break;
		case SUB:
			top--;
			S[top]=S[top]-S[top+1];
			break;
		case MULT:
			top--;
			S[top]=S[top]*S[top+1];
			break;
		case IDIV:
			top--;
			S[top]=S[top]/S[top+1];
			break;
		case IMOD:
			top--;
			S[top]=S[top]%S[top+1];
			break;
		case ANDS:
			top--;
			S[top]=S[top]&S[top+1];
			break;
		case ORS:
			top--;
			S[top]=S[top]|S[top+1];
			break;
		case NOTS:
			top--;
			S[top]=~S[top];
			break;
		case EQ:
			top--;
			S[top]=(S[top]==S[top+1])?1:0;
			break;
		case NE:
			top--;
			S[top]=(S[top]!=S[top+1])?1:0;
			break;
		case LS:
			top--;
			S[top]=(S[top]<S[top+1])?1:0;
			break;
		case GE:
			top--;
			S[top]=(S[top]>=S[top+1])?1:0;
			break;
		case GT:
			top--;
			S[top]=(S[top]>S[top+1])?1:0;
			break;
		case LE:
			top--;
			S[top]=(S[top]<=S[top+1])?1:0;
			break;
		}
	}while(!stop);
	
	//printf("��������������������ַ��˳���\n");
	printf("���������\n");
}


int main(int arg,char ** argv)
{	
	char objFileName[255];
	FILE * objFile;
	int objLength;
	int dataunit;
	//INSTRUCTION tempIns;
	
	if(arg>1)
		strcpy(objFileName,argv[1]);
	else
	{
		printf("���������Դ�ļ������֣�");
		scanf("%s",objFileName);
	}

	if(!(objFile=fopen(objFileName,"rb")))
	{
		printf("�������������Դ�ļ�%s���ܴ򿪣�\n",objFileName);
		exit(1);
	}

	fseek(objFile,0,SEEK_END);
	objLength=ftell(objFile);   //��ô����ļ��ĳ���
	rewind(objFile);
	
	if(objLength%(3*sizeof(int)))   //�жϴ����ļ��Ƿ�����
	{
		printf("����Ĵ���Դ�ļ���");
		exit(2);
	}
    dataunit=2*sizeof(int)+sizeof(OPCOD);//һ��ָ�����ĳ���
	int codeSize=objLength/dataunit;   //��ô����ļ������ָ������
	CODE=new INSTRUCTION[codeSize];   //���ɴ�������


	if(!CODE)
	{
		printf("û���㹻�Ķѿɷ��䣡");
		exit(3);
	}

	

	int count=fread(CODE,dataunit,codeSize,objFile);    //�������ļ������������

	printf("%d",count);
	INTERPRET();   //��ʼ����ִ��

	delete CODE;   //������ϣ�ɾ������
	fclose(objFile);
	
	//int a;         //С�ؿ������Բ鿴���н��
	//scanf("%d",a);
}


