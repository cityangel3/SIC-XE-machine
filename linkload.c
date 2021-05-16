#include "20171700.h"
//헤더파일에서 extern 선언한 전역변수 definition//
int cs_addr;
int cs_lth;
int start_addr;
int end_addr;
Est* estab[26];
int start_run=-7;
int arr_bp[1000];
int check[65536][16] = {0,};
int num_bp=0;
int regA,regX,regL,regB,regS,regT,regPC,CC;


int loader(char *file1, char *file2, char*file3, int num){ //process1과 process2를 통해 link & load 수행
    int cs_st1=0,cs_st2=0,cs_st3=0;
    cs_addr = prog_addr;
    start_addr = prog_addr;
    free_estab();
    //형식 확인
    if(strcmp(file1 + strlen(file1)-4,".obj") != 0) //파일 형식 확인
        return 0;
    if((num >= 2) && (strcmp(file2 + strlen(file2)-4,".obj") != 0))
        return 0;
    if((num == 3) && (strcmp(file3 + strlen(file3)-4,".obj") != 0))
        return 0;
    switch(num){ //obj 파일의 개수에 따라 link 와 load를 수행함.
        case 1: //1개
            if(process1(file1) == 0) return 0;
            cs_st1 = cs_addr; cs_addr += cs_lth; end_addr = cs_addr;
            if(process2(file1,cs_st1) == 0) return 0;
            break;
        case 2: //2개
            if(process1(file1) == 0) return 0;
            cs_st1 = cs_addr; cs_addr += cs_lth;
            if(process1(file2) == 0) return 0;
            cs_st2 = cs_addr; cs_addr += cs_lth; end_addr = cs_addr;
            
            if(process2(file1,cs_st1) == 0) return 0;
            if(process2(file2,cs_st2) == 0) return 0;
            break;
        case 3: //3개
            if(process1(file1) == 0) return 0;
            cs_st1 = cs_addr; cs_addr += cs_lth;
            if(process1(file2) == 0) return 0;
            cs_st2 = cs_addr; cs_addr += cs_lth;
            if(process1(file3) == 0) return 0;
            cs_st3 = cs_addr; cs_addr += cs_lth; end_addr = cs_addr;
            
            if(process2(file1,cs_st1) == 0) return 0;
            if(process2(file2,cs_st2) == 0) return 0;
            if(process2(file3,cs_st3) == 0) return 0;
            break;
        default : break;
    }
    print_estab(); //로드맵 출력
    start_run = prog_addr; // load 성공시 run 시작점 및 각 register값 초기화
    regA = regX =  regB = regS = regT = 0;
    regPC = start_addr;
    regL = end_addr;
    return 1;
}
void print_estab(){ //로드맵을 출력함.
    Est* ptr;
    int i;
    printf("control symbol address length\n");
    printf("section name\n");
    printf("--------------------------------\n");
    for(i=0;i<26;i++){
        for(ptr = estab[i];ptr != NULL; ptr = ptr->next){
            if(ptr->ps == 'p')
                printf("%s\t\t  %04X   %04X\n",ptr->symbol_name,ptr->address,ptr->length);
            if(ptr->ps == 's')
                printf("\t %6s   %04X\n", ptr->symbol_name,ptr->address);
        }
    }
    printf("--------------------------------\n");
    printf("\t    total length %04X\n",(end_addr-prog_addr));
}
void free_estab(){ //estab을 동적 해제함.
    Est* ptr;
    int i;
    for(i=0;i<26;i++){
        if(estab[i] != NULL){
            for(;estab[i]!=NULL;){
                ptr=estab[i];
                estab[i] = estab[i]->next;
                free(ptr);
            }
        }
    }
}
int add_to_estab(char* name,int addr,int length,char ps){ //estab에 새로운 심볼 추가.
    sscanf(name,"%[^ ]",name); //공백 전까지의 문자로 다시 저장.
    name[strlen(name)] = '\0';
    int index = hash_value_estab(name); //hash value 가져옴
    Est* new = (Est*)malloc(sizeof(Est)), *ptr;
    new->address = addr + cs_addr; new->ps = ps;
    new->length = length; new->next = NULL;
    sprintf(new->symbol_name,"%s",name);
    if(estab[index] == NULL) //해당 index 비었으면 입력
        estab[index] = new;
    else{// 해당 index 가장 끝 node에 입력
        for(ptr = estab[index]; ptr->next !=NULL;ptr = ptr->next){
            if(strcmp(name,ptr->symbol_name) == 0) return 0;
        }
        ptr->next = new;
    }
    return 1;
}
int is_in_estab(char* name){ //estab에 해당 심볼이 있는지 확인.
    sscanf(name,"%[^ ]",name); //공백 전까지의 문자로 다시 저장.
    name[strlen(name)] = '\0';
    int index = hash_value_estab(name);
    Est *ptr = estab[index];
    for(;ptr !=NULL;ptr = ptr->next){ /*estab에 있으면 주소 반환*/       if(strcmp(name,ptr->symbol_name) == 0) return ptr->address;
    }//없으면 -1 반환
    return -1;
}
int hash_value_estab(char* name){
    int value,temp = (int)name[strlen(name)-1]; //가장 뒷자리의 문자로 hash value설정
    if(temp >= 65) value = (temp-65) %26;
    else value = temp % 26;
    return value;
}
int process1(char* file){ //pass1 과정
    char line[700],name[7],haddr[7]; // line은 한줄씩 받음. name은 program or symbol name
    char type; //레코드 type
    int addr,i;
    FILE* fp = fopen(file,"r");
    if(fp == NULL){ //잘못된 file 일 때 0 return
        printf("File open error.\n");
        return 0;
    }
    for(;fgets(line,700,fp)!=NULL;){ //.obj file 한 줄씩 읽음
        if(line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
        type = line[0]; i = 1;
        name[6] = '\0'; haddr[6] = '\0';
        if(type == 'H'){ //type 이 H일 때
            sscanf(line,"%c %06s %06X %06X",&type,name,&addr,&cs_lth);
            if(add_to_estab(name,addr,cs_lth,'p') == 0) return 0;
        }
        else if(type == 'D'){ //type 이 D일 때
            while(i < (int)strlen(line)){
                strncpy(name,line+i,6);
                strncpy(haddr,line+i+6,6);
                sscanf(haddr,"%06X",&addr);
                if(add_to_estab(name,addr,0,'s') == 0) return 0;
                i += 12;
            }
        }
        else break; //type이 T,M,R,E or comment 일 때 break
    }
    fclose(fp);
    return 1;
}
int process2(char* file,int cs_start){ //pass2과정
    char line[700],type;  // line은 한줄씩 받음. type은 레코드 type
    char byte[3],threebyte[7],ref[3];
    int baddr,taddr,raddr,otcur,incur;
    int ref_num[200],row,col,maddr,modif,temp;
    ref_num[1] = cs_start;
    FILE* fp = fopen(file,"r");
    if(fp == NULL){ //잘못된 file 일 때 0 return
        printf("File open error.\n");
        return 0;
    }
    for(;fgets(line,700,fp)!=NULL;){ //.obj file 한 줄씩 읽음
        if(line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
        memset(threebyte,' ',7); memset(byte,' ',3); memset(ref,' ',3);
        type = line[0]; otcur = 1; threebyte[6] = '\0'; byte[2] = '\0'; ref[2]='\0';
        if(type == 'E'){ //type이 E일 때
            sscanf(line,"%c %s",&type,threebyte);
            if(threebyte[0] != ' '){
                sscanf(threebyte,"%06X",&taddr);
                start_addr = taddr + cs_start;
            }
        }
        if(type == 'R'){ //type이 R일 때
            while(otcur < (int)strlen(line)){
                strncpy(byte,line+otcur,2);
                sscanf(byte,"%02X",&baddr);
                strncpy(threebyte,line+otcur+2,6);
                taddr = is_in_estab(threebyte);
                if(taddr == -1) return 0; //estab에 없으면 error
                ref_num[baddr] = taddr;
                otcur += 8;
            }
        }
        if(type == 'T'){ //type이 T일 때 해당 object 코드를 메모리에 올림.
            strncpy(threebyte,line+1,6);
            sscanf(threebyte,"%06X",&taddr);
            otcur = 9; incur = 0;
            while(otcur < (int)strlen(line)){
                strncpy(byte,line+otcur,2);
                sscanf(byte,"%02X",&baddr);
                maddr = cs_start + taddr + incur;
                row = maddr / 16; col = maddr % 16;
                virtual_mem[row][col] = baddr;
                incur += 1; otcur += 2;
            }
        }
        if(type == 'M'){ //type이 M일 때 해당 메모리에 접근하여 값을 수정함.
            strncpy(threebyte,line+1,6);
            sscanf(threebyte,"%06X",&taddr);
            maddr = taddr + cs_start;
            row = maddr / 16; col = maddr % 16;
            temp = (virtual_mem[row][col])&(0x0F);
            strncpy(byte,line+1+6,2);
            sscanf(byte,"%02X",&baddr);
            strncpy(ref,line+1+6+2+1,2);
            sscanf(ref,"%02X",&raddr);
            if(baddr == 5){
                modif = (temp<<16);
                for(otcur=1;otcur<3;otcur++){
                    if((col+otcur) >= 16){
                        modif += (virtual_mem[row+1][col+otcur-16] << (2-otcur)*8);
                    }
                    else
                        modif += (virtual_mem[row][col+otcur] << (2-otcur)*8);
                }
                if(line[9] == '+') modif += ref_num[raddr];
                if(line[9] == '-') modif -= ref_num[raddr];
                modif = modif & (0xFFFFFF);
                virtual_mem[row][col] = virtual_mem[row][col] - temp + (modif>>16);
                for(otcur=1;otcur<3;otcur++){
                    if((col+otcur) >= 16){
                        virtual_mem[row+1][col+otcur-16]=(modif >> (2-otcur)*8)&(0xFF);
                    }
                    else
                        virtual_mem[row][col+otcur] =(modif >> (2-otcur)*8)&(0xFF);
                }
            }
            if(baddr == 6){
                modif = (virtual_mem[row][col]<<16);
                for(otcur=1;otcur<3;otcur++){
                    if((col+otcur) >= 16){
                        modif += (virtual_mem[row+1][col+otcur-16] << (2-otcur)*8);
                    }
                    else
                        modif += (virtual_mem[row][col+otcur] << (2-otcur)*8);
                }
                if(line[9] == '+') modif += ref_num[raddr];
                if(line[9] == '-') modif -= ref_num[raddr];
                modif = modif & (0xFFFFFF);
                virtual_mem[row][col] = (modif>>16)&(0xFF);
                for(otcur=1;otcur<3;otcur++){
                    if((col+otcur) >= 16){
                        virtual_mem[row+1][col+otcur-16]=(modif >> (2-otcur)*8)&(0xFF);
                    }
                    else
                        virtual_mem[row][col+otcur] =(modif >> (2-otcur)*8)&(0xFF);
                }
            }
        }
        else continue; //type이 H, D or comment일 때 skip
    }
    fclose(fp);
    return 1;
}
void bp_process(int bp){ // break point값을 설정하거나 clear, 출력함.
    int i,row,col,temp;
    if(bp == -7){ // bp
        printf("\t breakpoint\n");
        printf("\t ----------\n");
        for(i=0;i<num_bp;i++)
            printf("\t %-X\n",arr_bp[i]);
    }
    if(bp == -3){ // bp clear
        for(i=0;i<num_bp;i++){
            temp = arr_bp[i];
            row = temp / 16; col = temp % 16;
            check[row][col] = 0;
            arr_bp[i] = 0;
        }
        num_bp = 0;
        printf("\t     [ok] clear all breakpoints\n");
    }
    if(bp >= 0){ // bp address
        row = bp / 16; col = bp % 16;
        if(check[row][col]){
            printf("Redundancy\n");
        }
        else{
            arr_bp[num_bp++] = bp;
            check[row][col] = 1;
            printf("\t     [ok] create breakpoint %-x\n",bp);
        }
    }
}
void print_reg(){ //run 수행 후 출력
    printf("A : %06X  X : %06X\n", regA, regX);
    printf("L : %06X PC : %06X\n", regL, regPC);
    printf("B : %06X  S : %06X\n", regB, regS);
    printf("T : %06X\n", regT);
}
int run(){
    if(start_run < 0){ printf("didn't loaded\n"); return 0;}
    int op_num,ni,xbpe,addr,val,f_addr,f_val,temp1,temp2,reg1,reg2;
    int row,col,row1,row2,row3,col1,col2,col3,trow,tcol;
    int incur,otcur = start_run;
    for(;otcur < start_addr;otcur++){
        row = otcur / 16; col = otcur % 16;
        if(check[row][col]){
            print_reg(); start_run = otcur + 1;
            printf("\t  Stop at checkpoint[%X]\n",otcur);
            return 1;
        }
    }
    for(;otcur < end_addr;){ //break point전까지 프로그램 실행(없을시 끝까지).
        row = otcur / 16; col = otcur % 16;
        if((col + 3) >= 16) {row3 = row +1; col3 = col + 3 - 16;}
        else{row3 = row; col3 = col + 3;}
        if((col + 2) >= 16) {row2 = row +1; col2 = col + 2 - 16;}
        else{row2 = row; col2 = col + 2;}
        if((col + 1) >= 16) {row1 = row +1; col1 = col + 1 - 16;}
        else{row1 = row; col1 = col + 1;}
        val = addr = f_val = f_addr = 0;
        ni = (virtual_mem[row][col]&(0x03));
        op_num = virtual_mem[row][col] - ni;
        xbpe = (virtual_mem[row1][col1]>>4);
        if((xbpe % 2) == 0){ // format 3
            addr = ((virtual_mem[row1][col1]&(0x0F))<<8) + virtual_mem[row2][col2];
            if(xbpe&8) addr = addr + regX; //x index
            if(xbpe&4){//base relative
                if(addr&0x800){ // 주소가 음수일 때
                    addr = ~addr; addr += 1;
                    addr &= 0xFFF; addr *= -1;
                }
                addr = addr + regB;
            }
            else if(xbpe&2){//pc relative
                if(addr&0x800){ // 주소가 음수일 때
                    addr = ~addr; addr += 1;
                    addr &= 0xFFF; addr *= -1;
                }
                addr = addr + regPC + 3; // pc는 다음줄을 가르킨 상태였음.
            }
            if(ni == 3){ //simple addressing
                trow = addr / 16; tcol = addr % 16;
                val = (virtual_mem[trow][tcol]<<16);
                for(incur = 1;incur < 3;incur++){ //주소에 있는 값 저장
                    if((tcol + incur) >= 16)
                    { val += virtual_mem[trow+1][tcol+incur-16]<<((2-incur)*8);}
                    else
                    { val += virtual_mem[trow][tcol+incur]<<((2-incur)*8);}
                }
            }
            if(ni == 2){ //indirect addressing
                trow = addr / 16; tcol = addr % 16;
                addr = (virtual_mem[trow][tcol]<<16);
                for(incur = 1;incur < 3;incur++){ // 해당 주소에 있는 값이 다시 주소가됨.
                    if((tcol + incur) >= 16)
                    { addr += virtual_mem[trow+1][tcol+incur-16]<<((2-incur)*8);}
                    else
                    { addr += virtual_mem[trow][tcol+incur]<<((2-incur)*8);}
                }
                trow = addr / 16; tcol = addr % 16;
                val = (virtual_mem[trow][tcol]<<16);
                for(incur = 1;incur < 3;incur++){ // 주소에 있는 값 저장.
                    if((tcol + incur) >= 16)
                    { val += virtual_mem[trow+1][tcol+incur-16]<<((2-incur)*8);}
                    else
                    { val += virtual_mem[trow][tcol+incur]<<((2-incur)*8);}
                }
            }
            if(ni == 1) // immediate addressing
                val = addr;
        }
        if((xbpe % 2) == 1){ // format 4
            f_addr = ((virtual_mem[row1][col1]&(0x0F))<<16) + (virtual_mem[row2][col2] << 8) + virtual_mem[row3][col3];
            if(ni == 3){ //simple addressing
                trow = f_addr / 16; tcol = f_addr % 16;
                f_val = (virtual_mem[trow][tcol]<<16);
                for(incur = 1;incur < 3;incur++){ //주소에 있는 값 저장
                    if((tcol + incur) >= 16)
                    { f_val += virtual_mem[trow+1][tcol+incur-16]<<((2-incur)*8);}
                    else
                    { f_val += virtual_mem[trow][tcol+incur]<<((2-incur)*8);}
                }
            }
            if(ni == 2){ //indirect addressing
                trow = f_addr / 16; tcol = f_addr % 16;
                f_addr = (virtual_mem[trow][tcol]<<16);
                for(incur = 1;incur < 3;incur++){ // 해당 주소에 있는 값이 다시 주소가됨.
                    if((tcol + incur) >= 16)
                    { f_addr += virtual_mem[trow+1][tcol+incur-16]<<((2-incur)*8);}
                    else
                    { f_addr += virtual_mem[trow][tcol+incur]<<((2-incur)*8);}
                }
                trow = f_addr / 16; tcol = f_addr % 16;
                f_val = (virtual_mem[trow][tcol]<<16);
                for(incur = 1;incur < 3;incur++){ // 주소에 있는 값 저장.
                    if((tcol + incur) >= 16)
                    { f_val += virtual_mem[trow+1][tcol+incur-16]<<((2-incur)*8);}
                    else
                    { f_val += virtual_mem[trow][tcol+incur]<<((2-incur)*8);}
                }
            }
            if(ni == 1) // immediate addressing
                f_val = f_addr;
        }
        //아래는 각 opcode에 따라서 해당 동작을 수행함.
        if(op_num == 160) { //compr
            temp1 = virtual_mem[row1][col1]/16;
            temp2 = virtual_mem[row1][col1]&(0x0F);
            reg1 = reg_num(temp1,0); reg2 = reg_num(temp2,0);
            if(reg1 < reg2) CC = 1; // less than '<'
            else if(reg1 == reg2) CC = 2; // equal '='
            else CC = 3; // bigger than '>'
            regPC = otcur+2;
        }
        else if(op_num == 184) { //tixr
            regX += 1;
            temp1 = virtual_mem[row1][col1]/16;
            reg1 = reg_num(temp1,0);
            if(regX < reg1) CC = 1; // less than '<'
            else if(regX == reg1) CC = 2; // equal '='
            else CC = 3; // bigger than '>'
            regPC = otcur+2;
        }
        else if(op_num == 0){//lda
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = val;}
            else{ regPC = otcur+4; temp1 = f_val;}
            regA = temp1;
        }
        else if(op_num == 104){ //ldb
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = val;}
            else{ regPC = otcur+4; temp1 = f_val;}
            regB = temp1;
        }
        else if(op_num == 80){//ldch
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = val;}
            else{ regPC = otcur+4; temp1 = f_val;}
            regA -= (regA & (0xFF)); regA += (temp1>>16);
        }
        else if(op_num == 116){ //ldt
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = val;}
            else{ regPC = otcur+4; temp1 = f_val;}
            regT = temp1;
        }
        else if(op_num == 180){ //clear
            temp1 = virtual_mem[row1][col1]/16;
            reg_num(temp1,1); //erase
            regPC = otcur+2;
        }
        else if(op_num == 40){//comp
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = val;}
            else{ regPC = otcur+4; temp1 = f_val;}
            if(regA < temp1) CC = 1; // less than '<'
            else if(regA == temp1) CC = 2; // equal '='
            else CC = 3; // bigger than '>'
        }
        else if(op_num == 60){ //j
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = addr;}
            else{ regPC = otcur+4; temp1 = f_addr;}
            regPC = temp1;
        }
        else if(op_num == 48){//jeq
            if((xbpe%2) == 0){ regPC = otcur+3;}
            else{ regPC = otcur+4;}
            if(CC == 2){ //equal
                if((xbpe%2) == 0){temp1 = addr;}
                else{temp1 = f_addr;}
                regPC = temp1;
            }
        }
        else if(op_num == 56){//jlt
            if((xbpe%2) == 0){ regPC = otcur+3;}
            else{ regPC = otcur+4;}
            if(CC == 1){ //less than
                if((xbpe%2) == 0){temp1 = addr;}
                else{temp1 = f_addr;}
                regPC = temp1;
            }
        }
        else if(op_num == 72){ //jsub
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = addr;}
            else{ regPC = otcur+4; temp1 = f_addr;}
            regL = regPC; regPC = temp1;
        }
        else if(op_num == 12){ //sta
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = addr;}
            else{ regPC = otcur+4; temp1 = f_addr;}
            trow = temp1 / 16; tcol = temp1 % 16;
            virtual_mem[trow][tcol] = (regA>>16);
            for(incur=1;incur<3;incur++){
                if(tcol+incur >= 16)
                { virtual_mem[trow+1][tcol+incur-16] = (regA>>((2-incur)*8))&(0xFF);}
                else
                { virtual_mem[trow][tcol+incur] = (regA>>((2-incur)*8))&(0xFF);}
            }
        }
        else if(op_num == 224){ //td
            if((xbpe%2) == 0){ regPC = otcur+3;}
            else{ regPC = otcur+4;}
            CC = 1; // less than '<'
        }
        else if(op_num == 216){ //rd
            if((xbpe%2) == 0){ regPC = otcur+3;}
            else{ regPC = otcur+4;}
            regA = 0;
        }
        else if(op_num == 220){ //wd
            if((xbpe%2) == 0){ regPC = otcur+3;}
            else{ regPC = otcur+4;}
        }
        else if(op_num == 76){ //rsub
            if((xbpe%2) == 0){ regPC = otcur+3;}
            else{ regPC = otcur+4;}
            regPC = regL;
        }
        else if(op_num == 84){ //stch
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = addr;}
            else{ regPC = otcur+4; temp1 = f_addr;}
            trow = temp1 / 16; tcol = temp1 % 16;
            virtual_mem[trow][tcol] = (regA&(0xFF));
        }
        else if(op_num == 20){ //stl
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = addr;}
            else{ regPC = otcur+4; temp1 = f_addr;}
            trow = temp1 / 16; tcol = temp1 % 16;
            virtual_mem[trow][tcol] = (regL>>16);
            for(incur=1;incur<3;incur++){
                if(tcol+incur >= 16)
                { virtual_mem[trow+1][tcol+incur-16] = (regL>>((2-incur)*8))&(0xFF);}
                else
                { virtual_mem[trow][tcol+incur] = (regL>>((2-incur)*8))&(0xFF);}
            }
        }
        else if(op_num == 16){ //stx
            if((xbpe%2) == 0){ regPC = otcur+3; temp1 = addr;}
            else{ regPC = otcur+4; temp1 = f_addr;}
            trow = temp1 / 16; tcol = temp1 % 16;
            virtual_mem[trow][tcol] = (regX>>16);
            for(incur=1;incur<3;incur++){
                if(tcol+incur >= 16)
                { virtual_mem[trow+1][tcol+incur-16] = (regX>>((2-incur)*8))&(0xFF);}
                else
                { virtual_mem[trow][tcol+incur] = (regX>>((2-incur)*8))&(0xFF);}
            }
        }
        else regPC += 1;
        
        if(regPC < end_addr){
            trow = (regPC) / 16; tcol = (regPC) % 16;
            if(check[trow][tcol]){ //pc 레지스터 값이 bp가 되면 중단함.
                print_reg();
                printf("\t  Stop at checkpoint[%X]\n",regPC);
                start_run = regPC;
                return 1;
            }
        }
        otcur = regPC;
    }
    print_reg();
    printf("\t  End Program\n");
    start_run = prog_addr; // register값 초기화
    regA = regX =  regB = regS = regT = 0;
    regPC = start_addr;
    regL = end_addr;
    return 1;
}
int reg_num(int num,int erase){ //num값에 해당하는 레지스터 값 반환. erase=1이면 해당 레지스터 값 clear
    if(erase == 0){
        switch(num){
            case 0: return regA; break;
            case 1: return regX; break;
            case 2: return regL; break;
            case 3: return regB; break;
            case 4: return regS; break;
            case 5: return regT; break;
            case 8: return regPC; break;
            default: break;
        }
    }
    else{ // 'clear'
        switch(num){
            case 0: regA = 0; break;
            case 1: regX = 0; break;
            case 2: regL = 0; break;
            case 3: regB = 0; break;
            case 4: regS = 0; break;
            case 5: regT = 0; break;
            case 8: regPC = 0; break;
            default: break;
        }
    }
    return -1;
}
