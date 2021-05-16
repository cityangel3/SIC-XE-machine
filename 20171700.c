#include "20171700.h"
//헤더파일에서 extern 선언한 전역변수 definition//
Queue* list_ht;
char command[50];
int virtual_ad;
int prog_addr = 0;
int main()
{
    char inst[101],back_inst[101],flag[101],flag2[50],flag3[50],temp; //명령어 관련 변수
    int cmlen,backlen,op;
    int start_ad,end_ad,val,quit=0; // dump [start,end] 주소 및 quit 명령어에 대한 변수
    
    if (!read_opcode()) //'opcode.txt'읽고 저장
        printf("Wrong opcode file\n");
    
    virtual_ad = 0; //dump 실행 후 마지막 address 저장 변수
    list_ht = (Queue*)malloc(sizeof(Queue)); //history 저장 list
    list_ht->front = NULL;
    list_ht->rear = NULL;
    
    //'q[uit]' 입력 전까지 명령어 받음
    while(!quit){
        printf("sicsim> ");
        memset(inst,'0',sizeof(char)*101);
        fgets(inst,101,stdin);
        if(inst[strlen(inst)-1] != '\n'){
            while(getchar() != '\n');
        }
        if(inst[0] == '\n'||inst[0] == '\0') continue; //입력 없는 경우
        back_inst[0] = '\0';
        flag[0] = '\0'; flag2[0] = '\0'; flag3[0] = '\0';
        start_ad = end_ad = val = -1;
        sscanf(inst,"%s %s",command,back_inst); //명령어 부분과 그 뒷부분으로 나눔 ex) dump start
        cmlen = strlen(command);
        backlen = strlen(back_inst);
        switch(command_to_num()){ //명령어에 따라서 case를 나눔
            case 1 : //'h[elp]' case
                if(backlen){ //명령어 뒷부분에 입력이 없어야 되는데 있는경우 예외 처리 (이하 동문)
                    printf("Input error\n");
                    break;
                }
                help();
                record_ht(command,cmlen); break; //명령어 history에 저장 (이하 동문)
            case 2 : //'d[ir]' case
                if(backlen){
                    printf("Input error\n");
                    break;
                }
                dir();
                record_ht(command,cmlen); break;
            case 3 : //'q[uit]' case
                if(backlen){
                    printf("Input error\n");
                    break;
                }
                record_ht(command,cmlen);
                quit = 1;
                break;
            case 4 : //'hi[story]' case
                if(backlen){
                    printf("Input error\n");
                    break;
                }
                record_ht(command,cmlen); //'history'명령어도 저장
                print_list(); break;
            case 5 : //'du[mp]' case
                if(backlen){ //dump start 또는 dump start, end 인 case
                    if(back_inst[0] == ','){ printf("Input error\n"); break;}
                    if(error_hy(inst) == 1){printf("Input error\n"); break;}
                    flag[0] = '\0';
                    sscanf(inst,"%s %X %s",command,&start_ad,flag);
                    if(flag[0] == '\0'){ // 'dump start' case
                        if(start_ad < 0 || start_ad > 0xfffff){
                            printf("Out Of Range!\n");
                            break;
                        }
                        else
                            end_ad = -1;
                    }
                    else{ // 'dump start, end' case
                        flag[0] = '\0';
                        sscanf(inst,"%s %X %c %X %s",command,&start_ad,&temp,&end_ad,flag);
                        if(flag[0] != '\0'|| (start_ad-end_ad) > 0 || temp != ','){
                            printf("Input error\n");
                            break;
                        }
                        else if(start_ad < 0 || start_ad > 0xfffff || end_ad < 0 || end_ad > 0xfffff){
                            printf("Out Of Range!\n");
                            break;
                        }
                    }
                    record_ht(inst,-1); //명령어 이외에 뒷부분이 있는 경우 정제된 형태로 저장 (이하 동문)
                }
                else{ // 'dump' case
                    start_ad = end_ad = -1;
                    record_ht(command,cmlen);
                }
                dump(start_ad,end_ad);
                break;
            case 6 : // 'e[dit]' case
                if(back_inst[0] == ','){ printf("Input error\n"); break;}
                if(error_hy(inst) == 1){printf("Input error\n"); break;}
                flag[0] = '\0';
                sscanf(inst,"%s %X %c %X %s",command,&start_ad,&temp,&val,flag);
                if(flag[0] != '\0' || temp != ',' || val == -1){
                    printf("Input error\n");
                    break;
                }
                else if(0xfffff < start_ad || start_ad <0 || 0xff < val || val < 0){
                    printf("Input error\n");
                    break;
                }
                else
                    edit(start_ad,val);
                record_ht(inst,-1);
                break;
            case 7 : // 'f[ill]' case
                if(back_inst[0] == ','){ printf("Input error\n"); break;}
                if(error_hy(inst) == 1){printf("Input error\n"); break;}
                memset(back_inst,'\0',sizeof(char)*101);
                sscanf(inst,"%s %X %c %X %c %X %s",command,&start_ad,&flag[0],&end_ad,&flag[1],&val,back_inst);
                if(back_inst[0] != '\0' || flag[0] != ',' || flag[1] != ','|| (start_ad-end_ad) > 0 || val == -1){
                    printf("Input error\n");
                    break;
                }
                else if(0xfffff<start_ad || start_ad<0 || 0xfffff<end_ad || end_ad<0){
                    printf("Out Of Range!\n");
                    break;
                }
                else if(0xff < val || val < 0){
                    printf("Input error\n");
                    break;
                }
                else{
                    fill(start_ad,end_ad,val);
                }
                record_ht(inst,-2);
                break;
            case 8 : // 'reset' case
                if(backlen){
                    printf("Input error\n"); break;
                }
                record_ht(command,cmlen);
                reset();
                break;
            case 9 : // 'opcode mnemonic' case
                sscanf(inst,"%s %s %s",command,back_inst,flag);
                if(flag[0] != '\0'){
                    printf("Input error\n"); break;
                }
                op = opcode(back_inst);
                if(op==0){
                    printf("Input error\n"); break;
                }
                else
                    printf("opcode is %02X\n",op);
                record_ht(inst,-3);
                break;
            case 10 : // 'opcodelist' case
                if(backlen){
                    printf("Input error\n"); break;
                }
                record_ht(command,cmlen);
                opcodelist();
                break;
            case 11 : // 'type filename' case
                if(!read_file(back_inst)){
                    printf("Input error\n"); break;
                }
                record_ht(inst,-3);
                break;
            case 12 : // 'assemble filename' case
                if(assemble(back_inst)){
                    record_ht(inst,-3);
                    store_symtab(); //assemble에 성공했을 시, symbol과 location을 따로 저장.
                }
                break;
            case 13 : // 'symbol' case
                if(backlen){
                    printf("Input error\n"); break;
                }
                record_ht(command,cmlen);
                symbol();
                break;
            case 14 : // 'progaddr' case
                if(backlen){
                    sscanf(inst,"%s %X %s",command,&prog_addr,flag);
                    if(flag[0] != '\0'){printf("Input error\n"); break;}
                    if(0xfffff < prog_addr || prog_addr < 0){printf("Input error\n"); break;} // 범위 초과하는 경우 error
                    record_ht(inst,-5);
                }
                else{printf("Input error\n");}
                break;
            case 15 : // 'loader' case
                if(backlen){
                    sscanf(inst,"%s %s %s %s %s",command,back_inst,flag,flag2,flag3);
                    if(flag[0] == '\0'){ // object file 1개인 경우
                        if(loader(back_inst,"\0","\0",1) == 1){//정상적으로 수행시
                            record_ht(inst,-3);
                        }//error 있는 경우 estab free
                        else{printf("Input error\n"); free_estab();}
                    }
                    else if(flag2[0] == '\0'){ // object file 2개인 경우
                        if(loader(back_inst,flag,"\0",2) == 1){//정상적으로 수행시
                            record_ht(inst,-6);
                        }//error 있는 경우 estab free
                        else{printf("Input error\n"); free_estab();}
                    }
                    else{ // object file 3개인 경우
                        if(flag3[0] != '\0'){printf("Input error\n"); break;}
                        if(loader(back_inst,flag,flag2,3) == 1){//정상적으로 수행시
                            record_ht(inst,-6);
                        }//error 있는 경우 estab free
                        else{printf("Input error\n"); free_estab();}
                    }
                }
                else{printf("Input error\n");}
                break;
            case 16 : // 'bp' case
                if(backlen){
                    sscanf(inst,"%s %s %s",command,flag,flag2);
                    if(flag2[0] != '\0'){printf("Input error\n"); break;}
                    else{ // bp clear
                        if(strcmp(flag,"clear")==0){
                            bp_process(-3);
                            record_ht(inst,-3);
                        }
                        else{ // bp address
                            sscanf(inst,"%s %X",command,&val);
                            if(0xfffff<val || val<0){printf("Input error\n"); break;} // 범위 check
                            bp_process(val);
                            record_ht(inst,-5);
                        }
                    }
                }
                else{ // bp만 입력된 경우
                    bp_process(-7);
                    record_ht(command,cmlen);
                }
                break;
            case 17 : // 'run' case
                if(backlen){
                    printf("Input error\n"); break;
                }
                if(run() == 0) break;
                record_ht(command,cmlen);
                break;
            default : printf("Input error\n"); break;
        }
    }
    free_all(); //동적 할당 해제하고 종료
    return 0;
}
int command_to_num(){
    if(strcmp(command,"h") == 0 || strcmp(command,"help") == 0) return 1;
    if(strcmp(command,"d") == 0 || strcmp(command,"dir") == 0) return 2;
    if(strcmp(command,"q") == 0 || strcmp(command,"quit") == 0) return 3;
    if(strcmp(command,"hi") == 0 || strcmp(command,"history") == 0) return 4;
    if(strcmp(command,"du") == 0 || strcmp(command,"dump") == 0) return 5;
    if(strcmp(command,"e") == 0 || strcmp(command,"edit") == 0) return 6;
    if(strcmp(command,"f") == 0 || strcmp(command,"fill") == 0) return 7;
    if(strcmp(command,"reset") == 0) return 8;
    if(strcmp(command,"opcode") == 0) return 9;
    if(strcmp(command,"opcodelist") == 0) return 10;
    if(strcmp(command,"type") == 0) return 11;
    if(strcmp(command,"assemble") == 0) return 12;
    if(strcmp(command,"symbol") == 0) return 13;
    if(strcmp(command,"progaddr") == 0) return 14;
    if(strcmp(command,"loader") == 0) return 15;
    if(strcmp(command,"bp") == 0) return 16;
    if(strcmp(command,"run") == 0) return 17;
    return 0;
}
void help(void){
    printf("h[elp]\n");
    printf("d[ir]\n");
    printf("q[uit]\n");
    printf("hi[story]\n");
    printf("du[mp] [start, end]\n");
    printf("e[dit] address, value\n");
    printf("f[ill] start, end, value\n");
    printf("reset\n");
    printf("opcode mnemonic\n");
    printf("opcodelist\n");
    printf("assemble filename\n");
    printf("type filename\n");
    printf("symbol\n");
    printf("progaddr [address]\n");
    printf("loader [object filename1] [object filename2] [...] (max 3)\n");
    printf("run\n");
    printf("bp [adddress] / bp / bp clear\n");
}
void dir(void){
    DIR* dirtory;
    struct dirent* ent;
    int cnt=0;
    struct stat buffer;
    dirtory = opendir ("./");
    if(dirtory != NULL){
        while((ent = readdir (dirtory)) != NULL){
            if(!cnt)
                printf("\t");
            lstat(ent->d_name,&buffer);
            if(S_ISDIR(buffer.st_mode)) //directory인 경우 /표시
                printf("%s/\t",ent->d_name);
            else if(S_IEXEC & buffer.st_mode) //실행 파일인 경우 *표시
                printf("%s*\t",ent->d_name);
            else
                printf("%s\t",ent->d_name);
            cnt += 1;
            if(cnt % 5 == 0){
                cnt -= 5;
                printf("\n");
            }
        }
        printf("\n");
        closedir(dirtory);
    }
    else
        printf("Directory Error!\n");
}
void record_ht(char* inst,const int len){
    char flag[100],flag2[50],flag3[50];
    int front,back,val;
    flag2[0] = '\0'; flag3[0] = '\0';
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->next = NULL;
    if(len == -1){ //dump start or dump start, end or edit address, value 일 때
        flag[0] = '\0';
        sscanf(inst,"%s %X %s",command,&front,flag);
        if(flag[0] != '\0'){
            sscanf(inst,"%s %X %c %X",command,&front,&flag[0],&back);
            sprintf(temp->instruction,"%s %X, %X",command,front,back);
        }
        else
            sprintf(temp->instruction,"%s %X",command,front);
    }
    else if(len == -2){ //fill start, end, val 일 때
        sscanf(inst,"%s %X %c %X %c %X",command,&front,&flag[0],&back,&flag[1],&val);
        sprintf(temp->instruction,"%s %X, %X, %X",command,front,back,val);
    }
    else if(len == -3){ // type filename, assemble filename, loader file1, bp clear일 때
        sscanf(inst,"%s %s",command,flag);
        sprintf(temp->instruction,"%s %s",command,flag);
    }
    else if(len == -5){ // progaddr [address] , bp [address] 일 때
        sscanf(inst,"%s %X",command,&front);
        sprintf(temp->instruction,"%s %X",command,front);
    }
    else if(len == -6){ //loader file 2 or file 3 일 때
        sscanf(inst,"%s %s %s %s",command,flag,flag2,flag3);
        if(flag3[0] == '\0')
            sprintf(temp->instruction,"%s %s %s",command,flag,flag2);
        else
            sprintf(temp->instruction,"%s %s %s %s",command,flag,flag2,flag3);
    }
    else //명령어만 있는 경우
        sprintf(temp->instruction,"%s",inst);
    if(list_ht->front == NULL){
        list_ht->front = temp;
        list_ht->rear = temp;
    }
    else{
        list_ht->rear->next = temp;
        list_ht->rear = temp;
    }
}
void print_list(void){ //명령어들을 입력한 순서대로 출력
    Node* temp = list_ht->front;
    for(int i = 1;temp != NULL; i++){
        printf("%d\t%s\n",i,temp->instruction);
        temp = temp->next;
    }
}
void free_all(){ //동적 할당 해제
    int i;
    Node* hi_ptr,*temp1;
    Hash* hash_ptr;
    free_symtab();
    for(hi_ptr = list_ht->front;hi_ptr != NULL;){
        temp1 = hi_ptr;
        hi_ptr = hi_ptr->next;
        free(temp1);
    }
    free(list_ht);
    
    for(i=0;i<20;i++){
        if(h_table[i] !=NULL){
            for(;h_table[i] != NULL;){
                hash_ptr = h_table[i];
                h_table[i] = h_table[i]->next;
                free(hash_ptr);
            }
        }
    }
    for(i=0;i<symbol_num;i++){
        free(symbol_arr[i]);
    }
    free(symbol_arr);
    free(location_arr);
}
int error_hy(char* inst){ //0x 형태의 16진수 입력을 error로 체크함.
  int first, end;
  char tempor1,tempor2;
  char check1[20],check2[20];
  sscanf(inst,"%s %s",command,check1);
  if(check1[0] == '0' && check1[1] == 'x')
    return 1;
  sscanf(inst,"%s %X %c %s",command,&first,&tempor1,check2);
  if(check2[0] == '0' && check2[1] == 'x')
    return 1;
  sscanf(inst,"%s %X %c %X %c %s",command,&first,&tempor1,&end,&tempor2,check2);
  if(check2[0] == '0' && check2[1] == 'x')
    return 1;
  return 0;
}
int read_file(char* name){ //'type filename' 일 때 해당 filename있으면 읽고 출력하고 return 1, 없으면 return 0;
    FILE* file = fopen(name,"r");
    char readch;
    if(strlen(name) == 0 || file == NULL) // filename이 공백이거나, directory에 없으면 error 처리.
        return 0;
    while(fscanf(file,"%c",&readch) != EOF)// character 단위로 읽는다.
        printf("%c",readch);
    if(readch != '\n') // 파일 마지막에 개행이 없을경우 개행을 해준다.
        printf("\n");
    fclose(file);
    return 1;
}
