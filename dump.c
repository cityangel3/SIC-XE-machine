#include "20171700.h"
//헤더파일에서 extern 선언한 전역변수 definition//
unsigned char virtual_mem[65536][16]={0,};

void dump_print(int start_pt,int end_pt){ //dump 명령어에 따라 형태에 맞춰 메모리 공간 출력
    int i,j,bound1=0,bound2=15,ff=(start_pt/16),lf=(end_pt/16); //ff= 행 시작 lf= 행 마지막
    for(i = ff;i <= lf;i++){
        printf("%05X ",i*16);
        if(i == ff) bound1 = start_pt % 16; //첫번째 행과 마지막 행에 대해
        if(i == lf) bound2 = end_pt % 16; //범위를 지정한다
        
        for(j =0;j < 16;j++){
            if((i==ff && j<bound1) || (i==lf && bound2<j))
                printf("   ");
            else
                printf("%02X ",virtual_mem[i][j]);
        }
        printf("; ");
        for(j =0;j < 16;j++){
            if(virtual_mem[i][j] < 32 || 126 < virtual_mem[i][j]) printf(".");
            else if((i==ff && j<bound1) || (i==lf && bound2<j)) printf(".");
            else printf("%c",virtual_mem[i][j]);
        }
        printf("\n");
    }
}
void dump(int start_ad,int end_ad){
    int start_pt,end_pt;
    if(end_ad != -1){ //dump start, end
        start_pt = start_ad;
        end_pt = end_ad;
    }
    else{
        if(start_ad == -1) //dump
            start_pt = virtual_ad;
        else //dump start
            start_pt = start_ad;
        end_pt = start_pt + 159;
    }
    if(end_pt > 0xfffff) end_pt = 0xfffff; //마지막 번지를 넘으면 마지막 번지까지만 출력
    dump_print(start_pt,end_pt);
    if(start_ad == -1) //dump 명령어 입력시에만 마지막 주소 저장
        virtual_ad = end_pt + 1;
    if(virtual_ad > 0xfffff) //마지막 번지까지 출력했을 시 번지 처음으로 초기화
        virtual_ad = 0;
}
void edit(int add,int val){
    int i = add/16, j = add%16; //각각 가상 메모리 공간의 행과 열을 의미
    virtual_mem[i][j] = val;
}
void fill(int start_ad,int end_ad,int val){
    int i,j,bound1=0,bound2=15,ff=(start_ad/16),lf=(end_ad/16);
    for(i = ff;i <= lf;i++){
        if(i == ff) bound1 = start_ad % 16; //메모리 공간을 2차원 배열로 설정했기 때문에
        if(i == lf) bound2 = end_ad % 16; //첫번째 행과 마지막 행에 대해서 범위를 지정
        for(j =0;j < 16;j++){
            if((i==ff && j<bound1) || (i==lf && bound2<j)) continue;
            else virtual_mem[i][j] = val;
        }
    }
}
void reset(void){ //가상 메모리 공간 0으로 초기화
    int i, j;
    for(i =0;i <65536;i++){
        for(j = 0;j<16;j++)
            virtual_mem[i][j] = 0;
    }
}
