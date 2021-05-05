%{
   /* definition */
   #include "minipython.c"
   #include "lex.yy.c"    //lex.yy.c和yacc的输出一起编译
%}

%union{
        int val;
        float fval;
        char name[MAX_IDLEN + 1];
        string* str;
        atm vatm;  //struct{name, type, addr}
	atm_e vatm_e;
        objtab obj; //struct{type, addr}
        asgninfo asgn;
	para* stk;
}

%token <name> ID       /*lex里要return的记号的声明*/
%token <val> INT 
%token <fval> REAL 
%token <str> STRING_LTERAL 

%type <asgn> assignExpr

%type <vatm> atom
%type <vatm_e> atom_expr

%type <obj> stat
%type <obj> List_items
%type <obj> List
%type <obj> number
%type <obj> factor
%type <obj> add_expr
%type <obj> mul_expr
%type <obj> sub_expr
%type <obj> slice_op

%type <stk> arglist

%type <val> opt_comma  //可否不定义？

%%
Start : prompt Lines
      ;
Lines : Lines  stat '\n' prompt
      | Lines  '\n' prompt
      | 
      | error '\n' {yyerrok; err = 0; }
      ;
prompt : {cout << "miniPy> ";}
       ;
stat  : assignExpr {if($1.mode == _PRT) {print(push_stack(0, $1.addr, $1.type), $$.type);}/*else do nothing*/ cite_record($$.addr, $$.type, 0);}
      ;
assignExpr:
        atom_expr '=' assignExpr    {$$.addr = tab_assign($1.name, $1.type, $1.addr, $3.type, $3.addr, $1.slice); $$.type = $3.type; $$.mode = _ASGN; cite_record($$.addr, $$.type, 1); if(err == 1) YYERROR;}
      | add_expr                    {$$.addr = $1.addr; $$.type = $1.type; $$.mode = _PRT; if(err == 1) YYERROR;}
      ;
number : INT                    {$$.addr = _malloc(_INT); *($$.addr) = $1; $$.type = _INT;}
       | REAL                   {$$.addr = _malloc(_REAL); *(float *)($$.addr) = $1; $$.type = _REAL;}
       ;
factor : '+' factor             {$$.addr = pos_obj($2.type, $2.addr); $$.type = $2.type; if(err == 1) YYERROR;}
       | '-' factor             {$$.addr = neg_obj($2.type, $2.addr); $$.type = $2.type; if(err == 1) YYERROR;}
       | atom_expr              {$$.addr = judge_tab($1.name, $1.addr, $1.type, $$.type, $1.offset, $1.slice); if($$.type == _R_INT) $$.type = _INT; if(err == 1) YYERROR;}  //在此时可以判断是引用而不会是赋值, 在此之后name不再重要
       ; 
atom  : ID                        {symlook($1,1); strcpy($$.name, $1); $$.addr = symbol_table[$1].addr; $$.type = symbol_table[$1].type; cite_record($$.addr, $$.type, 1); if(symbol_table[$1].funcptr){$$.funcptr = symbol_table[$1].funcptr;} $$.offset = 0;}
      | STRING_LTERAL             {$$.name[0] = '\0'; $$.addr = (int*)$1; $$.type = _STRING_LTERAL; $$.offset = 0;}
      | List                      {$$.name[0] = '\0'; $$.addr = $1.addr; $$.type = $1.type; $$.offset = 0;}
      | number                    {$$.name[0] = '\0'; $$.addr = $1.addr; $$.type = $1.type; $$.offset = 0;}
      ;
slice_op :  /*  empty production */     {$$.type = _NONE; $$.addr = NULL;}    //类型：float型怎么办？
        | ':' add_expr                  {if($2.type == _NONE) {$$.addr = NULL;} else $$.addr = $2.addr; $$.type = $2.type;  if(*($$.addr) == 0) {yyerror("slice cannot be zero"); YYERROR;}}
        ;
sub_expr:  /*  empty production */      {$$.type = _NONE; $$.addr = NULL;}
        | add_expr                      {if($1.type == _NONE) {$$.addr = NULL;} else $$.addr = $1.addr; $$.type = $1.type;}
        ;        
atom_expr : atom                                               {strcpy($$.name, $1.name); $$.addr = $1.addr; $$.type = $1.type; $$.offset = $1.offset; $$.funcptr = $1.funcptr; $$.slice[2] = 0; }
        | atom_expr  '[' sub_expr  ':' sub_expr  slice_op ']'  {strcpy($$.name, $1.name); if($$.offset && $1.type != _CHAR) $$.addr = ((item*)($1.addr))->addr;
                                                                else $$.addr = $1.addr; $$.type = $1.type; $$.offset = 0; slice_def($3.type, $3.addr, $5.type, $5.addr, $6.type, $6.addr, $$.type, $$.addr, $$.slice); if(err == 1) YYERROR; }
        | atom_expr  '[' add_expr ']'                          {strcpy($$.name, $1.name); if($$.offset && $1.type != _CHAR) $$.addr = index_visit($1.type, ((item*)($1.addr))->addr, $3.type, $3.addr, $$.type); 
                                                                else {$$.addr = index_visit($1.type, $1.addr, $3.type, $3.addr, $$.type);} $$.offset = 1; if($$.type == _CHAR || $$.type == _R_INT){$$.offset = 0;}
$$.slice[2] = 0; if(err == 1) YYERROR;}
        | atom_expr '.'ID       {if(symbol_table[$3].funcptr) {if($$.offset) $$.addr = ((item*)($1.addr))->addr; else $$.addr = $1.addr; $$.funcptr = symbol_table[$3].funcptr; $$.type = $1.type;} else yyerror("not a function"); $$.offset = 0; $$.slice[2] = 0; if(err == 1) YYERROR;}
        | atom_expr  '(' arglist opt_comma ')'   {if($$.offset) $$.addr = $1.funcptr(push_stack($3, ((item*)($1.addr))->addr, $1.type), $$.type); else $$.addr = $1.funcptr(push_stack($3, $1.addr, $1.type), $$.type);  $$.name[0] = '\0'; $$.slice[2] = 0; if(err == 1) YYERROR;}
        | atom_expr  '('  ')'  {if(!strcmp($1.name,"quit")) {cite_record(0,0,2); return 0;} else{ yyerror("function name is not defined"); YYERROR;} $$.slice[2] = 0;  }
        ;
arglist : add_expr                  {$$ = push_stack(0, $1.addr, $1.type);}
        | arglist ',' add_expr      {$$ = push_stack($1, $3.addr, $3.type);}
        ;      
List  : '[' ']'                           {$$.addr = add_list(0,0,-1); $$.type = _LIST; if(err == 1) YYERROR;}
      | '[' List_items opt_comma ']'      {$$.addr = $2.addr; $$.type = _LIST;}
      ;
opt_comma : /*  empty production */   {$$ = 0;}
          | ','    {$$ = 0;}
          ;
List_items   
      : add_expr                          {$$.addr = add_list(0, $1.addr, $1.type); $$.type = _LIST; if(err == 1) YYERROR;}
      | List_items ',' add_expr           {$$.addr = add_list($1.addr, $3.addr, $3.type); $$.type = _LIST; if(err == 1) YYERROR;}
      ;
add_expr : add_expr '+' mul_expr           {$$.addr = add_obj($1.type, $1.addr, $3.type, $3.addr, $$.type); if(err == 1) YYERROR;}
	      |  add_expr '-' mul_expr     {$$.addr = sub_obj($1.type, $1.addr, $3.type, $3.addr, $$.type); if(err == 1) YYERROR;}
	      |  mul_expr                  {$$.addr = $1.addr; $$.type = $1.type;}
        ;
mul_expr : mul_expr '*' factor       {$$.addr = mul_obj($1.type, $1.addr, $3.type, $3.addr, $$.type); if(err == 1) YYERROR;}
        |  mul_expr '/' factor       {$$.addr = div_obj($1.type, $1.addr, $3.type, $3.addr, $$.type); if(err == 1) YYERROR;}
	|  mul_expr '%' factor       {$$.addr = rem_obj($1.type, $1.addr, $3.type, $3.addr, $$.type); if(err == 1) YYERROR;}
        |  factor                    {$$.addr = $1.addr; $$.type = $1.type;}  //不需要重新分配空间
        ;

%%

int main()
{
   addfunc("print", print);
   addfunc("append", append);
   addfunc("len", len);
   addfunc("list", make_list);
   addfunc("range", range);
   return yyparse(); /*使yacc开始读取输入和解析，它会调用lex的yylex()读取记号*/
}

int yywrap()
{ return 1; }        		    
