#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bf.h"
#include "sort_file.h"

struct info{
	int curblock;
	int sorted;
	int curentrie;
};
typedef struct info Info;

Record selectmax(char **,Info*,int*,int);
int getbentries(char*);
int output(char* d,Record *r);
int mergedall(Info *,int);

void merge(int fd,int nfd,int buffsize,int firstb,int sorted,int field){
	Info info[buffsize];
	int pos[buffsize],curentrie[buffsize],bn,last,index=0;			// current Record
	BF_Block *block[buffsize],*outp;
	char *data[buffsize],*outd;
	Record max;

	BF_Block_Init(&outp);
	int outi=(buffsize-1)*sorted*firstb+1;
	if(BF_GetBlock(nfd,outi,outp)!=BF_OK) return;
	outd = BF_Block_GetData(outp);
	memset(outd,0,BF_BLOCK_SIZE);
	BF_GetBlockCounter(fd,&bn);
	for(int i=0;i<buffsize;i++){
		last = i;

		pos[i] = (i+firstb)*sorted +1;
		info[i].curblock = pos[i];
		info[i].sorted = sorted;

		BF_Block_Init(&(block[i]));
		if(BF_GetBlock(fd,pos[i],block[i])!= BF_OK){
			last--;
			break;
		}
		data[i] = BF_Block_GetData(block[i]);
		info[i].curentrie = getbentries(data[i]);
		if(getbentries(data[i]) > 17 ) printf("kaooooooooooooooooooo\n" );
		if(pos[i]>=bn-1) break;
	}
	int i;
	while(!mergedall(info,last)){
		int cmp,merged;
		max = selectmax(data,info,&merged,last);//getrec(data,0);
		for( i=0;i<=last;i++){
			if(info[i].curentrie==0){
				if(--info[i].sorted > 0){
					info[i].curblock++;
					BF_UnpinBlock(block[i]);
					if(BF_GetBlock(fd,info[i].curblock,block[i]) != BF_OK) goto end;
					data[i] = BF_Block_GetData(block[i]);
					info[i].curentrie = getbentries(data[i]);
				}
				else continue;
			}
			Record cur;

			cur = getRec(data[i],getbentries(data[i])-info[i].curentrie);
			cmp = compare(max,cur,field);
			if(cmp > 0){
				max = cur;
				merged = i;
			}
		}
		int a = output(outd,&max);
		if(a == 1){
			BF_Block_SetDirty(outp);
			BF_UnpinBlock(outp);
			if(BF_GetBlock(nfd,++outi,outp) !=BF_OK) goto end;
			outd = BF_Block_GetData(outp);
			memset(outd,0,BF_BLOCK_SIZE);
		}
		info[merged].curentrie--;
	}
	end: for(int i=0;i<last;i++){
		BF_UnpinBlock(block[i]);
		BF_Block_Destroy(&block[i]);
	}
	BF_Block_SetDirty(outp);
	BF_UnpinBlock(outp);
	BF_Block_Destroy(&outp);

}

int mergesort(int fd,int fieldNo,int bufferSize){
	int nfd,t;
	nfd = copyfile(fd);
	int bn,counter=0;
	BF_GetBlockCounter(fd,&bn);
	int sorted = bufferSize;
	int b=0;
	while(sorted < bn){
		 int b=0;
		while(b<((bn-1)/sorted)){
			merge(fd,nfd,bufferSize-1,b,sorted,fieldNo);
			printf("c=%d\n",++counter );
			b+=(bufferSize-1);
		}
		sorted *= (bufferSize - 1);
		t = nfd;
		nfd = fd;
		fd = t;
	}
	SR_PrintAllEntries(t);
}

int getbentries(char *data){
	int temp;

	memcpy(&temp,data,sizeof(int));

	return temp;
}

int mergedall(Info *info,int bfsize){
	int merged = 1;
	for(int i=0;i<=bfsize;i++){
		if((info[i]).curentrie > 0 || info[i].sorted > 0)
			merged = 0;
		break;
	}
	return merged;
}


Record selectmax(char **data,Info *info,int *p,int bfsize){
	Record temp;
	int i;
	for(i=0;i<=bfsize;i++){
		int t= info[i].curentrie;
		if(t > 0){
			temp=getRec(data[i],getbentries(data[i])-info[i].curentrie);
			*p=i;
			return temp;
		}
	}
}



Record getRec(char *data,int entrie){
	Record temp;

	int off = (unsigned int) ((entrie)*sizeof(Record)+sizeof(int));

	memcpy(&temp,&data[off],sizeof(Record));
	return temp;
}

int output(char* d,Record *r){
	int e;
	memcpy(&e,d,sizeof(int));
	e++;
	memcpy(d,&e,sizeof(int));
	d += (unsigned int) ((e-1)*sizeof(Record)+sizeof(int));
	memcpy(d,r,sizeof(Record));
	if(e==17)
		return 1;
	return 0;
}
