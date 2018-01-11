#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./include/bf.h"
#include "./include/sort_file.h"



void merge(){
	BF_Block *block[bfsize-1];
	for(int i=0;i<bfsize-1;i++)
		BF_Block_Init(&(block[i]));

	char *data[bfsize-1] ;
	int pos[bfsize-1]={0};

	for(int i=0;i<bfsize-1;i++){              // ========= index? ========== //
		if(BF_GetBlock(fd,i*bfsize,block[i]) != BF_OK) printf("GetBlock error\n");
		data[i] = BF_Block_GetData(block[i]);
	}
}
