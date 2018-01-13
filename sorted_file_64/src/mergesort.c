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
	int pos[buffsize],curentrie[buffsize],bn,last,index=0;																										// current Record
	BF_Block *block[buffsize],*outp;
	char *data[buffsize],*outd;
	Record max;

	BF_Block_Init(&outp);
	BF_GetBlockCounter(nfd,&bn);
	/*if(bn < 2){
		BF_AllocateBlock(nfd,outp);
		outd = BF_Block_GetData(outp);
		memset(outd,0,BF_BLOCK_SIZE);
	}
	else{
		BF_GetBlock(nfd,bn-1,outp);
		outd = BF_Block_GetData(outp);
	}
	if(getbentries(outd) >= 17 ){
		BF_AllocateBlock(nfd,outp);
		outd = BF_Block_GetData(outp);
		memset(outd,0,BF_BLOCK_SIZE);
	}
*/
BF_AllocateBlock(fd,outp);
outd = BF_Block_GetData(outp);
memset(outd,0,BF_BLOCK_SIZE);
	BF_GetBlockCounter(fd,&bn);
	for(int i=0;i<buffsize;i++){
		last = i;

		pos[i] = (i+firstb)*sorted +1;
		info[i].curblock = pos[i];
		info[i].sorted = sorted;

		BF_Block_Init(&(block[i]));
		if(BF_GetBlock(fd,pos[i],block[i])!= BF_OK) break;
		data[i] = BF_Block_GetData(block[i]);
		info[i].curentrie = getbentries(data[i]);
		if(getbentries(data[i]) > 17 ) printf("kaooooooooooooooooooo\n" );
		if(pos[i]>=bn-1) break;
	}
	while(!mergedall(info,last)){
				int cmp,merged,i;
				max = selectmax(data,info,&merged,last);//getrec(data,0);
		//		printf("Name:%s\n",max.name );
				for( i=0;i<last;i++){
					if(info[i].curentrie==0){
						if(--info[i].sorted > 0){
							info[i].curblock++;
							BF_UnpinBlock(block[i]);
							if(BF_GetBlock(fd,info[i].curblock,block[i]) != BF_OK) printf("fjhgsadfkjhasgfkajshgfaksjhfgaskjhdg\n" );
							data[i] = BF_Block_GetData(block[i]);
							info[i].curentrie = getbentries(data[i]);
					//		printf("ERROR ENTRIES\n" );
						}
						else continue;
					}
					Record cur;

					cur = getRec(data[i],17-info[i].curentrie);
				//	printf("name=%s\n",cur.name );
					cmp = compare(max,cur,field);

					if(cmp > 0){
						max = cur;
						merged = i;
					}
				}
				if(output(outd,max) == 1){
					BF_Block_SetDirty(outp);
					BF_UnpinBlock(outp);
					if(BF_AllocateBlock(nfd,outp)!=BF_OK) printf("sdasdasdasda\n" );
					outd = BF_Block_GetData(outp);
					memset(outd,0,BF_BLOCK_SIZE);
				}
				info[merged].curentrie--;
	}
	printf("========================================================\n");
	for(int i=0;i<last;i++){
		BF_UnpinBlock(block[i]);
		BF_Block_Destroy(&block[i]);
	}

}

int mergesort(int fd,int fieldNo,int bufferSize){
	int nfd;
	//SR_CreateFile("output");
//	SR_OpenFile("output",&nfd);
	int bn,counter=0;
	BF_GetBlockCounter(fd,&bn);
	int sorted = bufferSize;
	int b=0;
	while(sorted < bn){
		// int b=0;
		 while(b<bn){
			 merge(fd,fd,bufferSize-1,b,sorted,fieldNo);
			 printf("c=%d\n",++counter );
			 b+=sorted;
		 }
		// BF_GetBlockCounter(fd,&bn);
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
		if((info[i]).curentrie > 0 || info[i].sorted > 0)
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
		if(t > 0){
			temp=getRec(data[i],17-info[i].curentrie);
			*p=i;
			return temp;
		}

		return temp;
	}
}



Record getRec(char *data,int entrie){
	Record temp;

	int off = (unsigned int) ((entrie)*sizeof(Record)+sizeof(int));

	memcpy(&temp,&data[off],sizeof(Record));
	return temp;
}

int output(char* d,Record r){
	int e;

	memcpy(&e,d,sizeof(int));
	e++;
	//printf("e=%d\n", e);
	memcpy(d,&e,sizeof(int));
	d += (unsigned int) ((e-1)*sizeof(Record)+sizeof(int));
	memcpy(d,&r,sizeof(Record));
	if(e==17) {
	//	printf("===========\n");
		return 1;
	}
	return 0;

}
