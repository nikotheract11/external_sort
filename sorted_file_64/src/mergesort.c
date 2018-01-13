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
	if(bn < 2){
		BF_AllocateBlock(nfd,outp);
		outd = BF_Block_GetData(outp);
		memset(outd,0,BF_BLOCK_SIZE);
	}
	else{
		BF_GetBlock(nfd,bn-1,outp);
		outd = BF_Block_GetData(outp);
	}

	for(int i=0;i<buffsize;i++){
		pos[i] = (i+firstb)*sorted +1;
	//	(pos[i])++;
		info[i].curblock = pos[i];
		info[i].sorted = sorted;


		BF_Block_Init(&(block[i]));
		BF_GetBlock(fd,pos[i],block[i]);
		data[i] = BF_Block_GetData(block[i]);
		info[i].curentrie = getbentries(data[i]);

//		curentrie[i] = getbentries(data[i]);
	}
	printf("\naaaa=%d\n",info[0].curentrie );

	while(!mergedall(info,buffsize)){
				int cmp,merged;
				//printf("%d , %d \n",info[0].curblock,info[0].curentrie );
				max = selectmax(data,info,&merged,buffsize);//getrec(data,0);
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

					cur = getRec(data[i],17-info[i].curentrie+1);
					printf("mpa=%d\n",cur.id );
					cmp = compare(max,cur,field);

					if(cmp > 0){
						max = cur;
						merged = i;
					}
				}
				if(output(outd,max) == 1){
					BF_Block_SetDirty(outp);
					BF_UnpinBlock(outp);
					//BF_Block_Destroy(&outp);
					//BF_Block_Init(&outp);
					if(BF_AllocateBlock(nfd,outp)!=BF_OK) printf("sdasdasdasda\n" );
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
	int bn,counter=0;
	BF_GetBlockCounter(fd,&bn);
	int l = ceil(log(bn)/log(bufferSize-1)); // how many iterations should be done
	int sorted = bufferSize;
	 for(int i=0;i<l;i++){
		 int b=1;
		 while(b<bn){
			 merge(fd,nfd,bufferSize-1,b,sorted,fieldNo);
			 printf("c=%d\n",++counter );
			 b+=sorted;
		 }
		 sorted *= (bufferSize - 1);
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
		if((info[i]).curentrie != 0 || info[i].sorted != 0)
			merged = 0;
			break;
	}
	return merged;
}


Record selectmax(char **data,Info *info,int *p,int bfsize){
	Record temp;
//	printf("===================\n%d , %d \n",info[0].curblock,info[0].curentrie );

int i;
	for(i=0;i<bfsize;i++){
		int t= info[i].curentrie;
		//printf("%d\n",t );
		if(t == 1){
			temp=getRec(data[i],17-info[i].curentrie);
			*p=i;
			return temp;
	}

	return temp;
}
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
	//printf("e=%d\n", e);
	memcpy(d,&e,sizeof(int));
	d += (unsigned int) (e*sizeof(Record)+sizeof(int));
	memcpy(d,&r,sizeof(Record));
	if(e==17) return 1;
	return 0;

}
