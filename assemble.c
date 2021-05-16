#include "20171700.h"
//헤더파일에서 extern 선언한 전역변수 definition//
Syb* symtab[26];
Obj* objcode;
int* locctr;
char program_name[30];
char** symbol_arr;
int success_asm=0;
int t_line;
int* location_arr;
int symbol_num;
int assemble(char* name){
    int nm_len = strlen(name),sym_temp;
    int i,j,len,cursor,wcur,flag=0;
    char obj_name[30], list_name[30], temp[10];
    char line[700];
    FILE* object, *list, *file;
    t_line = 0;
    file = fopen(name,"r"); //directory에 file 있는지 확인
    if(file == NULL) {
        printf("Input error: file does not exist.\n");
        return 0;
    }
    strcpy(temp,name+nm_len-4);
    if(strcmp(temp,".asm")!=0){ //extension
        printf("Input error: filename extension is not asm.\n");
        return 0;
    }
    else{
        strncpy(obj_name, name, nm_len - 3);
        sprintf(obj_name+nm_len-3,"obj");
        strncpy(list_name, name, nm_len - 3);
        sprintf(list_name+nm_len-3,"lst");
        list_name[nm_len] = '\0'; obj_name[nm_len] = '\0';
    }
    
    free_symtab();
    if(success_asm == 0) {symbol_num = 0;}
    sym_temp = symbol_num;
    symbol_num = 0;
    if(!pass1(name)){ // pass1을 수행하고 실패하면 flag -1로 set, 할당해제
        success_asm = -1; symbol_num = sym_temp;
        free_symtab();
        return 0;
    }
    else if(!pass2(name)){ // pass2를 수행하고 실패하면 flag -1로 set, 할당해제
        success_asm = -1; symbol_num = sym_temp;
        free_symtab();
        free(objcode);
        return 0;
    }
    else
        success_asm = 1;
    printf("[%s], [%s]\n",list_name,obj_name);
    //.obj file
    object = fopen(obj_name,"w");
    if(strlen(program_name)) //header record
        fprintf(object,"H%-6s%06X%06X\n",program_name,locctr[1],locctr[t_line]-locctr[1]);
    else
        fprintf(object,"H      %06X%06X\n",locctr[1],locctr[t_line]-locctr[1]);
    
    for(cursor = 1;cursor <= t_line;){ // text record
        while(locctr[cursor] < locctr[1] || objcode[cursor].format == -7){//주석이나, RESW, RESB일때 넘어감
            cursor++;
            if(cursor > t_line) {flag = 1; break;}
        }
        if(flag) break;
        fprintf(object,"T%06X",locctr[cursor]);
        for(i = cursor,len=0; objcode[i].format != -7;){ //RESW, RESB일때 넘어감
            if(0 < objcode[i].format){
                if(30 < len + objcode[i].format && 0 < len)
                    break;
                len += objcode[i].format;
                i++;
            }
            else
                i++;
        }
        fprintf(object,"%02X",len);
        for(j = cursor; j < i; j++){
            if(0 < objcode[j].format){
                for(wcur = 0;wcur<objcode[j].format;wcur++){
                    fprintf(object,"%02X",objcode[j].code[wcur]);
                }
            }
        }
        fprintf(object,"\n");
        cursor = i;
    }
    for(i=1;i<t_line;i++){ //4형식 modification record
        if(objcode[i].format == 4){
            if(objcode[i].flag != 7){
                fprintf(object,"M%06X%02X\n",1+locctr[i],5);
            }
        }
    }
    fprintf(object,"E%06X\n",locctr[1]);
    fclose(object);
    //.lst file
    list = fopen(list_name,"w");
    file = fopen(name,"r");
    for(i=1;i<=t_line;i++){
        fgets(line,700,file);
        fprintf(list,"%-1d\t",i*5); //line number 작성 (5씩 증가)
        if(i==t_line || locctr[i]<locctr[1]) fprintf(list,"    ");
        else fprintf(list,"%04X",locctr[i]);
        fprintf(list,"\t");
        //listing 파일에서 symbol(label), mnemonic, operand 부분 작성
        len = strlen(line);
        if(line[len-1]=='\n')line[len-1]='\0';
        fprintf(list,"%-35s",line);
        //START, BASE, coment를 제외하고 object code 출력
        if(line[0] == '.' || objcode[i].format < 0){
            fprintf(list,"\n");
            continue;
        }
        else if(0 < objcode[i].format){
            for(j=0;j<objcode[i].format;j++){
                fprintf(list,"%02X",objcode[i].code[j]);
            }
        }
        fprintf(list,"\n");
    }
    fclose(list);
    fclose(file);
    free(objcode);
    free(locctr);
    return 1;
}
void error_in_asm(char* content, int number){
    content[strlen(content)-1] = '\0';
    printf("[Assemble Error] in %d's line -> %s\n",number*5,content);
}
int is_decimal(char* op,int len,int cs){
    int i;
    if(cs){
        for(i=0;i<len;i++){ //10진수인지 확인
            if('0'<=op[i]&&op[i]<='9') continue;
            else
                return 0;
        }
    }
    return 1;
}
int is_char(char* op,int len,int cs){
    int i;
    if(cs == 1){// BYTE 일 때
        for(i=2;i<=len-2;i++){ //char형인지 확인
            if('A'<=op[i] && op[i]<='Z') continue;
            else if('0'<=op[i] && op[i]<='9') continue;
            else
                return 0;
        }
    }
    else{ // 나머지 경우
        for(i=0;i<len;i++){ //char형인지 확인
            if('A'<=op[i] && op[i]<='Z') continue;
            else
                return 0;
        }
    }
    return 1;
}
int direct_or_opcode(char* name){ // directive인 경우와 아닌경우를 나눔.
    if(strcmp(name,"BYTE")==0) return 1;
    else if(strcmp(name,"WORD")==0) return 2;
    else if(strcmp(name,"RESB")==0) return 3;
    else if(strcmp(name,"RESW")==0) return 4;
    else return 5;//opcode인 경우
}
int is_in_optab(char* name){ //optab에 있는지와, 있으면 그 형식을 찾는다.
    int e_bit=0, index;
    char format[10];
    Hash* ptr = (Hash*)malloc(sizeof(Hash));
    if(name[0] == '+'){
        sscanf(name,"+%s",name);
        e_bit = 1;
    }
    index = hash_function(name,strlen(name));
    if(index == -1) return 0; // 대문자 아닐시 오류처리
    for(ptr = h_table[index];ptr != NULL;ptr = ptr->next){
        if(strcmp(ptr->mnemonic,name) == 0){ //hash table에서 해당 mnemonic 찾으면
            strcpy(format,ptr->format);
            if(strcmp(format,"1")==0) return 1;
            if(strcmp(format,"2")==0) return 2;
            if(strcmp(format,"3/4")==0 && e_bit==0) return 3;
            if(strcmp(format,"3/4")==0 && e_bit==1) return 4;
        }
    }
    if(ptr == NULL) return 0; //못찾았을시 0 return
    return 1;
}
int add_to_symtab(char* symbol,int location){
    Syb *ptr = (Syb*)malloc(sizeof(Syb));
    Syb *new_sym = (Syb*)malloc(sizeof(Syb));
    int first_l = (int)(symbol[0]) - 65; //symbol의 첫글자
    symbol_num++;
    //if((int)strlen(symbol) > 6) return 0;
    if(25<first_l || first_l<0) return 0; //첫글자 소문자이거나 숫자인 경우 예외처리
    
    new_sym->next = NULL; new_sym->location = location;
    strcpy(new_sym->symbol,symbol);
    if(symtab[first_l] == NULL)
        symtab[first_l] = new_sym;
    else{ // 새로운 symbol 넣을때 오름차순으로 넣음. 0~25 index는 알파벳 A ~ Z를 뜻하고, symbol의 첫글자에 따라 저장됨.
        if(strcmp(symtab[first_l]->symbol,symbol)>0){
            ptr = symtab[first_l];
            new_sym->next = ptr;
            symtab[first_l] = new_sym;
        }
        else if(strcmp(symtab[first_l]->symbol,symbol)==0) return 0; //pass1인데 같은 symbol 발견되면 오류
        else{
            if(strcmp(symtab[first_l]->symbol,symbol)<0){
                if(symtab[first_l]->next == NULL){
                    symtab[first_l]->next = new_sym; return 1;
                }
            }
            for(ptr=symtab[first_l];ptr->next !=NULL;ptr = ptr->next){ //첫글자 같은 symbol들과 순서비교하여 넣음.
                if(strcmp(ptr->next->symbol,symbol)==0) return 0; //pass1인데 같은 symbol 발견되면 오류
                else if(strcmp(ptr->next->symbol,symbol)<0){
                    if(ptr->next->next == NULL){
                        ptr->next->next = new_sym; break;
                    }
                    else continue;
                }
                else if(strcmp(ptr->next->symbol,symbol)>0){
                    new_sym->next = ptr->next;
                    ptr->next = new_sym; break;
                }
            }
        }
    }
    return 1;
}
int pass1(char* name){ // pass1 수행
    char line[700], front[30], mid[30], back[30], mnemonic[30], operand[30];
    int s_f=0,e_f=0,location=0,len_o,dec;
    locctr = (int*)malloc(sizeof(int)*1000000);
    t_line = 0;
    FILE* file = fopen(name,"r");
    if(file == NULL){ //잘못된 file 일 때 0 return
        printf("File open error.\n");
        return 0;
    }
    for(;fgets(line,700,file)!=NULL;){ //.asm file 한 줄씩 읽음
        if(location > 0xffff){error_in_asm(line,t_line); return 0;}
        memset(front,'\0',sizeof(front)); memset(mid,'\0',sizeof(mid)); memset(back,'\0',sizeof(back));
        sscanf(line,"%s %s %s",front,mid,back);
        t_line += 1;
        if(e_f==0){ // END가 나오면 빠져나간다.
            if(strcmp(front,"END")==0){
                if(feof(file)){error_in_asm(line,t_line); return 0;}
                e_f=1;
                locctr[t_line] = location; break;
            }
        }
        if(s_f==0){ // 첫줄 START에 대한 처리
            s_f = 1;
            if(strcmp(front,"START")==0) program_name[0] = '\0';
            else if(strcmp(mid,"START")==0) sprintf(program_name,"%s",front);
            else {error_in_asm(line,t_line); return 0;}
            if(strlen(back))
                sscanf(line,"%s %s %X",front,mid,&location);
            locctr[t_line] = location;
            continue;
        }
        if(front[0] == '.' || strcmp(front,"BASE")==0){ //comment(주석)과 BASE에 대한 처리
            locctr[t_line] = locctr[1] - 1;
            continue;
        }
        else{ // comment, BASE, START, END 아닌 경우
            locctr[t_line] = location;
            if(line[0] != ' '&&line[0] != '\t'){ // symbol 읽었을 때
                if(!add_to_symtab(front,location)){ //pass1인데 symtab에 이미 동일 symbol있거나 label양식 틀린경우.
                    error_in_asm(line,t_line); return 0;}
                else{
                    strcpy(mnemonic,mid); strcpy(operand,back);
                }
            }
            else{
                strcpy(mnemonic,front); strcpy(operand,mid);
            }
            len_o = strlen(operand);
            switch(direct_or_opcode(mnemonic)){
                case 1: //BYTE인 경우
                    if(operand[0]=='C'){ // char인경우
                        if(operand[1]=='\''&& operand[len_o-1]=='\''){
                            if(is_char(operand,len_o,1)==0){
                                    error_in_asm(line,t_line); return 0;
                            }
                            location = location + (len_o - 3);
                        }
                        else{ //byte의 operand 형식이 틀렸을 경우
                            error_in_asm(line,t_line); return 0;
                        }
                    }
                    else if(operand[0]=='X'){ //16진수인경우
                        if(operand[1]=='\''&& operand[len_o-1]=='\''){
                            if(is_char(operand,len_o,1)==0){
                                error_in_asm(line,t_line); return 0;
                            }
                            if((len_o-3)%2 != 0){
                                error_in_asm(line,t_line); return 0;
                            }
                            else  location = location + ((len_o - 3)/2);
                        }
                        else{ //byte의 operand 형식이 틀렸을 경우
                            error_in_asm(line,t_line); return 0;
                        }
                    }
                    break;
                case 2: //WORD인 경우
                    if(!is_decimal(operand,len_o,2)){
                        error_in_asm(line,t_line); return 0;
                    }
                    else location = location + 3;
                    break;
                case 3: //RESB인 경우
                    if(!is_decimal(operand,len_o,3)){
                        error_in_asm(line,t_line); return 0;
                    }
                    else{
                        sscanf(operand,"%d",&dec);
                        location = location + dec;
                    }
                    break;
                case 4: //RESW인 경우
                    if(!is_decimal(operand,len_o,3)){
                        error_in_asm(line,t_line); return 0;
                    }
                    else{
                        sscanf(operand,"%d",&dec);
                        location = location + (dec * 3);
                    }
                    break;
                case 5://opcode인 경우
                    location += is_in_optab(mnemonic);
                    if(!is_in_optab(mnemonic)){
                        error_in_asm(line,t_line); return 0;
                    }
                    break;
                default : break;
            }
        }
    }
    if(e_f*s_f == 0){
        printf("[Assemble error] : there is no START or END");
        return 0;
    }
    fclose(file);
    return 1;
}
int pass2(char* name){
    FILE* file = fopen(name,"r");
    Mode info,info2;
    objcode = (Obj*)malloc(sizeof(Obj)*(t_line+1));
    char front[30], mid1[30], mid2[30], back[30];
    char line[700], mnemonic[30], operand1[30], operand2[30];
    int l_num=0,len_o1,len_o2,hex,hex_len,i,j;
    int pc,base=0,op_form,op_code;
    
    for(;fgets(line,700,file)!=NULL;){ //.asm file 한 줄씩 읽음
        memset(front,'\0',sizeof(front)); memset(mid1,'\0',sizeof(mid1));
        memset(back,'\0',sizeof(back)); memset(mid2,'\0',sizeof(mid2));
        sscanf(line,"%s %s %s %s",front,mid1,mid2,back);
        l_num += 1; hex = 0;
        if(strcmp(front,"END")==0){ objcode[l_num].format = -1;break;} //END 나오면 빠져나간다.
        if(strcmp(front,"START")==0 || strcmp(mid1,"START")==0){objcode[l_num].format = 0;}
        pc = locctr[l_num+1];
        if(pc < locctr[1]){
            i = l_num;
            do{
                i++;
                pc = locctr[i];
            }while(pc<0);
        }
        if(line[0] != ' '&&line[0] != '\t'){ // symbol 읽었을 때
            strcpy(mnemonic,mid1); strcpy(operand1,mid2); strcpy(operand2,back);
        }
        else{
            strcpy(mnemonic,front); strcpy(operand1,mid1); strcpy(operand2,mid2);
        }
        if(line[0] == '.'){objcode[l_num].format = -1; continue;} //coment일 때
        len_o1 = strlen(operand1); len_o2 = strlen(operand2);
        switch(direct_or_opcode(mnemonic)){
            case 1: // BYTE 일 때
                if(operand1[0] == 'X'){ //16진수 일 때
                    hex_len = (len_o1 - 3) / 2;
                    objcode[l_num].format = hex_len;
                    sscanf(operand1,"X'%X",&hex);
                    for(i=0;i<hex_len;i++){ //16진수에 대해 byte 만큼 right shift 시켜 저장.
                        objcode[l_num].code[i] = (hex>>8*(hex_len-1-i))&(0xff);
                    }
                }
                else if(operand1[0] == 'C'){ // character 일 때
                    objcode[l_num].format = len_o1 - 3;
                    for(i=2,j=0;i<len_o1-1;i++,j++){
                        objcode[l_num].code[j] = operand1[i];
                    }
                }
                break;
            case 2: // WORD 일 때
                objcode[l_num].format = 3;
                sscanf(operand1,"%d",&hex);
                for(i=0;i<3;i++){ //각 자리에 대해 byte 만큼 right shift 시켜 저장.
                    objcode[l_num].code[i] = (hex>>8*(2-i))&(0xff);
                }
                break;
            case 3: // RESB 일 때
                objcode[l_num].format = -7;
                break;
            case 4: // RESW 일 때
                objcode[l_num].format = -7;
                break;
            case 5: //BASE or opcode인 경우
                if(strcmp(mnemonic,"BASE")==0){ //Base인 경우
                    objcode[l_num].format = -1;
                    base = find_from_symtab(operand1);
                    if(0>base){
                        error_in_asm(line,l_num); return 0;
                    }
                }
                else{
                    op_form = is_in_optab(mnemonic);
                    op_code = opcode(mnemonic);
                    if(op_form == 3 || op_form == 4){ //format 3, 4
                        objcode[l_num].code[0] = op_code; //첫 byte에 opcode 들어감.
                        objcode[l_num].format = op_form;
                        if(!len_o1){ // operand 없을때
                            for(i=1;i<=3;i++)
                                objcode[l_num].code[i] = 0;
                            objcode[l_num].code[0] += 3; // n=1, i=1 ->simple addressing
                        }
                        else if(!len_o2){ //operand 1개 일 때
                            info = find_addressing(operand1);
                            if(info.address_mode == 'f' || info.address_mode == 'd'){
                                error_in_asm(line,l_num); return 0;
                            }
                            if(op_form == 4){ //4형식
                                if(info.address_mode == 'b'){ //immediate인데 숫자 나왔을때
                                    objcode[l_num].code[0] += info.ni; // n=0, i=1 ->immediate addressing
                                    for(i=1;i<=3;i++) // 각 byte별로 shift연산을 통해 대입.
                                        objcode[l_num].code[i] = (info.address>>8*(3-i))&(0xff);
                                    objcode[l_num].code[1] += 16; // e = 1 -> format 4
                                    
                                    objcode[l_num].flag = 7;
                                }
                                else{ // simple or indirect addressing
                                    objcode[l_num].code[0] += info.ni; // 각 case의 n,i bit 덧셈.
                                    for(i=1;i<=3;i++) // 각 byte별로 shift연산을 통해 대입.
                                        objcode[l_num].code[i] = (info.address>>8*(3-i))&(0xff);
                                    objcode[l_num].code[1] += 16; // e = 1 (1 0000)b-> format 4
                                }
                            }
                            else if(op_form == 3){ //3형식
                                if(info.address_mode == 'b'){ //immediate인데 숫자 나왔을때
                                    if(info.address>0xfff){
                                        error_in_asm(line,l_num); return 0;
                                    }
                                    objcode[l_num].code[0] += info.ni; // n=0, i=1 ->immediate addressing
                                    for(i=1;i<=2;i++) // 각 byte별로 shift연산을 통해 대입.
                                        objcode[l_num].code[i] = (info.address>>8*(2-i))&(0xff);
                                    
                                    objcode[l_num].flag = 7;
                                }
                                else{ // operand가 symbol 이거나 register 일 때
                                    objcode[l_num].code[0] += info.ni; // 각 case의 n,i bit 덧셈.
                                    hex = info.address - pc;
                                    if(-2048<=hex && hex<=2047){
                                        for(i=1;i<=2;i++) // 각 byte별로 shift연산을 통해 대입.
                                            objcode[l_num].code[i] = (hex>>8*(2-i))&(0xff);
                                        objcode[l_num].code[1] += 32; // p=1 (10 0000)b->pc relative
                                        if(hex<0){
                                            objcode[l_num].code[1] += 16;
                                        }
                                    }
                                    else{
                                        hex = info.address - base;
                                        if(0<=hex && hex<=4095){
                                            for(i=1;i<=2;i++) // 각 byte별로 shift연산을 통해 대입.
                                                objcode[l_num].code[i] = (hex>>8*(2-i))&(0xff);
                                            objcode[l_num].code[1] += 64; // b=1 (100 0000)b->base relative
                                        }
                                        else{
                                            error_in_asm(line,l_num); return 0;
                                        }
                                    }
                                }
                            }
                        }
                        else if(strlen(back)==0){ //operand 2개 일 때 3형식,4형식 모두 index bit set
                            if(operand1[len_o1-1] == ',') operand1[len_o1-1] = '\0';
                            else{
                                error_in_asm(line,l_num); return 0;
                            }
                            info = find_addressing(operand1);
                            if(info.address_mode == 'f' || info.address_mode == 'd'){ //fail이거나 operand하나일때 error
                                error_in_asm(line,l_num); return 0;
                            }
                            if(op_form == 4){
                                if(info.address_mode == 's'){ //operand 2개이면서 4형식인것은 simple&index addressing이다.
                                    objcode[l_num].code[0] += info.ni; // n=1, i=1 ->simple addressing
                                    for(i=1;i<=3;i++) // 각 byte별로 shift연산을 통해 대입.
                                        objcode[l_num].code[i] = (info.address>>8*(3-i))&(0xff);
                                    objcode[l_num].code[1] += 16; // e = 1 -> format 4
                                    objcode[l_num].code[1] += 128; // x = 1 -> index addressing mode
                                }
                                else{
                                    error_in_asm(line,l_num); return 0;
                                }
                            }
                            else if(op_form == 3){
                                if(info.address_mode == 'b'){ //immediate인데 숫자 나왔을때
                                    if(info.address>0xfff){
                                        error_in_asm(line,l_num); return 0;
                                    }
                                    objcode[l_num].code[0] += info.ni; // n=0, i=1 ->immediate addressing
                                    for(i=1;i<=2;i++) // 각 byte별로 shift연산을 통해 대입.
                                        objcode[l_num].code[i] = (info.address>>8*(2-i))&(0xff);
                                    objcode[l_num].code[1] += 128; // x = 1 -> index addressing mode
                                    
                                }
                                else{ // operand가 symbol 이거나 register 일 때
                                    objcode[l_num].code[0] += info.ni; // 각 case의 n,i bit 덧셈.
                                    hex = info.address - pc;
                                    if(-2048<=hex && hex<=2047){
                                        for(i=1;i<=2;i++) // 각 byte별로 shift연산을 통해 대입.
                                            objcode[l_num].code[i] = (hex>>8*(2-i))&(0xff);
                                        objcode[l_num].code[1] += 32; // p=1 (10 0000)b->pc relative
                                        objcode[l_num].code[1] += 128; // x = 1 -> index addressing mode
                                        if(hex<0){
                                            objcode[l_num].code[1] += 16;
                                        }
                                    }
                                    else{
                                        hex = info.address - base;
                                        if(0<=hex && hex<=4095){
                                            for(i=1;i<=2;i++) // 각 byte별로 shift연산을 통해 대입.
                                                objcode[l_num].code[i] = (hex>>8*(2-i))&(0xff);
                                            objcode[l_num].code[1] += 64; // b=1 (100 0000)b->base relative
                                            objcode[l_num].code[1] += 128; // x = 1 -> index addressing mode
                                        }
                                        else{
                                            error_in_asm(line,l_num); return 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if(op_form == 2){
                        objcode[l_num].code[0] = op_code;
                        objcode[l_num].format = op_form;
                        if(!len_o1){error_in_asm(line,l_num); return 0;}
                        else if(!len_o2){ //operand 1
                            info = find_addressing(operand1);
                            if(info.ni != 0){error_in_asm(line,l_num); return 0;}
                            objcode[l_num].code[1] = info.address*16; // r1 half byte left shift
                        }
                        else if(strlen(back)==0){ //operand 2
                            if(operand1[len_o1-1] == ',') operand1[len_o1-1] = '\0';
                            else{
                                error_in_asm(line,l_num); return 0;
                            }
                            info = find_addressing(operand1);
                            info2 = find_addressing(operand2);
                            if(info.ni != 0 || info2.ni != 0){error_in_asm(line,l_num); return 0;}
                            objcode[l_num].code[1] = info.address*16 + info2.address; // r1 half byte left shift and plus r2
                        }
                        else{
                            error_in_asm(line,l_num); return 0;
                        }
                    }
                    else if(op_form == 1){
                        if(len_o1){error_in_asm(line,l_num); return 0;}
                        objcode[l_num].code[0] = op_code;
                        objcode[l_num].format = op_form;
                    }
                }
                break;
            default: break;
        }
    }
    return 1;
}
Mode find_addressing(char* name){ // name이 register인지 symbol인지 알아내고, 각 addressing mode를 알아내, 주소 반환.
    int sym_add,immd;
    Mode res;
    if(strcmp(name,"A")==0) { //아래로 register인 경우
        res.address = 0; res.address_mode = 'd'; res.ni=0; return res;
    }
    else if(strcmp(name,"X")==0){
        res.address = 1; res.address_mode = 'd'; res.ni=0; return res;
    }
    else if(strcmp(name,"L")==0){
        res.address = 2; res.address_mode = 'd'; res.ni=0; return res;
    }
    else if(strcmp(name,"B")==0){
        res.address = 3; res.address_mode = 'd'; res.ni=0; return res;
    }
    else if(strcmp(name,"S")==0){
        res.address = 4; res.address_mode = 'd';res.ni=0;  return res;
    }
    else if(strcmp(name,"T")==0){
        res.address = 5; res.address_mode = 'd'; res.ni=0; return res;
    }
    else if(strcmp(name,"F")==0){
        res.address = 6; res.address_mode = 'd'; res.ni=0; return res;
    }
    else if(strcmp(name,"PC")==0){
        res.address = 8; res.address_mode = 'd'; res.ni=0; return res;
    }
    else if(strcmp(name,"SW")==0){
        res.address = 9; res.address_mode = 'd'; res.ni=0; return res;
    }
    else{
        if(name[0] == '#'){ //immediate addressing인 경우.
            sscanf(name,"#%s",name);
            res.address_mode = 'i'; res.ni=1;
        }
        else if(name[0] == '@'){ // indirect addressing인 경우.
            sscanf(name,"@%s",name);
            res.address_mode = 'n'; res.ni=2;
        }
        else { res.address_mode = 's'; res.ni=3; }
    }
    sym_add = find_from_symtab(name);
    if(sym_add>0){ //immediate 나 indirect 인데 operand symbol일 때 각각 'i', 'n'
        res.address = sym_add;
    }
    else{
        if(res.address_mode == 'i'){ //indirect addressing인데 operand 숫자일 경우.
            sscanf(name,"%d",&immd);
            if(0xfffff<immd || immd <0){ //4형식 20bit 주소 범위까지 넘었을 때
                res.address_mode = 'f'; //error (fail)
                return res;
            }
            res.address = immd; res.ni=1;
            res.address_mode = 'b'; //immediate인데 숫자일때 'b'
            return res;
        }
        else{
            res.address_mode = 'f'; //symtab에도 없고, immediate도 아니면 error
            return res;
        }
    }
    return res;
}
int find_from_symtab(char* name){ //symtab에 있는지 찾고 있으면 location 반환, 없으면 0 반환.
    int first_l = (int)(name[0]) - 65;
    Syb* ptr = symtab[first_l];
    for(;ptr != NULL;ptr = ptr->next){
        if(strcmp(name,ptr->symbol)==0){
            return ptr->location;
        }
    }
    return -1000;
}
void free_symtab(){ // SYMTAB 동적할당 해제
    int i;
    Syb *ptr;
    for(i = 0;i<26;i++){
        if(symtab[i] != NULL){
            for(;symtab[i] != NULL;){
                ptr = symtab[i];
                symtab[i] = symtab[i]->next;
                free(ptr);
                
            }
        }
    }
}
void store_symtab(){ // SYMTAB 내용 저장
    int i,s_index=0;
    location_arr = (int*)malloc(sizeof(int)*symbol_num);
    symbol_arr = (char**)malloc(sizeof(char*)*symbol_num);
    for(i=0;i<symbol_num;i++)
        symbol_arr[i] = (char*)malloc(sizeof(char)*30);
    Syb* ptr;
    if(success_asm == 1){
        for(i=0;i<26;i++){
            if(symtab[i] != NULL){
                for(ptr = symtab[i];ptr !=NULL;ptr = ptr->next){
                    location_arr[s_index] = ptr->location;
                    strcpy(symbol_arr[s_index++],ptr->symbol);
                }
            }
        }
    }
}
void symbol(){ //'symbol' 명령어 수행
    int i;
    if(success_asm == 0) return;
    else{
        for(i=0;i<symbol_num;i++){
            printf("\t%s\t%04X\n",symbol_arr[i],location_arr[i]);
        }
    }
}
