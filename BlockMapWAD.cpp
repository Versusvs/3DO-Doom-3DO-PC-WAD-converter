#include "stdafx.h"

///#include <cstdlib>
#include <iostream>

#include "burger.h"
#include <stdio.h>
#include <stdlib.h>
///#include <string.h>
///#include <ctype.h>

#include <conio.h>
//#include <io.h>
//#include <fcntl.h>

//#include <iomanip>
//#include <sstream>

using namespace std;


typedef int Boolean;         /* True if I should swap the endian */
Boolean SwapEndian;
#define TRUE  1	
#define FALSE 0	

static void MakeHeader(void);
static void CalcOffsets(void);
static void MakeFinal(void);

#define Swap2_2Bytes(val) val = ( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )

#define Swap4Bytes(val) val = ( (((val) >> 24) & 0x000000FF) | (((val) >> 8) & 0x0000FF00) | (((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )
//#define SwapULong(val) val = ( (((val) >> 24) & 0x000000FF) | (((val) >> 8) & 0x0000FF00) | (((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )
//#define SwapULong(val) (val << 24 | (val << 8 & 0xFF0000) | (val >> 8 & 0xFF00) | val >> 24 & 0xFF)
#define Swap8Bytes(val) ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )
#define Swap2Bytes(val) ( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )

#define BUFFER_SIZE 0x40000UL
#define Header_SIZE (0x2000*sizeof(VSHeader))

static Byte *Buffer;         /* File buffer for data transfer */
static char InFileName[32]="BLOCKMAP.lmp";
static char FILENAME[32];
char *filenamearg;

// Header_INPUT
//short OriginXShort;
//short OriginYShort;
//short ColumnsNShort;
//short RowsNShort;

// Header_OUTPUT
typedef struct {
	Word OriginX;
	Word OriginY;
	Word ColumnsN;
	Word RowsN;
} DOHeader_t;

// Offsets
//Word Offset;

// Strings
typedef struct {
	Word StrStart;
	Word StrEnd;
} String_t;

typedef struct {
	short OriginXShort;
	short OriginYShort;
	short ColumnsNShort;
	short RowsNShort;
} PCHeader_t;

typedef struct {
	short Blank;
	short Offset;
} Offset_t;
//static Word *Offset;


typedef struct {
	LongWord Blank;
	Word Line;
} DOLine;


short FirstPCOffset;
Word FileSize;

static String_t *StrArray;
static PCHeader_t *Header;
static DOHeader_t *DOHeader;
static Offset_t *Offset;
static DOLine *BlockLine;



static void InitVars(void)
{
Header->OriginXShort=0;
Header->OriginYShort=0;
Header->ColumnsNShort=0;
Header->RowsNShort=0;

DOHeader->OriginX=0;
DOHeader->OriginY=0;
DOHeader->ColumnsN=0;
DOHeader->RowsN=0;
FileSize = 0;
}







static void CalcOffsets(void)
{
FILE *infile, *tmpfile, *tmpfile2, *blocklists_file, *offreal;
//short offset[2];
//char string[32];
int i, j, n;
Word Offset_Size, tmpSize, StartOffset;
// ЕСЛИ НЕ РАБОТАЕТ, ВЕРНУТЬ SHORT!!!!!!!!!!!!!!!!!!!!!!!!
//short offsets[1000];
int offsets[1000];
DOLine *Line;
Line = &BlockLine[4];


infile = fopen(InFileName,"rb");
fseek(infile,0,SEEK_END);
FileSize = ftell(infile);

cout << "FILESIZE=" << FileSize << "\n";

rewind(infile);


//Обнулим массив offsets
i=0;
for (i=0; i<1000; i++) {
		offsets[i]=0;
	};


fseek(infile,8,SEEK_SET);
//fread(&offsets,2,1,infile);
Offset_Size = fread(&offsets,2,1,infile);

fseek(infile,offsets[0]*2,SEEK_SET);

tmpfile = fopen("tmpblocklists","wb");
// Читаем файл, начиная с первой строки blocklists 00 00 ... FF FF
StartOffset=offsets[0]*2;
cout << "StartOffset=" << StartOffset << "\n";

//i=StartOffset;
//j=0;
//cout << "FileSize-StartOffset=" << j << "\n";

///while (!EOF)
///{
///	fread(&StartOffset,0x2,1,infile);
///	fwrite(offsets,2,1,tmpfile);
///}

//short Line[2], endLN[2];

#if 1;
Line->Blank=0;
Word tmpStart, tmpFin, k;
j=k=0;
for (i=StartOffset; (i<FileSize); i++) {

	fread(Line,2,1,infile);
		j++;
		tmpStart = Swap2Bytes(Line->Blank)<<16;
		

		/* ИЗМЕНИТЬ 1 НА 4 !!!!!! */
	fwrite(&tmpStart,4,1,tmpfile);

}
#endif;

cout << "j=" << j << "\n";

fclose(tmpfile);




// Берем временный файл tmpblocklists и заменяем нули перед каждым 0xFFFF.
int Fsize, m;
LongWord *Val;
Word ArrStart[4] = {0x00,0x00,0x00,0x00};
Word ArrEnd[4] = {0x00,0x00,0xFF,0xFF};
Word temp;
//char CheckStr[8];
//int temp[10000];

	struct {
		Word StartStr;
		Word EndStr;
	} strings;
//Byte ArrStart[8] = {"00000000"};
//Byte ArrEnd[8] = {"0000FFFF"};
//char Array[4][3] = {{"00"},{"00"},{"FF"},{"FF"}};
m=0;
Val=0;

tmpfile2 = fopen("tmpblocklists_FF","wb");
tmpfile = fopen("tmpblocklists","rb");
fseek(tmpfile,0,SEEK_END);
Fsize = ftell(tmpfile);
rewind(tmpfile);

for (m=0; m<Fsize/4/2; m++) {
///for (m=0; m<2; m++) {
			fread(&strings.StartStr,sizeof(Word),1,tmpfile);
			temp = strings.StartStr;
// Здесь нужно распознать, что находится в 4-х байтах. Если это 00 00 FF FF, то заменяем их на FF FF FF FF.
// Больше никаких манипуляций не производим.
			if (temp == 0xFFFF0000) {
				temp = 0xFFFFFFFF;
//				cout << "String found! \n";
//				cout << m << "\n";
				} 
//		cout << "temp = " << temp << "\n";
			fwrite(&temp,sizeof(Word),1,tmpfile2);
}
fclose(tmpfile);
fclose(tmpfile2);






// Считаем длины каждой строки 0х0000 ... 0хFFFF и записываем длину каждой в массив.
Word LinesCnt, t, r, p, temp2, temp3, count, length, countBytes, FirstStrPos; 
#define MAXVALUES 30000
int OffsetArr[MAXVALUES]; // Массив под смещения.
//int *OffsetArr = new int[10000]; // Массив под смещения.
Boolean OnlyOnce;
int OffsetTbl; // Длина таблицы смещений
//string TargetStr;

tmpfile2 = fopen("tmpblocklists_FF","rb");
blocklists_file = fopen("blocklists_file","wb"); // Это файл со строками blocklists. Пока без отслеживания пустой 0000 FFFF
// Формируем файл с оффсетами
offreal = fopen("offsets","wb");

fseek(tmpfile2,0,SEEK_END);
Fsize = ftell(tmpfile2);
rewind(tmpfile2);

length = 0;
count = 0;
countBytes = 0; //Initialize
OnlyOnce = FALSE; 


//Нужно сначала обнулить счетчик length и начинать отсчитывать
// на каждый fread. Как только найден конец строки, записываем показания счетчика в массив []
// При следующем fread обнуляем счетчик и снова ищем конец строки. 

//Обнулим массив OffsetArr. Здесь пока используем размер 10000, он потом будет меняться.
p=0;
for (p=0; p<MAXVALUES; p++) {
		OffsetArr[p]=0;
	};


//for (t=0; t<420; t++) {
for (t=0; t<Fsize/4; t++) {
back:
	fread(&strings.EndStr,sizeof(Word),1,tmpfile2);
	countBytes = countBytes+4;
	temp2 = strings.EndStr;
	if (temp2 == 0x00000000) { // Это начало строки
		/* Здесь нужно проверить, есть ли за первыми нулями нули? Пока отслеживаем нули только один раз */
		//	Если за первыми нулями есть еще, то длину плюсуем.
		//		length += 4;
		// В противном случае, длина будет 4, как начало строки.
#if 1
		length = 4;
		fread(&strings.EndStr,sizeof(Word),1,tmpfile2);
			countBytes = countBytes+4;
			temp3=strings.EndStr;
			t = t+1;					//Так как мы сделали fread дополнительный раз, счетчик нужно честно накрутить тоже.
			if (temp3 == 0x00000000) { // Нули после начала строки. Да, и такое бывает. Записываем в файл и плюсуем длину.
//////				length += 4;

				cout << "Zero value found at " << countBytes-4 << "\n";
//////				fwrite(&temp2,sizeof(Word),1,blocklists_file); // Если эту строку включить, 
				// то будут писаться нулевые параметры в строках, если таковые есть в оригинальном PC файле.
				fwrite(&temp3,sizeof(Word),1,blocklists_file);
				continue;
//				goto back; 
				} else

		if (temp3 == 0xFFFFFFFF) { // А если это конец строки?
				length = 8;
				count++;
				OffsetArr[count]=length;
				fwrite(&temp2,sizeof(Word),1,blocklists_file);
				fwrite(&temp3,sizeof(Word),1,blocklists_file);
				continue;
				} else
						{
		// если temp3 не равняется 0х00000000 или 0xFFFFFFFF
		length=length+4; // плюсуем 4 байта
		fwrite(&temp2,sizeof(Word),1,blocklists_file);
		fwrite(&temp3,sizeof(Word),1,blocklists_file);
		continue;
//		goto back;
			} //else

#endif;

		length=4; // Отсчитываем 4 байта
// Проверяем, есть ли за нулями FFFFF?
// Если да, то это первая ПУСТАЯ строка и OnlyOnce=TRUE;
// Тут же нужно запомнить ее countBytes для дальнейшего формирования нового файла off_file2 уже без дубликатов пустых строк.
// Если пустая строка (то есть, только 0000 FFFFF), то ее размер = 8
// Сохранить ее нужно только один раз в том месте, где она первый раз встретилась. 
// Все остальные такие пустые строки игнорируем. Для этого просто откатываемся на 8 байт назад и продолжаем писать в файл последующие строки.
// Если нет, то идем дальше.

#if 0 // Включая этот код, мы говорим программе собирать файл blocklists_file без повторяющихся строк 00000000 FFFFFFFFF.
			fread(&strings.EndStr,sizeof(Word),1,tmpfile2);
			temp3=strings.EndStr;
//			cout << "temp3=" << temp3 << "\n";
			t = t+1;					//Так как мы сделали fread дополнительный раз, счетчик нужно честно накрутить тоже.
			if (temp3 == 0xFFFFFFFF) { // Это конец ПУСТОЙ строки. Встретили ее первый раз. Значит ее нужно записать.
				if (OnlyOnce==TRUE) {	// Первую пустую строку мы уже записали, значит просто идем
					countBytes = countBytes+4;
					// Строку считаем все равно.
					count++;
					length=8;
					OffsetArr[count]=length;
					continue;
//					goto back;
				}
//				cout << "temp3=" << temp3 << "\n";
			countBytes = countBytes+4;
			length=8;
			count++;
			OffsetArr[count]=length;
			OnlyOnce=TRUE;
			FirstStrPos = count; // Запоминаем позицию первой строки, чтобы потом ее использовать для формирования новой таблицы смещений.
			fwrite(&temp2,sizeof(Word),1,blocklists_file);
			fwrite(&temp3,sizeof(Word),1,blocklists_file);
			cout << "First empty string - OffsetArr[" << count << "]=" << OffsetArr[count] << "\n";
			continue;
//			goto write;
		} else 
			{
		// если temp3 не равняется никакому из шаблонов
		length=length+4; // плюсуем 4 байта
		countBytes = countBytes+4;
		fwrite(&temp2,sizeof(Word),1,blocklists_file);
		fwrite(&temp3,sizeof(Word),1,blocklists_file); 
		continue;
			}
//			goto contStr;
#endif;
//		length=4; // Отсчитываем 4 байта
	} else
//goFWD:
	if (temp2 == 0xFFFFFFFF) { // Это конец строки
		// Если найден конец строки, то вычисляем ее длину.
		length = length+4; // Отсчитываем 4 байта
		// Считаем каждую строку. Это число и будет размером OffsetArrCut[].
		count++;
		// Сохраняем итоговую длину строки в байтах в массив!
		OffsetArr[count]=length;
/////		cout << "OffsetArr[" << count << "]" <<"=" << OffsetArr[count] << "\n";
//		TargetStr = itoa(temp2,BlockString.String,16);

	} else	{
contStr:
	// если temp2 не равняется никакому из шаблонов
		length=length+4; // плюсуем 4 байта
	}
////cout << "length=" << length << "\n";

// Сюда пишем все строки. Пока пишу несортированные строки, т.е. все, которые попали из tmpblocklists_FF. 
// Потом буду сюда писать с одной пустой строкой без повторений.!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//write:
fwrite(&temp2,sizeof(Word),1,blocklists_file); 
}
cout << "count=" << count << "\n";
cout << "countBytes=" << countBytes << "\n";




// Скопируем элементы из первого массива во второй с уже известным размером. 
LongWord *OffsetArrCut=new LongWord [count];
int	d;

for (d=0; d<count+1; d++) {
	OffsetArrCut[d] = OffsetArr[d];
//	cout << "OffsetArrCut[" << d << "]" <<"=" << OffsetArrCut[d] << "\n";
}




// Подсчитаем длину таблицы смещений, чтобы понять какое будет первое смещение. Эта же цифра будет и первым смещением для строк.
OffsetTbl = count*4+16; // здесь 16 - размер шапки DOHeader
cout << "OffsetTbl=" << OffsetTbl << "\n";

// Пишем оффсеты, используя длину таблицы оффсетов OffsetTbl и длины строк length, записанные в массив OffsetArr[].
r=1;
int FirstOff = OffsetTbl; // Первое смещение будет указывать на начало всех blocklists.
//int CurLength=0;
short SwapedOff=0;
//int Swaped2=0;
int CurLength=FirstOff;
int tmp=0;

// Запишем в файл первое смещение FirstOff
//tmp = Swap4Bytes(FirstOff);
fwrite(&FirstOff,sizeof(Word),1,offreal);

//for (r=1; r<3; r++) { 
for (r=1; r<count; r++) {					/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Проверить количество оффсетов, записываемых в файл!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//for (r=1; r<sizeof(OffsetArr)/4/4; r++) {		// Первый элемент массива мы уже записали, поэтому начинаем с 1.

//	Подсчитаем смещение каждой последующей строки. 	
////	cout << "CurLength=" << CurLength << "\n";
	CurLength = CurLength+OffsetArrCut[r];
//	SwapedOff = Swap4Bytes(CurLength);
///	Swaped2 = Swap2Bytes(SwapedOff);
	
fwrite(&CurLength,4,1,offreal); 
}
fclose(offreal);

fclose(tmpfile2);
fclose(blocklists_file);

fclose(infile);


/*-----------------------------------------------------------------------------------------------------------------------------*/

#if 1
#define MAXLENGTH 128
#define MAXSTRUCTVALUES 6000

FILE *infl, *outfl, *offfl;
char tmpStr[MAXLENGTH];
int CurrOff, h, tmp2, cnt, dup, strcmprez, s, cur, Num;
Word tmpOff, tmpOff2, tmpOff3, Ofs;
Boolean Equal, FirstStr, EqualFound, OrigFound, Once;
char *tmpstr[1024][4]; // Такие размеры, чтобы полностью охватить [256]
// Исходный файл для обработки - tmpblocklists_FF.
// Результирующий файл - new_lump10.

#if 1
// Структура со строкой и ее параметрами
struct {
	char String[MAXLENGTH];
	Word startOFF;
	Word length;
	int posStr;
}	BlockString[MAXSTRUCTVALUES];		// Здесь нужно определить размер динамического массива. Пока статический.
#endif;

#if 0
struct BlockString1{
	char String[MAXLENGTH];
	Word startOFF;
	Word length;
	int posStr;
};		
BlockString1 *BlockString = new BlockString1[count+1]; // Динамически выделим память под массив из count элементов (ДОДЕЛАТЬ!!!!!!!!!!!!!!)
#endif;
//int count = 10000;
//BlockString MainArr[count]; // размерность его мы уже знаем.


h=0;
//CurrOff = StartOffset;
CurrOff = OffsetTbl;
tmpOff2 = tmpOff3 = OffsetTbl;
Equal = FALSE;
FirstStr = TRUE;
EqualFound = FALSE;
OrigFound = FALSE;
//Once = TRUE;
infl = fopen("blocklists_file","rb");
//infl = fopen("tmpblocklists_FF","rb");
offfl = fopen("off_sorted","wb");
outfl = fopen("temp","wb");
	
//	fread(&BlockString[1].String,OffsetArrCut[1],1,infl); // Запишем первый элемент массива
//	BlockString[1].startOFF = CurrOff;
//	BlockString[1].length = OffsetArrCut[1];
//	BlockString[1].posStr = 1;
//	fwrite(BlockString[1].String,OffsetArrCut[1],1,outfl);
	
//	fwrite(&CurrOff,sizeof(Word),1,offfl);

//	CurrOff += BlockString[1].length;


for (h=1; h<count+1; h++) {
//for (h=1; h<3; h++) {
//for (h=1; h<173; h++) {

	fread(&BlockString[h].String,OffsetArrCut[h],1,infl);

//	BlockString[h].startOFF = tmpOff2;
	BlockString[h].startOFF = CurrOff;
	BlockString[h].length = OffsetArrCut[h];
	BlockString[h].posStr = h;

#if 1
if (FirstStr == TRUE) {			// Первую строку пишем всегда.
		fwrite(BlockString[h].String,OffsetArrCut[h],1,outfl);
		tmpOff = BlockString[h].startOFF;
///		tmpOff = Swap4Bytes(BlockString[h].startOFF);
		fwrite(&tmpOff,sizeof(Word),1,offfl);
		Num = BlockString[h].posStr;
//		tmpOff2 = CurrOff - OffsetArrCut[h];
//		CurrOff += OffsetArrCut[h];
		//h++;
		FirstStr = FALSE;
		continue;
} 
//else {
#endif;
	// Здесь будем искать строку [h] по всему массиву и искать одинаковые. 
	// Если найдем такую же, то
	// текущую строку не сохраняем и оффсеты последующим строкам не будем делать CurrOff += OffsetArrCut[h];
	// То есть, оффсет следующей строки так и останется CurrOff;
	// Но оффсет текущей строки будет = оффсету строки оригинала.
#if 1
//	strcmprez = -1;
	for (dup=1; dup<h; dup++) {
//		cout << BlockString[h].String << "\n";
///			cout << "Current BlockString # is " << h << "\n";
///			cout << "Compare with BlockString[" << dup << "]\n";
////		strcmprez = strcmpi(BlockString[dup].String, BlockString[h].String);
////		if (strcmprez==0) {
		Equal = FALSE;
	#if 1
cur = dup;
anotherElement:
				for (s=0; s<MAXLENGTH; s++) {
					if (BlockString[cur].String[s] != BlockString[h].String[s]) {
							// Если строка h не равна строке cur, мы должны проверить остальные элементы массива до h.
						if (cur==h-1) {
							Equal = FALSE;
//							Num = BlockString[cur].posStr;
							goto mark;
							} else {
						cur++;
						goto anotherElement;
//						cout << "STR NOT CMP! Pos in str=" << s << "\n";
//						cout << "This is ORIGINAL string! h=" << h << "\n";
						//goto mark;
						}
					} else {
						Equal = TRUE;
//						EqualFound = TRUE;
						}
				}
mark:
				if (Equal == TRUE) {
	#endif; 

/////		if (BlockString[dup].length == BlockString[h].length) {		// Тест для сравнения длин.

				// Если дубль строки найден
			cout << "Duplicate found in [" << h << "]\n";
			// То сохраняем смещение найденной строки-дубликата, 
			// и назначаем такой же, какой был индекс у встретившегося дубля.

//				Num = BlockString[cur].posStr;
//				BlockString[cur].startOFF = BlockString[h].startOFF;
//				CurrOff = BlockString[cur].startOFF + BlockString[cur].length;
//				tmpOff2 = CurrOff + OffsetArrCut[cur];
///				tmpOff3 = CurrOff + OffsetArrCut[cur];
//				tmpOff2 = CurrOff + OffsetArrCut[h];

				tmpOff2 = BlockString[cur+1].startOFF;
				fwrite(&tmpOff2,sizeof(Word),1,offfl); // Пишем оффсеты.
//				BlockString[h].startOFF = tmpOff2;
				// Запоминаем порядковый номер строки, которая явилась оригиналом для дубля.

//				if (EqualFound == TRUE) { // Если хотя бы одна строка-дубль уже была найдена, 
//					EqualFound = FALSE;
//					break;
//				} else {
//				EqualFound = TRUE;
//				}

			// Саму строку не сохраняем и стартовое смещение следующей строки будет таким же, какое и текущее.
			break;
		} else {
/*===============================================================================================================*/
			cout << "This is an ORIGINAL string! h=" << h << "\n";
			OrigFound = TRUE;
			//Оригинальную строку сохраняем, оффсет тоже и считаем следующее смещение.
		fwrite(BlockString[h].String,OffsetArrCut[h],1,outfl);
			

			CurrOff += OffsetArrCut[Num];
			fwrite(&CurrOff,sizeof(Word),1,offfl); // Пишем оффсеты.
			Num = BlockString[h].posStr;
		break;
#if 0
		if (EqualFound == TRUE) {
			// Встречались ли уже оригиналы или дубли? 
// Если мы нашли оригинальную строку после дубликатов, то нам нужно смещение и длина предыдущей ОРИГИНАЛЬНОЙ строки. 
			CurrOff = BlockString[Num].startOFF + BlockString[Num].length;  // Если это после дубликата, то
//			Num = BlockString[cur].posStr;
			tmpOff3 = CurrOff;
//			tmpOff3 = Swap4Bytes(CurrOff);
		fwrite(&tmpOff3,sizeof(Word),1,offfl); // Пишем оффсеты.
			EqualFound = FALSE;
			break;

		} else {	// Если это не после дубликата.
				CurrOff += OffsetArrCut[cur];
//				CurrOff = BlockString[Num].startOFF + BlockString[Num].length;
				tmpOff3 = CurrOff;
//				tmpOff3 = Swap4Bytes(CurrOff);
			fwrite(&tmpOff3,sizeof(Word),1,offfl); // Пишем оффсеты.
				// Запомним номер строки, в которой был крайний оригинал.
				Num = BlockString[h].posStr;
				OrigFound = TRUE;
				break;
				}
#endif;
/*=============================================================================================================*/
		}
	}
/////}
	// Если не нашли такую же, то просто записываем ее и ее оффсет в файл outfl и off_sorted.
#endif;


//SaveFirst:
/*-----------------Здесь рабочий код сохранения оффсетов и строк----------------------*/
#if 0
	fwrite(BlockString[h].String,OffsetArrCut[h],1,outfl);
	tmpOff = BlockString[h].startOFF;
////////	tmpOff = Swap4Bytes(BlockString[h].startOFF);
	fwrite(&tmpOff,sizeof(Word),1,offfl); // Пишем оффсеты. Пока не сортируем.

///	cout << "BlockString[" << h << "].startOFF=" << BlockString[h].startOFF << "\n";
	CurrOff += OffsetArrCut[h]; // Следующее стартовое смещение будет больше на текущую длину строки
#endif;
/*-------------------------------------------------------------------------------------*/
}

#if 0
cnt=0; // Первый элемент мы уже сохранили.
///do {
///	cnt++;
///for (h=0; h<OffsetArrCut[h+cnt]/4; h++) {
for (h=0; h<count+1; h++) {
//	fread(&BlockString[h+cnt].String,sizeof(Word),1,infl);
	fread(&BlockString[h+cnt].String,OffsetArrCut[h+cnt],1,infl);

	BlockString[h+cnt].startOFF = CurrOff;
	BlockString[h+cnt].length = OffsetArrCut[h+cnt];
	BlockString[h+cnt].posStr = h+cnt;
	
//	tmpstr[h][cnt] = BlockString[h].String;
	fwrite(BlockString[h+cnt].String,OffsetArrCut[h+cnt],1,outfl);

	tmpOff = BlockString[h+cnt].startOFF;
	fwrite(&tmpOff,sizeof(Word),1,offfl);

	cout << "BlockString[" << h+cnt << "].startOFF=" << BlockString[h+cnt].startOFF << "\n";
	CurrOff += OffsetArrCut[h+cnt]; // Следующее стартовое смещение будет больше на текущую длину строки
}
///cout << "cnt="<< cnt <<"\n";
///} while (cnt<count);
//} while (cnt<130);
#endif;


fclose(infl);
fclose(offfl);
fclose(outfl);
/*-------------------------------- Переворачиваем байты файла смещений ------------------------*/
#if 1
int i2, OffSize1;
FILE *in, *out;

struct {
	Word Body;
} offst1;

in = fopen("off_sorted","rb");
out = fopen("off_sorted_swaped","wb");

// Обрабатываем файл смещений
fseek(in,0,SEEK_END);
OffSize1 = ftell(in);
rewind(in);
for (i2=0; i2<OffSize1/4; i2++) {
	fread(&offst1.Body,4,1,in);
	temp = Swap4Bytes(offst1.Body);
	fwrite(&temp,4,1,out);	
}

fclose(in);
fclose(out);
/*-------------------------------- Переворачиваем байты файла смещений----------- конец ------------------------*/
#endif;

/*----------Здесь считаем новые оффсеты от строк без повторений и записываем в файл off_sorted------------------*/
#if 0
FILE *ofFl, *infile1;
int len, coun, p1, t1;
Word temp4;

struct {
	Word Offs;
} Sorted;

infile1 = fopen("temp","rb");
ofFl = fopen("off_sorted","wb");

fseek(infile1,0,SEEK_END);
Fsize = ftell(infile1);
rewind(infile1);

len = 0;
coun = 0;

p1=0;
for (p1=0; p1<10000; p1++) {
		OffsetArr[p1]=0;
	};

//------------------------------------
for (t1=0; t1<Fsize/4; t1++) {
	fread(&Sorted.Offs,sizeof(Word),1,infile1);
	temp4 = Sorted.Offs;
	if (temp4 == 0x00000000) { // Это начало строки
			len=4; // Отсчитываем 4 байта
// Проверяем, есть ли за нулями FFFFF?
	} else
	if (temp4 == 0xFFFFFFFF) { // Это конец строки
		// Если найден конец строки, то вычисляем ее длину.
		len = len+4; // Отсчитываем 4 байта
		// Считаем каждую строку. Это число и будет размером OffsetArrCut[].
		coun++;
		// Сохраняем итоговую длину строки в байтах в массив!
		OffsetArr[coun]=len;

	} else	{
	// если temp2 не равняется никакому из шаблонов
		len=len+4; // плюсуем 4 байта
	}
//fwrite(&temp4,sizeof(Word),1,ofFl); 
}
//------------------------------------

// Скопируем элементы из первого массива во второй с уже известным размером. 
int *OffsetArrCut2=new int[coun];
int	d2;

for (d2=0; d2<coun+1; d2++) {
	OffsetArrCut2[d2] = OffsetArr[d2];
	cout << "OffsetArrCut2[" << d2 << "]" <<"=" << OffsetArrCut2[d2] << "\n";
}


#if 0
// Подсчитаем длину таблицы смещений, чтобы понять какое будет первое смещение. Эта же цифра будет и первым смещением для строк.
OffsetTbl = coun*4+16; // здесь 16 - размер шапки DOHeader
cout << "OffsetTbl=" << OffsetTbl << "\n";

// Пишем оффсеты, используя длину таблицы оффсетов OffsetTbl и длины строк length, записанные в массив OffsetArr[].
r=1;
int FirstOff = OffsetTbl; // Первое смещение будет указывать на начало всех blocklists.
//int CurLength=0;
short SwapedOff=0;
//int Swaped2=0;
int CurLength=FirstOff;
int tmp=0;

// Запишем в файл первое смещение FirstOff
//tmp = Swap4Bytes(FirstOff);
fwrite(&FirstOff,sizeof(Word),1,offreal);

for (r=1; r<count; r++) {					/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Проверить количество оффсетов, записываемых в файл!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//	Подсчитаем смещение каждой последующей строки. 	
	CurLength = CurLength+OffsetArrCut[r];

	
fwrite(&CurLength,4,1,offreal); 
}
#endif;

fclose(infile1);
#endif;
/*--------------------------------------------------------------------------------------------------------------*/

/* ---------------------------------------- 
Чтобы найти и устранить дубликаты всех строк.

1) Сохраняем строки в динамический массив структур. Берем снова исходный файл, откуда читали строки: tmpblocklists_FF.

Данные для чтения мы уже имеем. В массиве OffsetArrCut записаны все длины строк. Начальное исходное смещение мы знаем: OffsetTbl.
Чтобы получить адрес начала следующей строки, просто приплюсовываем к текущему величину [i+1].

Сам массив выглядит так:
struct {
	char String[256];
	Word startOFF;
	Word lenght;
	int posStr;
}	BlockString;

Итак, мы имеем динамический массив структур со строками и их параметрами. 

2) Заполняем в массиве строки, оффсеты и ищем дубли строк.

Читаем элемент массива. Смотрим, есть ли в этом же массиве уже такая же? 
Здесь алгоритм поиска по массиву и сравнения текущей строки с уже имеющимися.
--Нет? Пишем в новый массив структур или оставляем на месте без изменения.
--Да? Оффсеты последующих элементов структур смещаются вниз (от них нужно отнять length выбрасываемой строки) по значению выброшенного оффсета.
Также нам нужен порядковый номер строки, сравнение с которой дало TRUE (1) и порядковый номер нашей строки с дубликатом (2).
Оффсет на позиции отброшенной строки (2) = оффсету строки (1).
Оффсет на нашу текущую строку (2) должен быть заменен на оффсет строки (1).
Саму строку мы уже не пишем.
Итого, имеем массив структур с некоторыми пустыми параметрами строк и одинаковымии оффсетами на тех же местах, 
которые равняются оффсетам, взятым с тех мест, где впервые была найдена строка. 
Оффсет копируется на новое место, а самой строки (как параметра) там нет. Ебнуться можно...

3) Формируем оффсеты. Просто читаем последовательно все смещения из массива. Будут попадаться одинаковые на местах дубликатов строк.
Формируем строки. Читаем параметры строки из массива, ПРОПУСКАЯ пустые.

4) Пишем шапку файла, смещения файла, строки файла.




После того, как я отфильтрую дубликаты, у меня будет список строк и соответствующих им параметров. (начала длины)
Тем не менее, мне нужно подсчитать количество дублей(?) и оставить первый и единственный на месте. Количество же смещений должно быть прежним,
только смещения на одинаковые строки должны указывать на первую уникальную.
 ---------------------------------------- */

#endif;
}



static void MakeFinal(void) // Собираем из временных файлов один результирующий лумп 10 BlockMap.
{
	FILE *hdr, *off, *blklst, *lump10;
	int HdrSize, OffSize, BlkSize, i, temp;

struct {
		Word OriginX;
		Word OriginY;
		Word ColumnsN;
		Word RowsN;
	} head;

struct {
	Word Body;
} offst;

struct {
	Word Body;
} blkstr;

// Берем файл заголовков tmpheader
hdr = fopen("tmpheader","rb");
// Берем файл оффсетов offsets
///off = fopen("offsets","rb");
off = fopen("off_sorted_swaped","rb");
// Берем файл строк blocklists_file
///blklst = fopen("blocklists_file","rb");
blklst = fopen("temp","rb");
// Сохраняем результирующий файл lump10
//lump10 = fopen("rMAP01_lump10","wb");
////
#if 1;
char tmpstr18[20]={0}, tmpstr19[20]={0};
char *LastLump;
	strncpy(tmpstr18, filenamearg, 5);
	tmpstr19[0] = 114;	// "r" symbol
	strcat(tmpstr19, tmpstr18);				// Получилось "rMAP01"
	LastLump = strcat(tmpstr19, "_lump10");
	lump10 = fopen(tmpstr19,"wb");
#endif;
/////

// Обрабатываем файл заголовка
fseek(hdr,0,SEEK_END);
HdrSize = ftell(hdr);
rewind(hdr);
	fread(&head.OriginX,4,1,hdr);
	temp = head.OriginX;
	fwrite(&temp,4,1,lump10);

	fread(&head.OriginY,4,1,hdr);
	temp = head.OriginY;
	fwrite(&temp,4,1,lump10);

	fread(&head.ColumnsN,4,1,hdr);
	temp = head.ColumnsN;
	fwrite(&temp,4,1,lump10);

	fread(&head.RowsN,4,1,hdr);
	temp = head.RowsN;
	fwrite(&temp,4,1,lump10);

// Обрабатываем файл смещений
fseek(off,0,SEEK_END);
OffSize = ftell(off);
rewind(off);
for (i=0; i<OffSize/4; i++) {
	fread(&offst.Body,4,1,off);
///	temp = Swap4Bytes(offst.Body);
	temp = offst.Body;
	fwrite(&temp,4,1,lump10);	
}

// Обрабатываем файл строк
fseek(blklst,0,SEEK_END);
BlkSize = ftell(blklst);
rewind(blklst);
for (i=0; i<BlkSize/4; i++) {
	fread(&blkstr.Body,4,1,blklst);
	temp = blkstr.Body;
	fwrite(&temp,4,1,lump10);	
}

fclose(hdr);
fclose(off);
fclose(blklst);
fclose(lump10);

cout << "\n";
cout << "Finished converting " << filenamearg << "\n";
cout << "\n";
}



static void MakeHeader(void)
{
FILE *infile, *tmpfile;


infile = fopen(InFileName,"rb");

fseek(infile,0x0,SEEK_SET);
fread(Header,0x8,1,infile);


//Header->OriginXShort=Swap2Bytes(Header->OriginXShort);
//Header->OriginYShort=Swap2Bytes(Header->OriginYShort);
//Header->ColumnsNShort=Swap2Bytes(Header->ColumnsNShort);
//Header->RowsNShort=Swap2Bytes(Header->RowsNShort);

cout << "OriginX=" << Header->OriginXShort;
cout << "\n";
cout << "OriginY=" << Header->OriginYShort;
cout << "\n";

cout << "ColumnsN=" << Header->ColumnsNShort;
cout << "\n";
cout << "RowsN=" << Header->RowsNShort;
cout << "\n";

tmpfile = fopen("tmpheader","wb");

DOHeader->OriginX=Header->OriginXShort;
DOHeader->OriginY=Header->OriginYShort;
DOHeader->ColumnsN=Header->ColumnsNShort;
DOHeader->RowsN=Header->RowsNShort;

#if 1;
DOHeader->OriginX=Swap2Bytes(Header->OriginXShort);
DOHeader->OriginY=Swap2Bytes(Header->OriginYShort);
DOHeader->ColumnsN=Swap2Bytes(Header->ColumnsNShort);
DOHeader->RowsN=Swap2Bytes(Header->RowsNShort);
#endif;

DOHeader->ColumnsN=DOHeader->ColumnsN << 16;
DOHeader->RowsN=DOHeader->RowsN << 16;

fwrite(DOHeader,16,1,tmpfile);
fclose(tmpfile);

fclose(infile);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CreateThingsLump (FILE *wadfile, char *outfilename, int lump_offset, int lump_size)
{
	struct {
		short x, y;
		short angle;
		short type;
		short flags;
	} thing;

	FILE *lumpfile;
	int i, entry_count, lump_count, temp;
	lumpfile = fopen (outfilename, "wb");
	if (!lumpfile) {
		printf ("\nLump file %s can't create", outfilename);
		getch();
	}
	
	fseek (wadfile, lump_offset, SEEK_SET);
	entry_count = lump_size / sizeof (thing);
	Swap4Bytes(entry_count);
	fwrite(&entry_count, 4, 1, lumpfile);
	Swap4Bytes(entry_count);
	i = 0;
	do {
			fread (&thing.x, 2, 1, wadfile);
			temp = thing.x;
			Swap2_2Bytes(temp); 
			fwrite(&temp, 4, 1, lumpfile);
			fread (&thing.y, 2, 1, wadfile);
			temp = thing.y;
			Swap2_2Bytes(temp); 
			fwrite(&temp, 4, 1, lumpfile);
			fread (&thing.angle, 2, 1, wadfile);
			thing.angle = thing.angle / 1.40625;
			temp = thing.angle;
			fwrite(&temp, 4, 1, lumpfile);
			fread (&thing.type, 2, 1, wadfile);
			temp = thing.type;
			Swap4Bytes(temp); 
			fwrite(&temp, 4, 1, lumpfile);
			fread (&thing.flags, 2, 1, wadfile);
			temp = thing.flags;
			Swap4Bytes(temp); 
			fwrite(&temp, 4, 1, lumpfile);
			i++;
	} while (i < entry_count);
	fclose (lumpfile);
}

void CreateLinedefsLump (FILE *wadfile, char *outfilename, int lump_offset, int lump_size)
{
	struct {
		short v1, v2;		
		short flags;		
		short special;	
		short tag;		
		short sidenum[2];	
	} linedef;

	FILE *lumpfile;
	int i, entry_count, lump_count, temp;
//	char buf1;
	int buf1;	// Заменил на int, потому что писал ненужные СС.
	short buf2;
	
	lumpfile = fopen (outfilename, "wb");
	if (!lumpfile) {
		printf ("\nLump file %s can't create", outfilename);
		getch();
	}
	fseek (wadfile, lump_offset, SEEK_SET);
	entry_count = lump_size / sizeof (linedef);
	Swap4Bytes(entry_count);
	fwrite(&entry_count, 4, 1, lumpfile);
	Swap4Bytes(entry_count);
	buf1 = 0;
	buf2 = -1;
	i = 0;
	do {
			fread (&linedef.v1, 2, 1, wadfile);
			temp = linedef.v1;
			Swap4Bytes(temp); 
			fwrite(&temp, 4, 1, lumpfile);
			fread (&linedef.v2, 2, 1, wadfile);
			temp = linedef.v2;
			Swap4Bytes(temp); 
			fwrite(&temp, 4, 1, lumpfile);
			fread (&linedef.flags, 2, 1, wadfile);
			temp = linedef.flags;
			Swap4Bytes(temp); 
			fwrite(&temp, 4, 1, lumpfile);
			fread (&linedef.special, 2, 1, wadfile);
			temp = linedef.special;
			Swap4Bytes(temp); 
			fwrite(&temp, 4, 1, lumpfile);
			fread (&linedef.tag, 2, 1, wadfile);
			temp = linedef.tag;
			Swap4Bytes(temp); 
			fwrite(&temp, 4, 1, lumpfile);
			fread (&linedef.sidenum[0], 2, 1, wadfile);
			if (linedef.sidenum[0] == -1) {
				fwrite(&buf2, 2, 1, lumpfile);
				fwrite(&linedef.sidenum[0], 2, 1, lumpfile);
			}
			else {
				Swap2_2Bytes(linedef.sidenum[0]);
				fwrite(&buf1, 1, 2, lumpfile);					// Пишет ненужные СС.
				fwrite(&linedef.sidenum[0], 2, 1, lumpfile);
			}
			fread(&linedef.sidenum[1], 2, 1, wadfile);
			if (linedef.sidenum[1] == -1) {
				fwrite(&buf2, 2, 1, lumpfile);
				fwrite(&linedef.sidenum[1], 2, 1, lumpfile);
			}
			else {
				Swap2_2Bytes(linedef.sidenum[1]);
				fwrite(&buf1, 1, 2, lumpfile);
				fwrite(&linedef.sidenum[1], 2, 1, lumpfile);
			}
			i++;
	} while (i < entry_count);
	fclose (lumpfile);
}

void CreateSideDefsLump (FILE *wadfile, char *outfilename, int lump_offset, int lump_size)
{
	struct {
		short x_offset, y_offset;
		char upper_texturename[8];
		char lower_texturename[8];
		char middle_texturename[8];
		short numsector;
	} sidedef;

	FILE *lumpfile, *textures;
	int i, index, entry_count, lump_count, filesize, temp;
	char buf[10000];
	char *str;

	lumpfile = fopen(outfilename, "wb");
	if (!lumpfile) {
		printf("\nLump file %s can't create", outfilename);
		getch();
	}
	textures = fopen("textures.txt", "r");
	if (!textures) {
		printf("\ntextures.txt can't open");
		getch();
	}
	/*fseek(textures, 0, SEEK_END);
	filesize = ftell(textures);
	rewind(textures);*/
	i = 0;
	while (!feof(textures)) {
		buf[i] = fgetc(textures);
		i++;
	}
	fclose(textures);
	fseek(wadfile, lump_offset, SEEK_SET);
	entry_count = lump_size / sizeof (sidedef);
	Swap4Bytes(entry_count);
	fwrite(&entry_count, 4, 1, lumpfile);
	Swap4Bytes(entry_count);
	i = 0;
	do {
		fread(&sidedef.x_offset, 2, 1, wadfile);
		temp = sidedef.x_offset;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&sidedef.y_offset, 2, 1, wadfile);
		temp = sidedef.y_offset;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&sidedef.upper_texturename, 8, 1, wadfile);
		if (strlen(sidedef.upper_texturename) > 8) sidedef.upper_texturename[8] = 0;
		strupr(sidedef.upper_texturename);
	    str = strstr(buf, sidedef.upper_texturename);
		index = (str-buf) / 9;
		Swap4Bytes(index);
		fwrite(&index, 4, 1, lumpfile);
		fread(&sidedef.lower_texturename, 8, 1, wadfile);
		if (strlen(sidedef.lower_texturename) > 8) sidedef.lower_texturename[8] = 0;
	    strupr(sidedef.lower_texturename);
		str = strstr(buf, sidedef.lower_texturename);
		index = (str-buf) / 9;
		Swap4Bytes(index);
		fwrite(&index, 4, 1, lumpfile);
		fread(&sidedef.middle_texturename, 8, 1, wadfile);
		if (strlen(sidedef.middle_texturename) > 8) sidedef.middle_texturename[8] = 0;
		strupr(sidedef.middle_texturename);
		str = strstr(buf, sidedef.middle_texturename);
		index = (str-buf) / 9;
		Swap4Bytes(index);
		fwrite(&index, 4, 1, lumpfile);
		fread(&sidedef.numsector, 2, 1, wadfile);
		temp = sidedef.numsector;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		i++;
	} while (i < entry_count);
	fclose (lumpfile);
}

void CreateVertexesLump (FILE *wadfile, char *outfilename, int lump_offset, int lump_size)
{
	struct {
		short x_pos;
		short y_pos;
	} vertex;
	
	FILE *lumpfile;
	int i, entry_count, lump_count, temp;
		
	lumpfile = fopen (outfilename, "wb");
	if (!lumpfile) {
		printf ("\nLump file %s can't create", outfilename);
		getch();
	}
	fseek (wadfile, lump_offset, SEEK_SET);
	entry_count = lump_size / sizeof (vertex);
	i = 0;
	do {
		fread(&vertex.x_pos, 2, 1, wadfile);
		temp = vertex.x_pos;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&vertex.y_pos, 2, 1, wadfile);
		temp = vertex.y_pos;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		i++;
	} while (i < entry_count);
	fclose (lumpfile);
}

void CreateSegsLump (FILE *wadfile, char *outfilename, int lump_offset, int lump_size)
{
	struct {
		short first_vertex;
		short last_vertex;
		short angle;
		short linedef;
		short direction;
		short dist;
	} seg;

	FILE *lumpfile;
	int i, entry_count, lump_count, temp;
		
	lumpfile = fopen (outfilename, "wb");
	if (!lumpfile) {
		printf ("\nLump file %s can't create", outfilename);
		getch();
	}
	fseek (wadfile, lump_offset, SEEK_SET);
	entry_count = lump_size / sizeof (seg);
	Swap4Bytes(entry_count);
	fwrite(&entry_count, 4, 1, lumpfile);
	Swap4Bytes(entry_count);
	
	i = 0;
	do {
		fread(&seg.first_vertex, 2, 1, wadfile);
		temp = seg.first_vertex;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&seg.last_vertex, 2, 1, wadfile);
		temp = seg.last_vertex;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&seg.angle, 2, 1, wadfile);
		temp = seg.angle;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&seg.linedef, 2, 1, wadfile);
		fread(&seg.direction, 2, 1, wadfile);
		fread(&seg.dist, 2, 1, wadfile);
		temp = seg.dist;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		temp = seg.linedef;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		temp = seg.direction;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		i++;
	} while (i < entry_count);
	fclose (lumpfile);
}

void CreateSubSectorsLump (FILE *wadfile, char *outfilename, int lump_offset, int lump_size)
{
	struct {
		short sec_count;
		short seg_num;
	} subsector;

	FILE *lumpfile;
	int i, entry_count, lump_count, temp;
		
	lumpfile = fopen (outfilename, "wb");
	if (!lumpfile) {
		printf ("\nLump file %s can't create", outfilename);
		getch();
	}
	fseek (wadfile, lump_offset, SEEK_SET);
	entry_count = lump_size / sizeof (subsector);
	Swap4Bytes(entry_count);
	fwrite(&entry_count, 4, 1, lumpfile);
	Swap4Bytes(entry_count);
	
	i = 0;
	do {
		fread(&subsector.sec_count, 2, 1, wadfile);
		temp = subsector.sec_count;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&subsector.seg_num, 2, 1, wadfile);
		temp = subsector.seg_num;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		i++;
	} while (i < entry_count);
	fclose (lumpfile);
}

void CreateNodesLump (FILE *wadfile, char *outfilename, int lump_offset, int lump_size)
{
	struct {
		short x, y, dx, dy;
		short rbox_top, rbox_bottom, rbox_left, rbox_right;
		short lbox_top, lbox_bottom, lbox_left, lbox_right;
		short rchild, lchild;
	} node;

	FILE *lumpfile;
	int i, entry_count, lump_count, temp;
	int buf1; 
//	char buf1;

	lumpfile = fopen (outfilename, "wb");
	if (!lumpfile) {
		printf ("\nLump file %s can't create", outfilename);
		getch();
	}
	fseek (wadfile, lump_offset, SEEK_SET);
	entry_count = lump_size / sizeof (node);
	Swap4Bytes(entry_count);
	fwrite(&entry_count, 4, 1, lumpfile);
	Swap4Bytes(entry_count);
	buf1 = 0;
	i = 0;
	do {
		fread(&node.x, 2, 1, wadfile);
		temp = node.x;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.y, 2, 1, wadfile);
		temp = node.y;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.dx, 2, 1, wadfile);
		temp = node.dx;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.dy, 2, 1, wadfile);
		temp = node.dy;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.rbox_top, 2, 1, wadfile);
		temp = node.rbox_top;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.rbox_bottom, 2, 1, wadfile);
		temp = node.rbox_bottom;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.rbox_left, 2, 1, wadfile);
		temp = node.rbox_left;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.rbox_right, 2, 1, wadfile);
		temp = node.rbox_right;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.lbox_top, 2, 1, wadfile);
		temp = node.lbox_top;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.lbox_bottom, 2, 1, wadfile);
		temp = node.lbox_bottom;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.lbox_left, 2, 1, wadfile);
		temp = node.lbox_left;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.lbox_right, 2, 1, wadfile);
		temp = node.lbox_right;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&node.rchild, 2, 1, wadfile);
		Swap2_2Bytes(node.rchild); 
		fwrite(&buf1, 1, 2, lumpfile);
		fwrite(&node.rchild, 2, 1, lumpfile);
		fread(&node.lchild, 2, 1, wadfile);
		Swap2_2Bytes(node.lchild); 
		fwrite(&buf1, 1, 2, lumpfile);
		fwrite(&node.lchild, 2, 1, lumpfile);
		i++;
	} while (i < entry_count);
	fclose (lumpfile);
}

void CreateSectorsLump (FILE *wadfile, char *outfilename, int lump_offset, int lump_size)
{
	struct {
		short floor_height, ceil_height;
		char floor_texture[8], ceil_texture[8];
		short light;
		short type;
		short tag;
	} sector;

	FILE *lumpfile, *flats;
	int i, index, entry_count, lump_count, temp, filesize;
	char buf[10000];
	char *str;

	lumpfile = fopen (outfilename, "wb");
	if (!lumpfile) {
		printf ("\nLump file %s can't create", outfilename);
		getch();
	}
	flats = fopen("flats.txt", "r");
	if (!flats) {
		printf("\nflats.txt can't open");
		getch();
	}
	fseek(flats, 0, SEEK_END);
	filesize = ftell(flats);
	rewind(flats);
	i = 0;
	while (!feof(flats)) {
		buf[i] = fgetc(flats);
		i++;
	}
	fclose(flats);
	fseek (wadfile, lump_offset, SEEK_SET);
	entry_count = lump_size / sizeof (sector);
	Swap4Bytes(entry_count);
	fwrite(&entry_count, 4, 1, lumpfile);
	Swap4Bytes(entry_count);
	i = 0;
	do {
		fread(&sector.floor_height, 2, 1, wadfile);
		temp = sector.floor_height;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&sector.ceil_height, 2, 1, wadfile);
		temp = sector.ceil_height;
		Swap2_2Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&sector.floor_texture, 1, 8, wadfile);
		if (strlen(sector.floor_texture) > 8) sector.floor_texture[8] = 0;
	    strupr(sector.floor_texture);
		str = strstr(buf, sector.floor_texture);
		index = (str-buf) / 9;
		index = (str-buf) / 9;
		Swap4Bytes(index);
		fwrite(&index, 4, 1, lumpfile);
		fread(&sector.ceil_texture, 1, 8, wadfile);
		if (strlen(sector.ceil_texture) > 8) sector.ceil_texture[8] = 0;
	    strupr(sector.ceil_texture);
		str = strstr(buf, sector.ceil_texture);
		if (sector.ceil_texture[0] == 'F' && sector.ceil_texture[1] == '_' && sector.ceil_texture[2] == 'S' && sector.ceil_texture[3] == 'K' && sector.ceil_texture[4] == 'Y' && sector.ceil_texture[5] == '1' ) index = -1;
		else index = (str-buf) / 9;
		Swap4Bytes(index);
		fwrite(&index, 4, 1, lumpfile);
		fread(&sector.light, 2, 1, wadfile);
		temp = sector.light;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&sector.type, 2, 1, wadfile);
		temp = sector.type;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		fread(&sector.tag, 2, 1, wadfile);
		temp = sector.tag;
		Swap4Bytes(temp); 
		fwrite(&temp, 4, 1, lumpfile);
		i++;
	} while (i < entry_count);
	fclose (lumpfile);
}

void CreateRejectLump (FILE *wadfile, char *outfilename, int lump_offset, int lump_size)
{
	FILE *lumpfile;
	char buf[1000000];	
	lumpfile = fopen (outfilename, "wb");
	if (!lumpfile) {
		printf ("\nLump file %s can't create", outfilename);
		getch();
	}
	fseek (wadfile, lump_offset, SEEK_SET);
	fread(&buf, 1, lump_size, wadfile);
	fwrite(&buf, 1, lump_size, lumpfile);
	fclose (lumpfile);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char *argv[])
{
	FILE *fp, *wadfile, *BlockmapFile;
	fpos_t pos; 
	char *infilename, *outfilename, *fname;
	char lumpname[8];
	int buffer, lump_count, lump_size, table_offset, lump_offset;
	short temp;

struct {
		short tmp;
	} tmpVal;

//argc=2;
#if 0
if (argc==1) {
//		cout << argc << "\n";
		cout << "No parameters found.\n\n";
		cout << "Usage: BlockMapWAD.exe InFile\n";
		cin.get();
		return 1;
	} else cout << argv << "\n";
#endif;

//infilename = argv[1];

	if (argc!=2) {
			cout << "Usage: Program inputfile\n";
			cout << "Press the enter key to continue ...";
			 cin.get();
		return 1;
	}
	Buffer = (Byte *)malloc(BUFFER_SIZE);
	if (!Buffer) {
		printf("# Not enough memory for buffer!\n");
		return 1;
	}

	StrArray = (String_t *)malloc(BUFFER_SIZE);
	if (!StrArray) {
		printf("# Not enough memory for StrArray buffer!\n");
		return 1;
	}


	Header = (PCHeader_t *)malloc(BUFFER_SIZE);
	if (!Header) {
		printf("# Not enough memory for PCHeader buffer!\n");
		return 1;
	}
#if 1;
	DOHeader = (DOHeader_t *)malloc(BUFFER_SIZE);
	if (!DOHeader) {
		printf("# Not enough memory for 3DOHeader buffer!\n");
		return 1;
	}
#endif;


	Offset = (Offset_t *)malloc(BUFFER_SIZE);
	if (!Offset) {
		printf("# Not enough memory for Offset buffer!\n");
		return 1;
	}

	BlockLine = (DOLine *)malloc(BUFFER_SIZE);
		if (!BlockLine) {
		free(Buffer);
		printf("# Not enough memory for BlockLine buffer!\n");
		return 1;
	}
/*-------------------------------------------------------------------------------------------*/
#if 1
//infilename = "map01.wad";

//argv[1] = "map02.wad";

infilename = argv[1];
//wadfile = fopen(infilename,"rb");
wadfile = fopen(argv[1],"rb");
BlockmapFile = fopen("BLOCKMAP.lmp","wb");


fseek (wadfile, 4, SEEK_SET);
	fread (&lump_count, 4, 1, wadfile);
//	cout << lump_count << "\n";
	fread (&table_offset, 4, 1, wadfile);
//	cout << table_offset << "\n";
	fseek (wadfile, table_offset, SEEK_SET);
	do {
		fread(&lump_offset, 4, 1, wadfile);
//		cout << lump_offset << "\n";
		fread(&lump_size, 4, 1, wadfile);
//		cout << lump_size << "\n";
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
	fgetpos(wadfile, &pos);
	fname = infilename;
//	outfilename = "rMAP01_lump1";
//------------------------------------- Зададим имя файла лумпа ---------------------//
#if 1;
	char tmpstr2[20]={0}, tmpstr3[20]={0};
	char *tmpstr, *NamePart, *NamePart2, *NamePart3, *NamePart4, *NamePart5;
//	char *NamePart6, *NamePart7, *NamePart8, *NamePart9, *NamePart10;
	strncpy(tmpstr2, argv[1], 5); //скопировать 5 символов строки argv[1] в строку tmpstr2. Получилось "MAP01"
	tmpstr3[0] = 114;	// "r" symbol
	strcat(tmpstr3, tmpstr2);				// Получилось "rMAP01"
	outfilename = strcat(tmpstr3, "_lump1");	// Получилось "rMAP01_lump1"
	NamePart = strncpy(tmpstr2, tmpstr3, 6);
//	NamePart2 = NamePart3 = NamePart4 = NamePart5 = NamePart6 = NamePart7 = NamePart8 = NamePart9 = NamePart10 = NamePart;

#endif;
//-----------------------------------------------------------------------------------//
	infilename = fname;
	CreateThingsLump (wadfile, outfilename, lump_offset, lump_size);

	fsetpos(wadfile, &pos);
	do {
		fread(&lump_offset, 4, 1, wadfile);
		fread(&lump_size, 4, 1, wadfile);
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
	fgetpos(wadfile, &pos);
//	outfilename = "rMAP01_lump2";
//	strncpy(tmpstr2, tmpstr3, 6);
	outfilename = strcat(NamePart, "_lump2");
	CreateLinedefsLump (wadfile, outfilename, lump_offset, lump_size);

	fsetpos(wadfile, &pos);
	do {
		fread(&lump_offset, 4, 1, wadfile);
		fread(&lump_size, 4, 1, wadfile);
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
	fgetpos(wadfile, &pos);
//	outfilename = "rMAP01_lump3";
////
char tmpstr4[20]={0}, tmpstr5[20]={0};
	strncpy(tmpstr4, argv[1], 5);
	tmpstr5[0] = 114;	// "r" symbol
	strcat(tmpstr5, tmpstr4);				// Получилось "rMAP01"
	outfilename = strcat(tmpstr5, "_lump3");
/////
	CreateSideDefsLump(wadfile, outfilename, lump_offset, lump_size);

	fsetpos(wadfile, &pos);
	do {
		fread(&lump_offset, 4, 1, wadfile);
		fread(&lump_size, 4, 1, wadfile);
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
	fgetpos(wadfile, &pos);
//	outfilename = "rMAP01_lump4";
////
char tmpstr6[20]={0}, tmpstr7[20]={0};
	strncpy(tmpstr6, argv[1], 5);
	tmpstr7[0] = 114;	// "r" symbol
	strcat(tmpstr7, tmpstr6);				// Получилось "rMAP01"
	outfilename = strcat(tmpstr7, "_lump4");
/////
	CreateVertexesLump(wadfile, outfilename, lump_offset, lump_size);

	fsetpos(wadfile, &pos);
	do {
		fread(&lump_offset, 4, 1, wadfile);
		fread(&lump_size, 4, 1, wadfile);
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
	fgetpos(wadfile, &pos);
//	outfilename = "rMAP01_lump5";
////
char tmpstr8[20]={0}, tmpstr9[20]={0};
	strncpy(tmpstr8, argv[1], 5);
	tmpstr9[0] = 114;	// "r" symbol
	strcat(tmpstr9, tmpstr8);				// Получилось "rMAP01"
	outfilename = strcat(tmpstr9, "_lump5");
/////
	CreateSegsLump(wadfile, outfilename, lump_offset, lump_size);

	fsetpos(wadfile, &pos);
	do {
		fread(&lump_offset, 4, 1, wadfile);
		fread(&lump_size, 4, 1, wadfile);
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
	fgetpos(wadfile, &pos);
//	outfilename = "rMAP01_lump6";
////
char tmpstr10[20]={0}, tmpstr11[20]={0};
	strncpy(tmpstr10, argv[1], 5);
	tmpstr11[0] = 114;	// "r" symbol
	strcat(tmpstr11, tmpstr10);				// Получилось "rMAP01"
	outfilename = strcat(tmpstr11, "_lump6");
/////
	CreateSubSectorsLump(wadfile, outfilename, lump_offset, lump_size);

	fsetpos(wadfile, &pos);
	do {
		fread(&lump_offset, 4, 1, wadfile);
		fread(&lump_size, 4, 1, wadfile);
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
	fgetpos(wadfile, &pos);
//	outfilename = "rMAP01_lump8";
////
char tmpstr12[20]={0}, tmpstr13[20]={0};
	strncpy(tmpstr12, argv[1], 5);
	tmpstr13[0] = 114;	// "r" symbol
	strcat(tmpstr13, tmpstr12);				// Получилось "rMAP01"
	outfilename = strcat(tmpstr13, "_lump8");
/////
	CreateNodesLump(wadfile, outfilename, lump_offset, lump_size);  // Может, поменять название на CreateSectorsLump?

	fsetpos(wadfile, &pos);
	do {
		fread(&lump_offset, 4, 1, wadfile);
		fread(&lump_size, 4, 1, wadfile);
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
	fgetpos(wadfile, &pos);
//	outfilename = "rMAP01_lump7";
////
char tmpstr14[20]={0}, tmpstr15[20]={0};
	strncpy(tmpstr14, argv[1], 5);
	tmpstr15[0] = 114;	// "r" symbol
	strcat(tmpstr15, tmpstr14);				// Получилось "rMAP01"
	outfilename = strcat(tmpstr15, "_lump7");
/////
	CreateSectorsLump(wadfile, outfilename, lump_offset, lump_size);	// Может, поменять название на CreateNodesLump?

	fsetpos(wadfile, &pos);
	do {
		fread(&lump_offset, 4, 1, wadfile);
		fread(&lump_size, 4, 1, wadfile);
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
	fgetpos(wadfile, &pos);
//	outfilename = "rMAP01_lump9";
////
char tmpstr16[20]={0}, tmpstr17[20]={0};
	strncpy(tmpstr16, argv[1], 5);
	tmpstr17[0] = 114;	// "r" symbol
	strcat(tmpstr17, tmpstr16);				// Получилось "rMAP01"
	outfilename = strcat(tmpstr17, "_lump9");
/////
	CreateRejectLump(wadfile, outfilename, lump_offset, lump_size);

	fsetpos(wadfile, &pos);
	do {
		fread(&lump_offset, 4, 1, wadfile);
		fread(&lump_size, 4, 1, wadfile);
		fread(&lumpname, 8, 1, wadfile);
		cout << lumpname << "\n";
	} while (lump_size == 0);
//	outfilename = "rMAP01_lump10";
	filenamearg = argv[1];
//	CreateBlockmapLump(wadfile, outfilename, lump_offset, lump_size);
fseek (wadfile, lump_offset, SEEK_SET);

int i;
for (i=0; i<lump_size/2; i++) {
fread(&tmpVal.tmp, 2, 1, wadfile);
temp = tmpVal.tmp;
fwrite(&temp, 2, 1, BlockmapFile);
}

fclose(BlockmapFile);
fclose(wadfile);
#endif;
/*----------------------------------------------------------------------------------------------------*/

    fp = fopen(InFileName,"rb");   
	if (!fp) {
		printf("# Can't open BLOCKMAP.lmp file %s.\n",argv[1]);    /* Oh oh */
		free(Buffer);
		return 0;
	}
	fclose(fp);     /* Close the file */



	InitVars();
	MakeHeader();
	CalcOffsets();
	MakeFinal();

	free(Buffer);
	free(StrArray);
	free(Header);
	free(DOHeader);

///////////// ------------- Чистим за собой --------------------///////////////////
remove("tmpblocklists");
remove("tmpheader");
remove("tmpblocklists_FF");
remove("offsets");
remove("blocklists_file");
remove("off_sorted");
remove("off_sorted_swaped");
remove("temp");
remove("BLOCKMAP.lmp");
	cout << "Press the enter key to continue ...";
    cin.get();
    return 0;
}
