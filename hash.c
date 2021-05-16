#include "20171700.h"
//헤더파일에서 extern 선언한 전역변수 definition//
Hash* h_table[20];

int hash_function(char* key,unsigned long max){ // mod 연산등을 활용하여 hash table 주소 결정
    unsigned long i;
    int temp=0,address=0,h,t;
    //mnemonic의 각 character를 ASCII값으로 더해준다
    for(i = 0;i < max;i++){
        if(90<key[i] || key[i]<65)
            return -1;
        temp += (int)key[i];
    }
    //더한 값을 제곱 후 백의자리와 십의자리만 가져와 hash table의 크기로 mod 연산 수행한다
    temp = (temp*temp) % 1000;
    h = temp/100;
    temp %= 100;
    t = temp/10;
    address = (h*10 + t) % 20;
    return address;
}
int read_opcode(void){
    FILE* fp = fopen("opcode.txt","r");
    if(fp == NULL) return 0; //못 읽었을시 0 return
    Hash* ptr = (Hash*)malloc(sizeof(Hash)),*n_hash;
    int f_opcode,key_value;
    char f_mnm[20],f_format[10];
    while(fscanf(fp,"%X %s %s",&f_opcode,f_mnm,f_format) != EOF){
        n_hash = (Hash*)malloc(sizeof(Hash));
        n_hash->code = f_opcode;
        n_hash->next = NULL;
        key_value = hash_function(f_mnm,strlen(f_mnm)); //hashing을 통해 고유 인덱스값 지정
        sprintf(n_hash->mnemonic,"%s",f_mnm);
        sprintf(n_hash->format,"%s",f_format);
        if(key_value == -1){ //mnemonic이 소문자로 입력된 경우
            printf("Mnemonic Lower Case\n");
            return 0;
        }
        if(h_table[key_value] == NULL)
            h_table[key_value] = n_hash;
        else{ //인덱스값이 중복됐을 경우 Chaining방식으로 뒤에 연결해줌
            for(ptr = h_table[key_value];ptr->next != NULL;ptr = ptr->next);
            ptr->next = n_hash;
        }
    }
    fclose(fp);
    return 1;
}
int opcode(char* mnm){
    int key = hash_function(mnm,strlen(mnm));
    if(key == -1) return 0;
    Hash* ptr = (Hash*)malloc(sizeof(Hash));
    for(ptr = h_table[key];ptr != NULL;ptr = ptr->next){
        if(strcmp(ptr->mnemonic,mnm) == 0){ //hash table에서 해당 mnemonic 찾으면 return
            return ptr->code;
        }
    }
    if(ptr == NULL) return 0; //못찾았을시 0 return
    return 1;
}
void opcodelist(void){
    Hash* ptr = (Hash*)malloc(sizeof(Hash));
    int num;
    for(num = 0;num < 20;num++){
        printf("%d  :  ",num);
        if(h_table[num] == NULL){
            printf("\n");
            continue;
        }
        for(ptr = h_table[num];ptr != NULL;ptr = ptr->next){ //hash table index마다 끝까지 출력
            if(ptr->next == NULL) {
                printf("[%s,%02X]\n",ptr->mnemonic,ptr->code);
                break;
            }
            printf("[%s,%02X] -> ",ptr->mnemonic,ptr->code);
        }
    }
}
