#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "sort_file.h"


void quicksort(int fd, int low, int high, int firstb,int bfsize, int field){
	if(low < high){
		int bn,max;
		BF_GetBlockCounter(fd,&bn);
	//	if(i+firstb>=bn-1) break;)
		int pi = partition(fd,low,high,firstb,bfsize,field);

		quicksort(fd,low,pi-1,firstb,bfsize,field);
		quicksort(fd,pi+1,high,firstb,bfsize,field);
	}
}


int partition (int fd, int low, int high, int firstb,int bfsize, int field){
	BF_Block *block[bfsize];
	for(int i=0;i<bfsize;i++)
        	BF_Block_Init(&(block[i]));

        char *data[bfsize] ;
  int bn,max;
	BF_GetBlockCounter(fd,&bn);
	if (firstb==0) firstb++;
	for(int i=0;i<bfsize;i++){		// ========= index? ========== //
		max=i;

		if(BF_GetBlock(fd,firstb+i,block[i]) != BF_OK) printf("GetBlock error\n");
		data[i] = BF_Block_GetData(block[i]);
		if(i+firstb>=bn-1) break;
	}
	//int max=i;

	/*	find high	*/
	//if(high/17 + firstb >=bn) {//return (low -1 );
	if(firstb+max >= bn-1){
		int temp;
		memcpy(&temp,data[max],sizeof(int));
		printf("%d\n", temp);//temp;
		high-= (17-temp);
	}
	Record pivot = getrec(data,high);
	printf("pivot:%s\n",pivot.name );
	int i = low - 1;
	for(int j=low;j<(high-1);j++){
		printf("%d\n",j );
		Record cur = getrec(data,j);
		int cmp = compare(pivot,cur,field);

		if(cmp >= 0){
			i++;
			swap(data,i,j);
		}
	}
	swap(data,i+1,high);
	for(int i=0;i<max;i++){

		BF_Block_SetDirty(block[i]);
		BF_UnpinBlock(block[i]);
		BF_Block_Destroy(&block[i]);
	}
	return (i+1);
}

void swap(char** data,int i, int j){
	int bn_i,bn_j,off_i,off_j;
	char *di,*dj;

	bn_i = i/17;
	off_i = (i%17)*sizeof(Record);
	off_i += sizeof(int);
	di = data[bn_i];
	di += off_i;

	bn_j = j/17;
	off_j = (j%17)*sizeof(Record);
	off_j += sizeof(int);
	dj = data[bn_j];
	dj += off_j;

	Record temp;

	memcpy(&temp,di,sizeof(Record));
	memcpy(di,dj,sizeof(Record));
	memcpy(dj,&temp,sizeof(Record));
}

int compare(Record r1,Record r2, int field){
	if(field == 0) return intcmp(r1,r2);
	else return cmpstr(r1,r2,field);
}

int intcmp(Record r1, Record r2){
	if(r1.id == r2.id) return 0;
	else if(r1.id > r2.id) return -1;
	else return 1;
}

int cmpstr(Record r1, Record r2, int field){
	if(field == 1) return strcmp(r1.name,r2.name);
	else if(field == 2) return strcmp(r1.surname,r2.surname);
	else if(field == 3) return strcmp(r1.city,r2.city);
}



Record getRec(int fd,int n,int firstb){
	int b_n = n/17;
	b_n += firstb;
        int off = n%17;
        BF_Block *tblock;
        BF_Block_Init(&tblock);
        BF_GetBlock(fd,b_n,tblock);
        char *temp =  BF_Block_GetData(tblock);
        Record rec;
        temp = (unsigned int) temp + (off + sizeof(int));
        memcpy(&rec,temp,sizeof(Record));
        BF_UnpinBlock(tblock);
				BF_Block_Destroy(&tblock);
        return rec;
}


Record getrec(char **d,int i){
	int bn = i/17;
	//bn += curblock;
	printf("%d\n",bn );
	char* data = d[bn];

	int off = (i%17)*sizeof(Record);
	off += sizeof(int);

	data += (unsigned int)off;

	Record rec;
	memcpy(&rec,data,sizeof(Record));
	return rec;
}


int copyfile(int fd){
	BF_Block *tblock,*block;
	BF_Block_Init(&tblock);
	BF_Block_Init(&block);

	SR_CreateFile("temp");
	int nfd;
	SR_OpenFile("temp",&fd);

	char *tdata,*data;
	int blocks;
	BF_GetBlockCounter(fd,&blocks);

	for(int i=0;i<blocks;i++){
		BF_GetBlock(fd,i,block);
		BF_GetBlock(nfd,i,tblock);

		data=BF_Block_GetData(block);
		tdata=BF_Block_GetData(tblock);

		memcpy(tdata,data,BF_BLOCK_SIZE);
		BF_UnpinBlock(block);
		BF_Block_SetDirty(tblock);
		BF_UnpinBlock(tblock);


	}
	BF_Block_Destroy(&block);
	BF_Block_Destroy(&tblock);
	return nfd;
}

int getEntries(int fd){
	BF_Block *tblock;
	BF_Block_Init(&tblock);
	char *tdata;

	int entries,le,blocks;

	BF_GetBlockCounter(fd,&blocks);
	entries = (blocks-2)*17;
	BF_GetBlock(fd,blocks-1,tblock);
	tdata = BF_Block_GetData(tblock);
	memcpy(&le,tdata,sizeof(int));
	BF_UnpinBlock(tblock);
	BF_Block_Destroy(&tblock);
	return entries+le;
}
