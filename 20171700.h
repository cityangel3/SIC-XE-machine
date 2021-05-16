#ifndef f_20171700_h
#define f_20171700_h

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>

typedef struct node{
    struct node* next;
    char instruction[101];
}Node;
/* => shell에 입력된 명령어들을 저장하는 Node struct*/

typedef struct queue{
    Node* front;
    Node* rear;
}Queue;
/* => Queue를 사용하여 저장된 명령어 node들의 처음과 끝을 가르킴*/

typedef struct hash{
    struct hash* next;
    char mnemonic[20],format[10];
    int code;
}Hash;
/* => opcode를 linked list형태로 저장하기 위한 Hash struct*/

typedef struct syb{
    struct syb* next;
    char symbol[30];
    int location;
}Syb;
/* => SYMTAB을 만들기위한 Syb struct*/

typedef struct obj{
    unsigned char code[20];
    int format,flag;
}Obj;
/* => objectcode를 만들기위한 obj struct, unsigned char형의 배열의 각 1byte씩 저장*/

typedef struct mode{
    int address,ni;
    char address_mode;
}Mode;
/* => pass2에서 해당 instruction의 addressing mode를 찾아 object code를 만드는데 쓰임*/

typedef struct est{
    struct est* next;
    int address,length;
    char symbol_name[30],ps;
}Est;
/* => ESTAB을 만들기위한 Est struct*/

/*-전역 변수 공유를 위해 extern으로 declaration-*/
extern Queue* list_ht;
/* => history 기능을 수행하기 위해 queue linked list 구현*/
extern Hash* h_table[20];
/* => opcode를 hashtable에 저장*/
extern char command[50];
/* => 명령어 저장*/
extern unsigned char virtual_mem[65536][16];
/* => 1MByte의 가상 메모리 공간*/
extern int virtual_ad;
/* => dump 명령어 실행 후 마지막 address가 저장됨*/
extern Syb* symtab[26];
/* => SYMTAB 역할 */
extern Obj* objcode;
/* => 각 line의 object code를 저장함 */
extern int* locctr;
/* => LOCCTR 역할 */
extern int t_line;
/* => .asm 프로그램의 총 길이*/
extern char program_name[30];
/* => .asm program의 이름 */
extern char** symbol_arr;
/* => SYMTAB에서 symbol만 따로 저장함 */
extern int* location_arr;
/* => SYMTAB에서 location만 따로 저장함 */
extern int symbol_num;
/* => SYMTAB 총 길이 */
extern int success_asm;
/* => assemble에 성공했는지 여부를 나타냄*/

extern int prog_addr;
/* => progaddr 값을 저장*/
extern Est* estab[26];
/* => ESTAB 역할 */
extern int cs_addr;
/* => CSADDR 역할 */
extern int cs_lth;
/* => CSLTH 역할 */
extern int start_addr;
/* => 프로그램 시작 가능 주소 */
extern int end_addr;
/* => 프로그램 마지막 주소 */
extern int start_run;
/* => run 명령어 입력시 시작 주소 */
extern int arr_bp[1000];
/* => breakpoint 위치를 저장하는 배열 */
extern int check[65536][16];
/* => breakpoint 위치 */
extern int num_bp;
/* => breakpoint 총 개수 */
extern int regA,regX,regL,regB,regS,regT,regPC,CC;
/* => run 수행 후 출력될 레지스터 */
         

int loader(char *file1, char *file2, char*file3, int num);
/*
 ┌
 ｜ ○ name: loader
 ｜ ○ argument: file1, file2, file3, num
 ｜ ○ perform: 주어진 obj file을 linking하고 load함.
 └
 */
int process1(char* file);
/*
 ┌
 ｜ ○ name: process1
 ｜ ○ argument: file
 ｜ ○ perform: linking loader의 pass1을 수행함. H와 D레코드를 읽음
 └
 */
int process2(char* file,int cs_start);
/*
 ┌
 ｜ ○ name: process2
 ｜ ○ argument: file1, cs_start
 ｜ ○ perform: linking loader의 pass2를 수행함.
 └
 */
int add_to_estab(char* name,int addr,int length,char ps);
/*
 ┌
 ｜ ○ name: add_to_estab
 ｜ ○ argument: name, addr, length, ps
 ｜ ○ perform: name을 이용하여 hash 값을 생성하고,
 ｜            해당 external 심볼의 정보를 estab에 저장함.
 └
 */
int is_in_estab(char* name);
/*
 ┌
 ｜ ○ name: is_in_estab
 ｜ ○ argument: name
 ｜ ○ perform: estab에 해당 이름의 심볼이 있는지 확인함.
 ｜            있으면 해당 주소를 반환. 없으면 -1반환.
 └
 */
int hash_value_estab(char* name);
/*
 ┌
 ｜ ○ name: hash_value_estab
 ｜ ○ argument: name
 ｜ ○ perform: name을 이용하여 hash 값을 생성한다.
 ｜            해당 name의 가장 끝자리 문자를 활용함.
 └
 */
void free_estab(void);
/*
 ┌
 ｜ ○ name: free_estab
 ｜ ○ argument: -
 ｜ ○ perform: estab을 동적 해제함.
 └
 */
void print_estab(void);
/*
 ┌
 ｜ ○ name: print_estab
 ｜ ○ argument: -
 ｜ ○ perform: load가 성공적으로끝나고, load map을 출력함.
 └
 */
void bp_process(int bp);
/*
 ┌
 ｜ ○ name: bp_proecss
 ｜ ○ argument: bp
 ｜ ○ perform: bp값에 따라서 bp clear, bp, bp address 등을 수행함.
 └
 */
int run(void);
/*
 ┌
 ｜ ○ name: run
 ｜ ○ argument: -
 ｜ ○ perform: copy.obj파일을 실행함.
 ｜            각 line의 object code를 disassemble한다.
 └
 */
void print_reg(void);
/*
 ┌
 ｜ ○ name: print_reg
 ｜ ○ argument: -
 ｜ ○ perform: run 수행 후 각 register들을 출력함.
 └
 */
int reg_num(int num,int erase);
/*
 ┌
 ｜ ○ name: reg_num
 ｜ ○ argument: num, erase
 ｜ ○ perform: num 값에 따라 해당 레지스터 값을 반환.
 ｜            erase =1 일시 해당 레지스터 값 지움.
 └
 */
int command_to_num(void);
/*
 ┌
 ｜ ○ name: command_to_num
 ｜ ○ argument: -
 ｜ ○ perform: 입력된 command마다 다른 상수값을 return,
 ｜             main의 switch문에서 그 값에 따른 함수 실행.
 └
 */
void help(void);
/*
 ┌
 ｜ ○ name: help
 ｜ ○ argument: -
 ｜ ○ perform: shell에서 실행 가능한 모든 명령어 출력.
 └
 */
void dir(void);
/*
 ┌
 ｜ ○ name: dir
 ｜ ○ argument: -
 ｜ ○ perform: directory에 있는 파일들을 출력.
 ｜             (실행파일은 * , directory는 / 표시)
 └
 */
void record_ht(char* inst,const int len);
/*
 ┌
 ｜ ○ name: record_ht
 ｜ ○ argument: inst( 입력된 command ), len( command의 길이 )
 ｜ ○ perform: 입력된 command를 형태에 맞추어 저장.
 ｜             (공백 제거하고 정제된 형태로 저장한다.)
 └
 */
void print_list(void);
/*
 ┌
 ｜ ○ name: print_list
 ｜ ○ argument: -
 ｜ ○ perform: 현재까지 사용한 명령어들을 순서대로 출력.
 ｜            (linked list구조로 저장된 값을 순회하며 출력한다.)
 └
 */
void dump_print(int start_pt,int end_pt);
/*
 ┌
 ｜ ○ name: dump_print
 ｜ ○ argument: start_pt (시작 번지), end_pt (마지막 번지)
 ｜ ○ perform: 시작번지부터 마지막번지까지 가상 메모리의 내용을 출력한다.
 └
 */
void dump(int start_ad,int end_ad);
/*
 ┌
 ｜ ○ name: dump
 ｜ ○ argument: start_ad (시작 번지) , end_ad (마지막 번지)
 ｜ ○ perform: start_ad 와 end_ad 값에 따라
 ｜            dump / dump start / dump [start, end]를 수행
 └
 */
void edit(int add,int val);
/*
 ┌
 ｜ ○ name: edit
 ｜ ○ argument: add (값을 바꿀 번지), val (바꿀 값)
 ｜ ○ perform: 입력된 번지의 값을 바꿈.
 └
 */
void fill(int start_ad,int end_ad,int val);
/*
 ┌
 ｜ ○ name: fill
 ｜ ○ argument: start_ad (시작 번지) , end_ad (마지막 번지), val (바꿀 값)
 ｜ ○ perform: 메모리 공간의 입력된 시작 번지부터 마지막 번지까지 해당 값으로 바꿈.
 └
 */
void reset(void);
/*
 ┌
 ｜ ○ name: reset
 ｜ ○ argument: -
 ｜ ○ perform: 메모리 공간 전체를 0으로 초기화함.
 └
 */
int read_opcode(void);
/*
 ┌
 ｜ ○ name: read_opcode
 ｜ ○ argument: -
 ｜ ○ perform: opcode.txt 로부터 opcode를 읽고 hash table에 저장함.
 ｜            잘못 읽었을시 0, 정상적으로 읽었을시 1 return.
 └
 */
int hash_function(char* key,unsigned long max);
/*
 ┌
 ｜ ○ name: hash_function
 ｜ ○ argument: key (mnemonic), max (mnemonic의 길이)
 ｜ ○ perform: key로부터 hashing을 통해 고유한 인덱스 값을 생성하여
 ｜            그 값을 return. (잘못된 mnemonic일시 -1 return.)
 └
 */
int opcode(char* mnm);
/*
 ┌
 ｜ ○ name: opcode
 ｜ ○ argument: mnm (mnemonic)
 ｜ ○ perform: mnemonic을 hash table에서 찾아 출력.
 ｜            잘못된 mneonic이거나 없을시 오류 메시지 출력.
 └
 */
void opcodelist(void);
/*
 ┌
 ｜ ○ name: opcodelist
 ｜ ○ argument: -
 ｜ ○ perform: hash table에 저장돼 있는 opcode 내용을 출력
 └
 */
void free_all(void);
/*
 ┌
 ｜ ○ name: free_all
 ｜ ○ argument: -
 ｜ ○ perform: 동적할당 받은 메모리를 해제함.
 └
 */
int error_hy(char* inst);
/*
 ┌
 ｜ ○ name: error_hy
 ｜ ○ argument: inst
 ｜ ○ perform: 0x 형태의 입력 오류를 체크함
 └
 */
int read_file(char* name);
/*
 ┌
 ｜ ○ name: read_file
 ｜ ○ argument: name
 ｜ ○ perform: name으로 filename을 받아서,
 ｜            해당 파일을 읽고 화면에 출력한다.
 └
 */
int assemble(char* name);
/*
 ┌
 ｜ ○ name: assemble
 ｜ ○ argument: name
 ｜ ○ perform: name으로 filename을 받아서,
 ｜            .lst 와 .obj 의 확장자 이름을 가진 list file 과 object program을 만듦.
 └
 */
int pass1(char* name);
/*
 ┌
 ｜ ○ name: pass1
 ｜ ○ argument: name
 ｜ ○ perform: name으로 filename을 받아서,
 ｜            해당 파일을 object code로 변환하기 위한 pass1과정. SYMTAB을 작성및 오류 검출.
 └
 */
int pass2(char* name);
/*
 ┌
 ｜ ○ name: pass2
 ｜ ○ argument: name
 ｜ ○ perform: name으로 filename을 받아서,
 ｜            해당 파일을 object code로 변환하기 위한 pass2과정. object code 작성및 오류 검출.
 └
 */
void error_in_asm(char* content, int number);
/*
 ┌
 ｜ ○ name: error_in_asm
 ｜ ○ argument: content, number
 ｜ ○ perform: .asm 파일에서 오류가 있을시 출력함. content는 해당 오류를 가지고 있는 line의 내용이고,
 ｜            number는 해당 라인 번호임.
 └
 */
int direct_or_opcode(char* name);
/*
 ┌
 ｜ ○ name: direct_or_opcode
 ｜ ○ argument: name
 ｜ ○ perform: name으로 mnemonic을 받아서,
 ｜            directive 인지 opcode인지 구별하여 return함.
 └
 */
int add_to_symtab(char* symbol,int location);
/*
 ┌
 ｜ ○ name: add_to_symtab
 ｜ ○ argument: symbol, location
 ｜ ○ perform: symbol과 location을 SYMTAB에 저장함,
 └
 */
int find_from_symtab(char* name);
/*
 ┌
 ｜ ○ name: find_from_symtab
 ｜ ○ argument: name
 ｜ ○ perform: symbol과 location을 SYMTAB에 저장함,
 └
 */
int is_decimal(char* op,int len,int cs);
/*
 ┌
 ｜ ○ name: is_decimal
 ｜ ○ argument: op, len, cs
 ｜ ○ perform: operand가 10진수로만 이뤄져 있는지 확인함.
 └
 */
int is_char(char* op,int len,int cs);
/*
 ┌
 ｜ ○ name: is_char
 ｜ ○ argument: op, len, cs
 ｜ ○ perform: operand가 char type으로만 이뤄져 있는지 확인함.
 └
 */
int is_in_optab(char*name);
/*
 ┌
 ｜ ○ name: is_in_optab
 ｜ ○ argument: name
 ｜ ○ perform: OPTAB에 해당 opcde가 있는지 확인하고 (hash table로부터 확인), format을 return.
 └
 */
void free_symtab(void);
/*
 ┌
 ｜ ○ name: free_symtab
 ｜ ○ argument: -
 ｜ ○ perform: SYMTAB을 작성할때 동적할당 받았던 메모리를 해제함.
 └
 */
Mode find_addressing(char* name);
/*
 ┌
 ｜ ○ name: find_addressing
 ｜ ○ argument: name
 ｜ ○ perform: name으로부터 register인지 symbol인지 구분하며,
 ｜            simple, immediate, indirect addressing을 구분하고, symbol일 경우 SYMTAB에 있는지 확인.
 └
 */
void store_symtab(void);
/*
 ┌
 ｜ ○ name: store_symtab
 ｜ ○ argument: -
 ｜ ○ perform: SYMTAB의 symbol과 location을 각각 symbol_arr과 location_arr에 저장함.
 └
 */
void symbol(void);
/*
 ┌
 ｜ ○ name: symbol
 ｜ ○ argument: -
 ｜ ○ perform: SYMTAB에 가장 최근에 저장돼 있는 내용을 출력함. (label과 loacation)
 └
 */
#endif /* f_20171700_h */
