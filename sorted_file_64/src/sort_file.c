#include "sort_file.h"
#include "bf.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int copyfile(int);
int getEntries(int);

SR_ErrorCode SR_Init() {
	// Your code goes here

	return SR_OK;
}

SR_ErrorCode SR_CreateFile(const char *fileName) {
	// Your code goes here
	if(BF_CreateFile(fileName) != BF_OK) return SR_ERROR;
	int fd;
	BF_OpenFile(fileName,&fd);
	BF_Block *block;
	BF_Block_Init(&block);
	BF_AllocateBlock(fd,block);
	char *data = BF_Block_GetData(block);
	char str[] = "hp";
	memcpy(data,str,strlen(str)+1);
	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
	BF_CloseFile(fd);

	return SR_OK;
}

SR_ErrorCode SR_OpenFile(const char *fileName, int *fileDesc) {
	// Your code goes here
	if(BF_OpenFile(fileName,fileDesc) != BF_OK) return SR_ERROR;
	BF_Block *block;
	BF_Block_Init(&block);
	BF_GetBlock(*fileDesc,0,block);
	char *data = BF_Block_GetData(block);
	char str[strlen("hp")+1];
	if(data != NULL) memcpy(str,data,strlen("hp")+1);
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
	if(strcmp(str,"hp")) return SR_ERROR;
	return SR_OK;
}

SR_ErrorCode SR_CloseFile(int fileDesc) {
	// Your code goes here
	if(BF_CloseFile(fileDesc) != BF_OK) return SR_ERROR;

	return SR_OK;
}

SR_ErrorCode SR_InsertEntry(int fileDesc,	Record record) {
	// Your code goes here
	/* * Initializations * */
	BF_Block *block;
	BF_Block_Init(&block);
	int block_num=0;
	BF_GetBlockCounter(fileDesc,&block_num);
	/* ============================ */
	char *data ;
	int entries;
	int cur_block = block_num-1;	// block 0 contains only metadata
	if(cur_block == 0){
		cur_block++;
		BF_GetBlock(fileDesc,0,block);
	}
	while(1){
		if(BF_GetBlock(fileDesc,cur_block,block) != BF_OK) {
			BF_UnpinBlock(block);
			BF_Block_Destroy(&block);
			BF_Block_Init(&block);

			BF_AllocateBlock(fileDesc,block);
			data = BF_Block_GetData(block);
			memset(data,0,BF_BLOCK_SIZE);}
		data = BF_Block_GetData(block);
		memcpy(&entries,data,sizeof(int));
		if(data != BF_Block_GetData(block)) data = BF_Block_GetData(block);
		if(entries >= 17) {
			cur_block++;
			BF_UnpinBlock(block);
			continue;
		}
		int index = (entries ) * sizeof(Record) + (int)sizeof(int);
		memcpy(&(data[index]),&record,sizeof(Record));
		if(data != BF_Block_GetData(block)) data = BF_Block_GetData(block);
		entries++;
		memcpy(data,&entries,sizeof(int));
		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);
		return SR_OK;
	}
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
	return SR_OK;
}

SR_ErrorCode SR_SortedFile(
		const char* input_filename,
		const char* output_filename,
		int fieldNo,
		int bufferSize
		) {
	// Your code goes here
	int fd,nfd;
	BF_Init(LRU);
	SR_OpenFile(input_filename,&fd);
	nfd=fd;//
//	int k=copyfile(fd);
	int blocks_num;
	BF_GetBlockCounter(nfd,&blocks_num);
	int b=1;
	//bufferSize=1;	//========== na to sbiswww =========//
	for(b=1;b<blocks_num;){
		if(b>=blocks_num) break;
		//if(i>=blocks_num-1) return SR_OK;
		if(bufferSize<blocks_num-1-b)quicksort(nfd,0,17*(bufferSize)-1,b,bufferSize,fieldNo);
		else quicksort(nfd,0,16*(blocks_num-1-b)+getentries(nfd),b,bufferSize,fieldNo);
		b+=bufferSize;
	}
	mergesort(nfd,fieldNo,bufferSize);
//	SR_CloseFile(k);
	SR_CloseFile(fd);
	return SR_OK;
}



SR_ErrorCode SR_PrintAllEntries(int fileDesc) {
	// Your code goes here
	BF_Block *block;
	BF_Block_Init(&block);
	Record record;
	int block_num;
	char *data;
	BF_GetBlockCounter(fileDesc,&block_num);
	for(int i = 1;i<block_num;i++){
		BF_GetBlock(fileDesc,i,block);
		data = BF_Block_GetData(block);
		int entries;
		memcpy(&entries,data,sizeof(int));
		if(data != BF_Block_GetData(block)) data = BF_Block_GetData(block);
		int j = 0;
		while(j < entries){
			int index = j * sizeof(Record) + sizeof(int);
			memcpy(&record,&(data[index]),sizeof(Record));
			if(data != BF_Block_GetData(block)) data = BF_Block_GetData(block);
			printf("%d,\"%s\",\"%s\",\"%s\"\n",record.id, record.name, record.surname, record.city);
			j++;
		}
		BF_UnpinBlock(block);
	}
	BF_Block_Destroy(&block);
	return SR_OK;
}
