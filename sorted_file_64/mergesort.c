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

Record selectmax(char **,int*,int*,int);
//Record getRec(char*,int);
int getbentries(char*);
int output(char* d,Record r);
int mergedall(Info *,int);

void merge(int fd,int nfd,int buffsize,int firstb,int sorted,int field){
	Info info[buffsize];
	int pos[buffsize],curentrie[buffsize],bn;																										// current Record
	BF_Block *block[buffsize],*outp;
	char *data[buffsize],*outd;
	Record max;

	BF_Block_Init(&outp);
	BF_GetBlockCounter(nfd,&bn);
	if(bn < 1)
		BF_AllocateBlock(nfd,outp);
	else
		BF_GetBlock(nfd,bn-1,outp);
	outd = BF_Block_GetData(outp);
	buffsize = 1;	//======================================= not to sbisww ===========
	for(int i=0;i<buffsize;i++){
		pos[i] = (i+firstb)*sorted;
		info[i].curblock = pos[i];
		info[i].sorted = sorted;

		BF_Block_Init(&(block[i]));
		BF_GetBlock(fd,pos[i],block[i]);
		data[i] = BF_Block_GetData(block[i]);
		info[i].curentrie = getbentries(data[i]);
//		curentrie[i] = getbentries(data[i]);
	}



	while(!mergedall(info,buffsize)){
				int cmp,merged;
				max = selectmax(data,curentrie,&merged,buffsize);//getrec(data,0);
				for(int i=0;i<buffsize;i++){
					if(info[i].curentrie==0){
						if(--sorted){
							info[i].curblock++;
							BF_UnpinBlock(block[i]);
							BF_GetBlock(fd,info[i].curblock,block[i]);
							data[i] = BF_Block_GetData(block[i]);
							info[i].curentrie = getbentries(data[i]);
						}
						else continue;
					}
					Record cur;

					cur = getRec(data[i],17-curentrie[i]);
					cmp = compare(max,cur,field);

					if(cmp > 0){
						max = cur;
						merged = i;
					}
				}
				if(output(outd,max) == 1){
					BF_Block_SetDirty(outp);
					BF_UnpinBlock(outp);
					BF_AllocateBlock(fd,outp);
					outd = BF_Block_GetData(outp);
					memset(outd,0,BF_BLOCK_SIZE);
				}
				info[merged].curentrie--;
	}

}

int mergesort(int fd,int fieldNo,int bufferSize){
	int nfd;
	SR_CreateFile("output");
	SR_OpenFile("output",&nfd);
	int bn;
	BF_GetBlockCounter(fd,&bn);
	int l = ceil(log(bn)/log(bufferSize-1)); // how many iterations should be done
	int sorted = bufferSize;
	 for(int i=0;i<l;i++){
		 int b=0;
		 while(b<bn){
			 merge(fd,nfd,bufferSize-1,b,sorted,fieldNo);
			 b+=sorted;
		 }
		 sorted *= bufferSize - 1;
	 }
}

int getbentries(char *data){
	int temp;

	memcpy(&temp,data,sizeof(int));

	return temp;
}

int mergedall(Info *info,int bfsize){
	int merged = 1;
	for(int i=0;i<bfsize;i++){
		if(info[i].curentrie != 0 || info[i].sorted != 0)
			merged = 0;
			break;
	}
	return merged;
}


Record selectmax(char **data,int *curentrie,int *p,int bfsize){
	Record temp;

	for(int i=0;i<bfsize;i++){
		if(curentrie[i] > 0)
			temp=getRec(data[i],17-curentrie[i]);
			*p=i;
			return temp;
	}
	return temp;
}



Record getRec(char *data,int entrie){
	Record temp;

	data += (unsigned int) ((entrie-1)*sizeof(Record)+sizeof(int));

	memcpy(&temp,data,sizeof(Record));
	return temp;
}

int output(char* d,Record r){
	int e;

	memcpy(&e,d,sizeof(int));
	e++;
	memcpy(d,&e,sizeof(int));
	d += (unsigned int) (e*sizeof(Record)+sizeof(int));
	memcpy(d,&r,sizeof(Record));
	if(e==17) return 1;
	return 0;

}
