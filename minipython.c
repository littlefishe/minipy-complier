#include "minipython.h"
#define ERROR err=1

int* _malloc(int type){
	switch(type){
		case _INT: {int *p = new int; cite_record((int*)p, _INT, 1); return (int*)p;}
		case _REAL:{float *p = new float; cite_record((int*)p, _REAL, 1); return (int*)p;}
	    case _LIST: {list *p = new list; cite_record((int*)p, _LIST, 1); return (int*)p;}
		case _STRING_LTERAL: {string *p = new string; cite_record((int*)p, _STRING_LTERAL, 1); return (int*)p;}
		case _RANGE: {int *p = new int(3); cite_record((int*)p, _RANGE, 1); return (int*)p;}
		default: return NULL;
	}
	return NULL;
}

void cite_record(int* addr, int type, int op){  //op=1,cite+1 op=0,cite-1
	if(addr){
		if(op == 1){
			cite_table[addr].type = type;
			cite_table[addr].count++;
		}
		else if(op == 0){
			cite::iterator iter = cite_table.find(addr);
			if(iter == cite_table.end())
				return;
			cite_table[addr].count--;
			if(cite_table[addr].count == 0){
				switch(cite_table[addr].type){
					case _INT:	delete addr; break;
					case _REAL:	delete (float*)addr; break;
					case _STRING_LTERAL:  delete (string*)addr; break;
					case _ITEM:	delete (item*)addr; break;
					case _LIST:	delete (list*)addr; break;
					case _RANGE:    delete [] addr; break;
					default: break;
				}
			}
			}
	}
	else if(op == 2){
		cite::iterator i;
		for(i = cite_table.begin(); i!= cite_table.end(); ++i)
			if(i->second.count > 0){
					switch(i->second.type){
						case _INT:	delete i->first; break;
						case _REAL:	delete (float*)(i->first); break;
						case _STRING_LTERAL:  delete (string*)(i->first); break;
						case _ITEM:	delete (item*)(i->first); break;
						case _LIST:	delete (list*)(i->first); break;
						case _RANGE:    delete [] (i->first); break;
						default: break;
				}
			}
	}
}

int symlook(char name[], int op){
    if(name){
		symtab::iterator iter;
    	iter = symbol_table.find(name);
    	if(iter!=symbol_table.end())
       		return 1;
  	  	else if(op == 1)  //op指示是否需要创建
        	if(CreateIndex(name))
				return 1;
		else return 0;
	}
	return 0;
}

void content_copy(int **addr1, int *addr2, int type){
	switch(type){
		case _INT:  **addr1 = *addr2; cite_record(addr2, 0, 0); break;
		case _REAL: *(float*)(*addr1) = *(float*)addr2; cite_record(addr2, 0, 0); break;
	    case _LIST: *addr1 = addr2; break;   //浅拷贝
		case _STRING_LTERAL: *(string *)(*addr1) = *(string *)addr2; cite_record(addr2, 0, 0); break;  
		case _RANGE: *(*addr1) = *(addr2); *(*addr1 + 1) = *(addr2 + 1); *(*addr1 + 2) = *(addr2 + 2); cite_record(addr2, 0, 0); break;
		default: return;
	}
}

int* tab_assign(char name[], int type1, int *addr1, int type2, int *addr2, int *slice){
	int *p;
	int t;
	if(symlook(name, 0)){
		if(slice[2]!=0){
			if(type1 == _LIST && type2 == _LIST){
				int len = ((list *)addr1)->size();
				int rs, re;

				rs = *(slice);
				re = *(slice + 1);

				if(re < -len-1)
					re = -1;
				else if(re < 0)
					re = len + re;
				else if(re >= len)
					re = len;
				rs = (rs >= 0 ? rs : len + rs);
				rs = (rs <= -len-1 ? 0 : rs);
				rs = (rs >= len ? len : rs);

				if(*(slice+2) == 1){  //normal case: pace = 1 and end can be less than start
					((list *)addr1)->erase(((list *)addr1)->begin() + rs, ((list *)addr1)->begin() + re);
					((list *)addr1)->insert(((list *)addr1)->begin() + rs, ((list *)addr2)->begin(), ((list *)addr2)->end());
				}
				else {

					if(rs < re && *(slice + 2) > 0){
						if((re - rs - 1) / *(slice + 2) + 1 != ((list *)addr2)->size()) {yyerror("ValueError: attempt to assign sequence to extended slice"); ERROR; return NULL;}
						int j = 0;
						for(int i = rs; i < re; i += *(slice + 2))
							(*(list *)addr1)[i] = (*(list *)addr2)[j++]; 
					}
					else if(rs > re && *(slice + 2) < 0){
						if((rs - re - 1) / (-*(slice + 2)) + 1 != ((list *)addr2)->size()) {yyerror("ValueError: attempt to assign sequence to extended slice"); ERROR; return NULL;}
						int j = 0;
						for(int i = rs; i > re; i += *(slice + 2))
							(*(list *)addr1)[i] = (*(list *)addr2)[j++]; 
					}
					else {
						if(((list *)addr2)->size() !=0) {yyerror("ValueError: attempt to assign sequence to extended slice of size 0"); ERROR; return NULL;}
					}
				}//
				return addr2;
			}
			else if(type1 == _LIST && type2 == _STRING_LTERAL){
				p = make_list(push_stack(push_stack(0, addr2, type2), 0, 0), t);  //additional push that will be popped in make_list()
				int len = ((list *)addr1)->size();
				int rs, re;

				rs = *(slice);
				re = *(slice + 1);

				if(re < -len-1)
					re = -1;
				else if(re < 0)
					re = len + re;
				else if(re >= len)
					re = len;
				rs = (rs >= 0 ? rs : len + rs);
				rs = (rs <= -len-1 ? 0 : rs);
				rs = (rs >= len ? len : rs);

				if(*(slice+2) == 1){  //normal case: pace = 1 and end can be less than start
					((list *)addr1)->erase(((list *)addr1)->begin() + rs, ((list *)addr1)->begin() + re);
					((list *)addr1)->insert(((list *)addr1)->begin() + rs, ((list *)p)->begin(), ((list *)p)->end());
				}
				else {
					
					if(rs < re && *(slice + 2) > 0){
						if((re - rs - 1) / *(slice + 2) + 1 != ((list *)p)->size()) {yyerror("ValueError: attempt to assign sequence to extended slice"); ERROR; return NULL;}
						int j = 0;
						for(int i = rs; i < re; i += *(slice + 2))
							(*(list *)addr1)[i] = (*(list *)p)[j++]; 
					}
					else if(rs > re && *(slice + 2) < 0){
						if((rs - re - 1) / (-*(slice + 2)) + 1 != ((list *)p)->size()) {yyerror("ValueError: attempt to assign sequence to extended slice"); ERROR; return NULL;}
						int j = 0;
						for(int i = rs; i > re; i += *(slice + 2))
							(*(list *)addr1)[i] = (*(list *)p)[j++]; 
					}
					else {
						if(((list *)p)->size() !=0) {yyerror("ValueError: attempt to assign sequence to extended slice of size 0"); ERROR; return NULL;}
					}

				}//
				return p;
			}
			else if(type1 == _LIST && type2 == _RANGE){
				p = make_list(push_stack(push_stack(0, addr2, type2), 0, 0), t);  //additional push that will be popped in make_list()
				int len = ((list *)addr1)->size();
				int rs, re;

				rs = *(slice);
				re = *(slice + 1);

				if(re < -len-1)
					re = -1;
				else if(re < 0)
					re = len + re;
				else if(re >= len)
					re = len;
				rs = (rs >= 0 ? rs : len + rs);
				rs = (rs <= -len-1 ? 0 : rs);
				rs = (rs >= len ? len : rs);

				if(*(slice+2) == 1){  //normal case: pace = 1 and end can be less than start
					((list *)addr1)->erase(((list *)addr1)->begin() + rs, ((list *)addr1)->begin() + re);
					((list *)addr1)->insert(((list *)addr1)->begin() + rs, ((list *)p)->begin(), ((list *)p)->end());
				}
				else {
					
					if(rs < re && *(slice + 2) > 0){
						if((re - rs - 1) / *(slice + 2) + 1 != ((list *)p)->size()) {yyerror("ValueError: attempt to assign sequence to extended slice"); ERROR; return NULL;}
						int j = 0;
						for(int i = rs; i < re; i += *(slice + 2))
							(*(list *)addr1)[i] = (*(list *)p)[j++]; 
					}
					else if(rs > re && *(slice + 2) < 0){
						if((rs - re - 1) / (-*(slice + 2)) + 1 != ((list *)p)->size()) {yyerror("ValueError: attempt to assign sequence to extended slice"); ERROR; return NULL;}
						int j = 0;
						for(int i = rs; i > re; i += *(slice + 2))
							(*(list *)addr1)[i] = (*(list *)p)[j++]; 
					}
					else {
						if(((list *)p)->size() !=0) {yyerror("ValueError: attempt to assign sequence to extended slice of size 0"); ERROR; return NULL;}
					}

				}//
				return p;
			}
			else if(type1 != _LIST) {yyerror("TypeError: object does not support item assignment"); ERROR; return NULL;}
			else {yyerror("TypeError: can only assign an iterable"); ERROR; return NULL;}
		}
		else if(symbol_table[name].addr == addr1){  //第一种情况：id = ...
			if(type2 == _CHAR){
				symbol_table[name].type = _STRING_LTERAL;
				cite_record(symbol_table[name].addr, 0, 0);
				symbol_table[name].addr = _malloc(_STRING_LTERAL);
				((string*)(symbol_table[name].addr))->push_back(*(char*)addr2);
				cite_record(addr2, 0, 0);
			}
			else{	
		    	symbol_table[name].type = type2;
				cite_record(symbol_table[name].addr, 0, 0);
				symbol_table[name].addr = _malloc(type2);   //分配空间后再复制内容，如果是list，已经在content_copy中处理
				content_copy(&(symbol_table[name].addr), addr2, type2);	
			}
			return symbol_table[name].addr;
		}
		else  //第二种情况：id[][]..= ...  这种情况不需要改符号表内容
		{
			if(symbol_table[name].type == _LIST){    //对list内元素的修改通常是可以的，但是存在问题：list的元素是string，id[][]不能修改，而id[]可以修改
				if(type1 == _CHAR)
					{yyerror("'str' object does not support item assignment"); ERROR; return NULL;}
				else if(type1 == _R_INT)
					{yyerror("'range' object does not support item assignment"); ERROR; return NULL;}
				else{
					cite_record(((item *)addr1)->addr, 0, 0);
					((item *)addr1)->addr = _malloc(type2);     //此时addr1为列表某元素的地址，而该元素的值为引用对象的地址             
					((item *)addr1)->type = type2;
					content_copy(&(((item *)addr1)->addr), addr2, type2);
				}
			}
			else {yyerror("object does not support item assignment"); ERROR; return NULL;} //addr和addr1不相等，如果不是list，那么只可能是string
			return ((item *)addr1)->addr;
		}

    }
	else 
		{yyerror("table assignment: object doesn't exist!"); ERROR; return NULL;} //这种情况应该不会发生
}

int CreateIndex(char name[]){ 
    if(name){
        objtab p;
        p.type = -1;
        p.addr = NULL;
	p.funcptr = NULL;
		symbol_table.insert(pair<string, objtab>(name,p));
		return 1;
	}
	return 0;
}

int *judge_tab(char name[], int *addr, int type1, int &type2, int offset, int* slice){
	int *p;
	if(name[0] != '\0'){
		if(!symbol_table[name].addr && type1!= _NONE){ 
			cite_record(symbol_table[name].addr, 0, 0);
			DeleteIndex(name); 
			yyerror("name is not defined!"); ERROR; return NULL;
		}
		else if(slice[2] != 0){
			p = slice_obj(type1, addr, slice, slice+1, slice+2);
			if(type1 == _CHAR)
				type2 = _STRING_LTERAL; 
			else type2 = type1;
		}
		else{
			if(offset){
				p = ((item*)addr)->addr;
				type2 = ((item*)addr)->type;
			}
			else{
				p = addr;
				type2 = type1;
			}

		}
	}
	else{ 		
		p = addr;//problem here!
		type2 = type1;
	}
	if(offset) cite_record(p, type1, 1);
	return p;
}

void DeleteIndex(char name[]){
	symtab::iterator iter = symbol_table.find(name);   //
	if(iter!=symbol_table.end())
		symbol_table.erase(iter);
}

int* index_visit(int type1, int *addr, int type2, int *index, int &type3){
	int *p = addr; 
	if(!p) 
		{yyerror("index visit error: object doesn't exist!"); ERROR; return NULL;}
	else {
		if(type2 != _INT) 
			{yyerror("list indices must be an integers or slices!"); ERROR; return NULL;}
		else if(type1 == _INT || type1 == _REAL || type1 == _NONE)
			{yyerror("object isn't suscriptable!"); ERROR; return NULL;} //
		else if(type1 == _LIST){
			int ri = *(index) >= 0 ? *(index) : ((list *)p)->size() + *(index); //index can be negative     
			if(ri >= ((list *)p)->size()  || ri < 0)  //out of range
				{yyerror("list index out of range"); ERROR; return NULL;}//over?
			type3 = (*(list *)p)[ri].type;
			p = (int *)&((*(list *)p)[ri]);  //offset problems here
		}
		else if(type1 == _STRING_LTERAL){
			int ri = *(index) >= 0 ? *(index) : ((string *)p)->size() + *(index);
			if(ri >= ((string *)p)->size()  || ri < 0)
				{yyerror("string index out of range"); ERROR; return NULL;}
			p = (int *)(&((*(string *)p)[ri]));//index visit start with 1th
			type3 = _CHAR;   //set char to strict the operation of string of size 1
		}
		else if(type1 == _CHAR){
			int ri = *(index) >= 0 ? *(index) : 1 + *(index);
			if(ri != 0)
				{yyerror("string index out of range"); ERROR; return NULL;}
			p = (int *)(&((char *)p)[ri]);
			type3 = _CHAR;   //把串的元素设置为_CHAR型，以对string元素的操作进行限制
		}
		else if(type1 == _RANGE){
			int len = (*(addr + 1) - *(addr) - 1) / *(addr+2) + 1 ;
			if(len < 0)
				len = 0;
			int ri = *(index) >= 0 ? *(index) : len + *(index);
			if(ri >= len || ri < 0)
				{yyerror("range object index out of range"); ERROR; return NULL;}
			int* p1 = _malloc(_INT);
			*p1 = *p + *(p+2) * ri;
			type3 = _R_INT;
			return p1;
		}
	}	
	return p;
}

int* add_list(int *addr1, int *addr2, int type){
	item p1;
	int *b;
	int *p2 = addr1;
	if(!p2){
		p2 = _malloc(_LIST);   //创建list
	}
	if(addr2 || type == _NONE){
		b = _malloc(type);
		content_copy(&b, addr2, type);
		p1.addr = b;
		p1.type = type;
		((list *)p2)->push_back(p1);
	}
	return p2;
}

int* pos_obj(int type, int *addr){
	if(type == _LIST || type == _STRING_LTERAL || type == _CHAR)
		{yyerror("unsupported operand type!"); ERROR; return NULL;}
	else{
		int *p = _malloc(type);
		*p = *addr;
		cite_record(addr, 0, 0);
		return p;
	}
}

int* neg_obj(int type, int *addr){
	if(type == _LIST || type == _STRING_LTERAL || type == _CHAR)
		{yyerror("unsupported operand type!"); ERROR; return NULL;}
	else{
		int *p = _malloc(type);
		*p = -(*addr);
		cite_record(addr, 0, 0);
		return p;
	}
}

int* add_obj(int type1, int *addr1, int type2, int *addr2, int &type){
	int *p;
	if(type1 != type2){
		if(type1 == _INT && type2 == _REAL){
			p = _malloc(_REAL); 
			*(float *)p = (float)*addr1 + *(float *)addr2;
			type = _REAL;
		}
		else if(type1 == _REAL && type2 == _INT){
			p = _malloc(_REAL); 
			*(float *)p = *(float *)addr1 + (float)*addr2;
			type = _REAL;
		}
		else {yyerror("unsupported operand type!"); ERROR; return NULL;}
	}  //剩下的情况type相同
	else if(type1 == _LIST){
		p = _malloc(_LIST);
		((list *)p)->insert(((list *)p)->end(), ((list *)addr1)->begin(), ((list *)addr1)->end());
		((list *)p)->insert(((list *)p)->end(), ((list *)addr2)->begin(), ((list *)addr2)->end());
		type = _LIST;
	}
	else if(type1 == _STRING_LTERAL || type1 == _CHAR){ //即便是char也可以直接通过+赋值
		p = _malloc(_STRING_LTERAL);
		if(type1 == _CHAR){
			*(string *)p += *(char *)addr1;  
			*(string *)p += *(char *)addr2;
		}
		else{
			*(string *)p += *(string *)addr1;  
			*(string *)p += *(string *)addr2;
		}

		type = _STRING_LTERAL;
	}

	else if(type1 == _INT){
		p = _malloc(_INT); 
		*p = *(addr1) + *(addr2);
		type = _INT;
	}
	else if(type1 == _REAL){
		p = _malloc(_REAL); 
		*(float *)p = *(float *)addr1 + *(float *)addr2;
		type = _REAL;
	}
	else 
		{yyerror("unsupported operand type!"); ERROR; return NULL;}
	cite_record(addr1, 0, 0);
	cite_record(addr2, 0, 0);
	
	return p;
}


int* sub_obj(int type1, int *addr1, int type2, int *addr2, int &type){
	int *p;
	if(type1 != type2){
		if(type1 == _INT && type2 == _REAL){
		p = _malloc(_REAL); 
		*(float *)p = (float)*addr1 - *(float *)addr2;
			type = _REAL;
		}
		else if(type1 == _REAL && type2 == _INT){
			p = _malloc(_REAL); 
			*(float *)p = *(float *)addr1 - (float)*addr2;
			type = _REAL;
		}
		else 
			{yyerror("unsupported operand type!"); ERROR; return NULL;} //
	}
	else {
		if(type1 == _INT){
			p = _malloc(_INT); 
			*p = *addr1 - *addr2;
			type = _INT;
		}
		else if(type1 == _REAL){
			p = _malloc(_REAL); 
			*(float *)p = *(float *)addr1 - *(float *)addr2;
			type = _REAL;
		}
		else
			{yyerror("unsupported operand type!"); ERROR; return NULL;} //
		}
	cite_record(addr1, 0, 0);
	cite_record(addr2, 0, 0);
	return p;
}

int* mul_obj(int type1, int *addr1, int type2, int *addr2, int &type){
	int *p;
	if(type1 == type2){
		if(type1 == _INT){
			p = _malloc(_INT); 
			*p = *addr1 * *addr2;
		}
		else if(type1 == _REAL){
			p = _malloc(_REAL); 
			*(float *)p = *(float *)addr1 * *(float *)addr2;
		}
	}
	else {
		if(type1 == _INT && type2 == _REAL){
		p = _malloc(_REAL); 
		*(float *)p = (float)*addr1 * *(float *)addr2;
			type = _REAL;
		}
		else if(type1 == _REAL && type2 == _INT){
			p = _malloc(_REAL); 
			*(float *)p = *(float *)addr1 * (float)*addr2;
			type = _REAL;
		}
		else if(type1 == _LIST && type2 == _INT){
			p = _malloc(_LIST);
			for(int i = 0; i < *addr2; ++i)
				((list *)p)->insert(((list *)p)->end(), ((list *)addr1)->begin(), ((list *)addr1)->end());
			type = _LIST;
		}
		else if(type2 == _LIST && type1 == _INT){
			p = _malloc(_LIST);
			for(int i = 0; i < *addr1; ++i)
				((list *)p)->insert(((list *)p)->end(), ((list *)addr2)->begin(), ((list *)addr2)->end());
			type = _LIST;
		}
		else if((type1 == _STRING_LTERAL || type1 == _CHAR) && type2 == _INT){
			p = _malloc(_STRING_LTERAL);
			for(int i = 0; i < *addr2; ++i){
				if(type1 == _CHAR)
					*(string *)p += *(char *)addr1;
				else{
					*(string *)p += *(string *)addr1;
				}	
			}
			type = _STRING_LTERAL;
		}
		else if((type2 == _STRING_LTERAL || type2 == _CHAR) && type1 == _INT){
			p = _malloc(_STRING_LTERAL);
			for(int i = 0; i < *addr1; ++i){
				if(type1 == _CHAR)
					*(string *)p += *(char *)addr2;
				else{
					*(string *)p += *(string *)addr2;
				}	
			}
			type = _STRING_LTERAL;
		}
		else
			{yyerror("unsupported operand type!"); ERROR; return NULL;}
	}
	cite_record(addr1, 0, 0);
	cite_record(addr2, 0, 0);
	return p;
}

int* div_obj(int type1, int *addr1, int type2, int *addr2, int &type){
	int *p;
	float a, b;
	if(type1 == _INT)
		a = (float)*addr1;
	else if(type1 == _REAL)
		a = *(float*)addr1;
	else {yyerror("unsupported operand type"); ERROR; return NULL;}//

	if(type2 == _INT)
		b = (float)*addr2;
	else if(type2 == _REAL)
		b = *(float*)addr2;
	else {yyerror("unsupported operand type"); ERROR; return NULL;}//

	if(b == 0)
		{yyerror("ZeroDivisionError: division by zero"); ERROR; return NULL;}//

	p = _malloc(_REAL); 
	*(float *)p = a/b;
	type = _REAL;
	cite_record(addr1, 0, 0);
	cite_record(addr2, 0, 0);
	return p;
}

int* rem_obj(int type1, int *addr1, int type2, int *addr2, int &type){
	int *p;
	float a, b;
	if(type1 == _INT)
		a = (float)*addr1;
	else if(type1 == _REAL)
		a = *(float*)addr1;
	else {yyerror("unsupported operand type"); ERROR; return NULL;}//

	if(type2 == _INT)
		b = (float)*addr2;
	else if(type2 == _REAL)
		b = *(float*)addr2;
	else {yyerror("unsupported operand type"); ERROR; return NULL;}//


	if(type1 == _INT && type2 == _INT){
		p = _malloc(_INT);
		*p = *addr1 % *addr2;
		type = _INT;
	}
	else {
		p = _malloc(_REAL);
		*(float *)p = fmod(a, b);
		type = _REAL;
	}
	cite_record(addr1, 0, 0);
	cite_record(addr2, 0, 0);
	return p;
}

void print_obj(para *addr){
	item src;
	if(addr->size() >= 2)
		addr->pop();
	while(!addr->empty()){
		src = addr->top();
		addr->pop();
		int *addr2 = src.addr;
		switch(src.type){
			case _INT:
				cout<<*addr2;
				break;
			case _REAL:
				cout<<*(float*)addr2;	
				break;
			case _STRING_LTERAL:
				cout<<"\""<<*(string *)addr2<<"\"";
				break;
			case _CHAR:
				cout<<"\""<<((char *)addr2)[0]<<"\"";  //只输出一个字符
				break;
			case _LIST:{
				list &v = *(list *)addr2;
				list::iterator i;
				cout<<"[";
				for(i = v.begin(); i!=v.end(); ++i){
					if(i->addr == addr2)
						cout<<"[...]";
					else if(i->type == _NONE){
						cout<<"None"; 
					}
					else{
						addr = push_stack(addr, i->addr, i->type);	
						print_obj(addr); 
					}
					if(i!=v.end()-1)
						cout<<",";
				}
				cout<<"]";
				break;
				}
			case _RANGE:
				cout<<"range(";
				cout<<*addr2<<","<<*(addr2+1);
				if(*(addr2+2)!=1)
					cout<<","<<*(addr2+2);
				cout<<")";
				break;
			case _NONE: break;
			default: yyerror("unrecognized type while printing"); ERROR; return; break;  //problems here!
		}
	}
}

int* print(para *addr, int &type){
	print_obj(addr);
	if(type)
		type = _NONE;
	cout<<endl;
	return NULL;
}

void slice_def(int type1, int *addr1, int type2, int *addr2, int type3, int *addr3, int type, int *src, int *p){
	if(type1 == _NONE)
		p[0] = 0;
	else if(type1 == _INT) 
		p[0] = *addr1;
 	else {yyerror("slice indices must be integers or None or have an __index__ method"); ERROR; return;}

	if(type2 == _NONE){
		if(type == _LIST)
			p[1] = ((list *)src)->size();
		else if(type == _STRING_LTERAL)
			p[1] = ((string *)src)->size();
		else if(type == _CHAR)
			p[1] = 1;
		else if(type == _RANGE)
			p[1] = *(src + 1);
	}
	else if(type2 == _INT) 
		p[1] = *addr2;
 	else {yyerror("slice indices must be integers or None or have an __index__ method"); ERROR; return;}

	if(type3 == _NONE)
		p[2] = 1;
	else if(type3 == _INT) 
		p[2] = *addr3;
 	else {yyerror("slice indices must be integers"); ERROR; return;}
}

int* slice_obj(int type, int *addr, int *s, int *e, int *op){
	int rs, re, rop, len;
	int *p;
	rs = *s;
	if(*op == 0)
		{yyerror("slice cannot be zero"); ERROR; return NULL;}
	else
		 rop = *op;
	if(type == _LIST){
		p = _malloc(_LIST);
		len = ((list *)addr)->size(); 
		re = *e;
		if(rs < re && rop > 0){	
			if(re < -len-1)
				re = -1;
			else if(re < 0)
				re = len + re;
			else if(re >= len)
				re = len;
			rs = (rs >= 0 ? rs : len + rs);
			rs = (rs <= -len-1 ? 0 : rs);
			rs = (rs >= len ? len : rs);

			if(rs > len)
				{yyerror("list index out of range"); ERROR; return NULL;}
			for(int i = rs; i < re; i+=rop)			
				((list *)p)->push_back((*(list *)addr)[i]);
		}
		else if(rs > re && rop < 0){
			if(re < -len-1)
				re = -1;
			else if(re < 0)
				re = len + re;
			else if(re >= len)
				re = len;
			rs = (rs >= 0 ? rs : len + rs);
			rs = (rs <= -len-1 ? 0 : rs);
			rs = (rs >= len ? len : rs);

			if(rs > len)
				{yyerror("list index out of range"); ERROR; return NULL;}
			for(int i = rs; i > re; i+=rop)
				((list *)p)->push_back((*(list *)addr)[i]);
		}
		else{}
			//p is empty
	}
	else if(type == _STRING_LTERAL || type == _CHAR){
		p = _malloc(_STRING_LTERAL);
		if(type == _CHAR)
			len = 1;
		else 
			len = ((string *)addr)->length();
		re = *e;
		if(rs < re && rop > 0){
			if(re < -len-1)
				re = -1;
			else if(re < 0)
				re = len + re;
			else if(re >= len)
				re = len;
			rs = (rs >= 0 ? rs : len + rs);
			rs = (rs <= -len-1 ? 0 : rs);
			rs = (rs >= len ? len : rs);	
			if(rs > len || re > len)
				{yyerror("string index out of range"); ERROR; return NULL;}
			for(int i = rs; i < re; i+=rop){
				if(type == _CHAR){								
					*(string *)p += *(char *)addr;
				}
				else{
					((string *)p)->push_back((*(string *)addr)[i]);
				}
			}	
		}
		else if(rs > re && rop < 0){
			if(re < -len-1)
				re = -1;
			else if(re < 0)
				re = len + re;
			else if(re >= len)
				re = len;
			rs = (rs >= 0 ? rs : len + rs);
			rs = (rs <= -len-1 ? 0 : rs);
			rs = (rs >= len ? len : rs);
			if(rs > len || re > len)
				{yyerror("string index out of range"); ERROR; return NULL;}
			for(int i = rs; i > re; i+=rop){
				if(type == _CHAR)								
					*(string *)p += *(char *)addr;  	
				else
					((string *)p)->push_back((*(string *)addr)[i]);
			}
		}
	}
	else if(type == _RANGE){
		p = _malloc(_RANGE);
		len = (*(addr + 1) - *(addr) - 1) / *(addr+2) + 1;
		if(len < 0)
			len = 0;
		re = *e; 
		if(re < -len-1)
			re = -1;
		else if(re < 0)
			re = len + re;
		else if(re >= len)
			re = len;

		if(rs < -len-1)
			rs = 0;
		else if(rs < 0)
			rs = len + rs;
		else if(rs >= len)
			rs = len;

		*p = *(addr) + rs * *(addr+2);
		*(p+1) = *(addr) + re * *(addr+2);
		*(p+2) = rop * *(addr+2);
		
	}
	else
		{yyerror("object is not subscriptable"); ERROR; return NULL;}

	//cout<<"start: "<<rs<<" end: "<<re<<" pace: "<<rop<<" length: "<<len<<endl
	cite_record(addr, 0, 0);
	cite_record(s, 0, 0);
	cite_record(e, 0, 0);
	cite_record(op, 0, 0);
	//cout<<*(string *)p<<endl;
	return p;
}

para* push_stack(para *addr1, int *addr2, int type){
	para* a;
	if(!addr1)
		a = new para;
	else
		a = addr1;
	item temp;
	temp.addr = addr2;
	temp.type = type;
	a->push(temp);
	return a;
}

int* append(para *addr, int &type){
	item temp, obj, para;
	obj = addr->top();
	addr->pop();
	para = addr->top();
	addr->pop();
	temp.type = para.type;
	temp.addr = para.addr;
	if(obj.type == _LIST){
		((list*)(obj.addr))->push_back(temp);
		type = _NONE;
	}
	else {yyerror("a non-list found"); ERROR; return NULL;}
	return NULL;
}

int* len(para *addr, int &type){
	item src, para;
	addr->pop();
	src = addr->top();
	addr->pop();
	int* length = _malloc(_INT);
	switch(src.type){
		case _INT: yyerror("object of type 'int' has no len()");  ERROR; return NULL; break;
		case _REAL: yyerror("object of type 'float' has no len()");  ERROR; return NULL; break;
		case _LIST: *length = ((list *)(src.addr))->size(); break;
		case _CHAR: *length = 1; break;
		case _STRING_LTERAL: *length = ((string *)(src.addr))->size(); break;
		case _RANGE: 
			if(*(src.addr + 1) > *(src.addr))
				*length = (*(src.addr + 1) - *(src.addr) - 1) / *(src.addr+2) + 1; 
			else
				*length = (*(src.addr + 1) - *(src.addr) + 1) / *(src.addr+2) + 1; 
			if(*length < 0) *length = 0; break;
		default: *length = 0; break;
	}
	type = _INT;
	return length;
}

int* make_list(para *addr, int &type){
	item src, para;
	addr->pop();
	src = addr->top();
	addr->pop();
	int *p = _malloc(_LIST);
	switch(src.type){
		case _INT: yyerror("TypeError: 'int' object is not iterable"); ERROR; return NULL; break;
		case _REAL: yyerror("TypeError: 'float' object is not iterable"); ERROR; return NULL; break;
		case _LIST:
			p = src.addr; 
			break;
		case _CHAR: {
			int* tmp = _malloc(_STRING_LTERAL);
			((string*)tmp)->push_back(*(char*)(src.addr));
			p = add_list(p, tmp, _STRING_LTERAL);
			break;
			}
		case _STRING_LTERAL:
			for(int i = 0; i<((string *)(src.addr))->size(); ++i){
				int* tmp = _malloc(_STRING_LTERAL);
				((string*)tmp)->push_back((*((string*)(src.addr)))[i]);
				p = add_list(p, tmp, _STRING_LTERAL);
			}
		 	break;
		case _RANGE:  {
			if((src.addr)[0] < (src.addr)[1] && (src.addr)[2] > 0){
            	for(int i=(src.addr)[0] ; i < (src.addr)[1] ; i+=(src.addr)[2] ){
					int *t = _malloc(_INT);
					*t=i;               		
					p = add_list(p, t, _INT);
				}
			}
			else if((src.addr)[0] > (src.addr)[1] && (src.addr)[2] < 0){
            			for(int i=(src.addr)[0] ; i > (src.addr)[1] ; i+=(src.addr)[2] ){
					int *t = _malloc(_INT);
					*t=i;               		
					p = add_list(p, t, _INT);
				}
			}
			else {          		
					p = add_list(p, 0, -1);
				}
 			break;
        	}
	}
	type = _LIST;
	return p;
}

int* range(para *addr, int &type){
    int *r = _malloc(_RANGE);  //r[0]起点，r[1]终点，r[2]偏移
    item temp;
    int i;
    addr->pop();
    r[0] = 0;  //起点可省略，默认0
    r[1] = 0;
    r[2] = 1;  //步长可省略，默认1
    switch( addr->size() ){
        case 1: {
            temp = addr->top(); addr->pop();
            r[1] = *(temp.addr);
            break;
        }
        case 2: {
            temp = addr->top(); addr->pop(); r[1] = *(temp.addr);
            temp = addr->top(); addr->pop(); r[0] = *(temp.addr);
            break;
        }
        case 3: {
            temp = addr->top(); addr->pop(); r[2] = *(temp.addr);
            temp = addr->top(); addr->pop(); r[1] = *(temp.addr);
            temp = addr->top(); addr->pop(); r[0] = *(temp.addr);
            break;
        }
    }
    
    type = _RANGE;
    return r;
}

void addfunc(const char *name, int* (*funcptr)(para*, int&)){
	if(name){
        	objtab p;
       		p.type = -1;
        	p.addr = NULL;
		p.funcptr = funcptr;
		symbol_table.insert(pair<string, objtab>(name,p));
	}
}

void yyerror(const char *s){
   cout << s << endl<<"miniPy> "; 
}

