%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "minipy-lab.tab.h"  /*用yacc编译yacc.y后生成的C头文件，内含%token、YYSTYPE、yylval等定义，供lex.yy.c和yacc.tab.c使用*/

//extern union yylval;

/*  any C declaration  */
%}

letter    [a-zA-Z]

digit     [0-9]

number   {digit}+

id       {letter}({letter}|{digit})*

real     {number}\.{number}?|{number}?\.{number}

%%
[ \t]+      {/*do nothing , just skip */}
{number}    { 
              yylval.val=atoi(yytext);
              return INT; //向yacc返回: 识别出的记号类型是INT
            }
{real}      {
              yylval.fval=atof(yytext);
              return REAL;
            }
{id}        {
              strcpy((char*)yylval.name, (const char*)yytext);  //符号表查询或者创建
              return ID; 
            }
\"(\\.|[^\\"])*\"	{               // 匹配字符串 "..\x..??"   
              yylval.str = (string *)_malloc(_STRING_LTERAL); int* addr = (int *)yylval.str; ((string *)addr)->assign(yytext,1,yyleng-2);
              return(STRING_LTERAL); 
                  }
.|\n        { return yytext[0];}
%%

/* yytext是lex内部已经定义好的指针变量,yytext总是指向当前获得匹配的字符串,此外yyleng是当前获得匹配的字符串长度 */

 /*int yywrap()
 { return 1; }*/
