#ifndef MINIPYTHON_H

#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <cmath>

using namespace std;

#define MAX_TABLEN 500	//符号表最大长度
#define MAX_NUMLEN 14   //数字（int、real）最大长度 （位数）
#define MAX_IDLEN 10	//变量标识符最大长度

/* 类型 */
enum symtype
{
	_NONE,
	_INT,
	_REAL,
	_ITEM,
	_LIST,
	_CHAR,
	_STRING_LTERAL,
 	_RANGE,
	_R_INT
}; 

enum modetype
{
	_PRT,
	_ASGN
}; 

struct element{   //列表元素
	int type;
	int *addr;
	element(int para1, int* para2){type = para1; addr = para2;}
	element(){type = -1, addr = NULL;}
};
typedef struct element item;
typedef stack <item> para;

/* 符号表 */
typedef struct
{
	char name[MAX_IDLEN + 1]; 
	int type;
	int *addr;		//引用，指针值，指向对象首地址
	int offset;  //偏移 
	int* ((*funcptr)(para*, int&));
} atm; 

typedef struct
{
	char name[MAX_IDLEN + 1]; 
	int type;
	int *addr;		//引用，指针值，指向对象首地址
	int offset;  //偏移 
	int slice[3];
	int* ((*funcptr)(para*, int&));
} atm_e; 

/*常量对象*/
typedef struct
{
	int type;
	int *addr;
	int* ((*funcptr)(para*, int&));		
} objtab; 

typedef struct{
	int type;
	int *addr;		//引用，指针值，指向对象首地址 
	int mode;  //模式，判断是id，计算模式还是赋值表达式
} asgninfo;

typedef struct{
	int *count;
	int type;
} tab;

typedef map <string, objtab> symtab;   //符号表
typedef vector <item> list;
typedef map <int*, tab> cite;

symtab symbol_table;   //预先给符号表分配空间
cite cite_table;  //引用表

int err = 0;  //flag

int* _malloc(int type);
void cite_record(int* addr, int type, int op);
int symlook(char name[], int op);	//在符号表中查找name，返回索引，用map则返回状态
int CreateIndex(char name[]);  //给定ID则插入条目
int* judge_tab(char name[], int type1, int *addr1, int type2, int *addr2, int *slice);  //判断符号表中ID是否有真实地址
int* tab_assign(char name[], int *addr1, int type, int *addr2, int *slice);   //符号表赋值
void DeleteIndex(char name[]); 
int* index_visit(int type1, int *addr, int type2, int *index, int &type3);
void content_copy(int **addr1, int *addr2, int type); 

int* pos_obj(int type, int *addr);
int* neg_obj(int type, int *addr);

int* add_obj(int type1, int *addr1, int type2, int *addr2, int &type);
int* sub_obj(int type1, int *addr1, int type2, int *addr2, int &type);
int* mul_obj(int type1, int *addr1, int type2, int *addr2, int &type);
int* div_obj(int type1, int *addr1, int type2, int *addr2, int &type);
int* rem_obj(int type1, int *addr1, int type2, int *addr2, int &type);

int* add_list(int *addr1, int *addr2, int type);

void print_obj(para *addr);
int* print(para *addr, int &type);
void slice_def(int type1, int *addr1, int type2, int *addr2, int type3, int *addr3, int type, int *src, int *p);
int* slice_obj(int type, int *addr, int *s, int *e, int *op);

void addfunc(const char *name, int* (*funcptr)(para*, int&));
para* push_stack(para *addr1, int *addr2, int type);
int* append(para *addr, int &type);
int* range(para *addr, int &type);
int* len(para *addr, int &type);
int* make_list(para *addr, int &type);

void yyerror(const char *s);

/* minipython_h */ 

#endif
