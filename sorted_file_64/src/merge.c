#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bf.h"
#include "sort_file.h"

int limit = BF_BLOCK_SIZE/sizeof(Record); // probably in init()

int get_field(int fieldNo)
{
  if(!fieldNo) return sizeof(int);
  else if(fieldNo == 1) return 15*sizeof(char);
  else if(fieldNo == 2) return 20*sizeof(char);
  else if(fieldNo == 3) return 20*sizeof(char);
  return -1;
}

int get_offset(int fieldNo)
{
  if(!fieldNo) return 0;
  else if(fieldNo == 1) return sizeof(int);
  else if(fieldNo == 2) return sizeof(int) + 15*sizeof(char);
  else if(fieldNo == 3) return sizeof(int) + 15*sizeof(char) + 20*sizeof(char);
  return -1;
}

void write_rec(char* data, char* src, int dpos, int spos)
{
  int offsetd = sizeof(int) + dpos*sizeof(Record);
  int offsetp = sizeof(int) + spos*sizeof(Record);
  memcpy(&data[offsetd],&src[offsetp],sizeof(Record));
}

Record get_rec(char* data,int pos ,int fieldNo)
{
  int offset = sizeof(int) + pos*sizeof(Record);

  Record temp;
  memcpy(&temp,&data[offset],sizeof(Record));
  return temp;
}

int compare2(Record r1, Record r2, int fieldNo)
{
  if(!fieldNo) return r1.id < r2.id;
  else if(fieldNo == 1) return strncmp(r1.name,r2.name,15);
  else if(fieldNo == 2) return strncmp(r1.surname,r2.surname,20);
  else if(fieldNo == 3) return strncmp(r1.city,r2.city,20);
}

int check_block(int fd,int fd2, int prev_min,char** data,int* position,BF_Block** block,int bufferSize,int* curr_block,int blocks_num,int gend)
{
  int climit;
  memcpy(&climit,data[prev_min],sizeof(int));    // get current limit

  if(position[prev_min] == climit)  // end of block
    if(++curr_block[prev_min] < blocks_num - 1)
      if(curr_block[prev_min] + 1 == (prev_min+1)*gend + 1){ //we enterd other group
        BF_GetBlock(fd,curr_block[prev_min],block[prev_min]);
        data[prev_min] = BF_Block_GetData(block[prev_min]);
        BF_UnpinBlock(block[prev_min]);
      }
      else
        curr_block[prev_min] = -1;
    else
      //curr_block[prev_min]++;
      curr_block[prev_min] = -1;

  if(position[bufferSize-1] == limit){  // new output block
    BF_Block_SetDirty(block[bufferSize-1]);
    BF_UnpinBlock(block[bufferSize-1]);
    BF_AllocateBlock(fd2,block[bufferSize-1]);
    int n;
    BF_GetBlockCounter(fd2,&n);
    curr_block[bufferSize-1] = n-1;
    position[bufferSize-1] = 0;
    //curr_block[bufferSize-1]++;
  }
}

int minIndex(char** data,int* position,int* curr_block, int bufferSize, int fieldNo)
{

  Record min;

  int counter = 0, indicator = 0;   // indicator points to min block using
  for(int i = 0; i < bufferSize-1; i++){
    if(curr_block[i] == -1) // end of the blocks in this group
      counter++;    // if counter == bufferSize-1 ---> end of iteration
    else{
      indicator = i;
      min = get_rec(data[indicator],position[indicator],fieldNo);  // !!!! sth thesh position[indicator]
      break;
    }
  }

  if(counter == bufferSize-1) // all blocks sorted
    return -1;

  int minpos = indicator;
  for(int i = indicator+1; i < bufferSize-1; i++)
    if(curr_block[i] != -1){
      if(compare2(min,get_rec(data[i],position[i],fieldNo),fieldNo) < 0) // see what compare returns and what means // i want min > cur_block[i]
        min = get_rec(data[i],position[i],fieldNo);
        minpos = i; // position in data array
    }

  //data[bufferSize-1]...   // last place takes the new record
  write_rec(data[bufferSize-1],data[minpos],position[bufferSize-1],position[minpos]);

  //data[minpos]-->move one record
  position[minpos]++; // move it, it will be checked in check_block
  position[bufferSize-1]++;

  return minpos;
}

void merge(int fileDesc, int bufferSize, int fieldNo, const char* fileName)
{
  int fd;
  SR_CreateFile(fileName);  // output file
  SR_OpenFile("unsorted_data.db",&fileDesc);  // temp
  SR_OpenFile(fileName,&fd);      // output

  int blocks_num;   // total blocks number
  BF_GetBlockCounter(fileDesc,&blocks_num);

  //int numberOfGroups = blocks_num/bufferSize + 1;   // +1 because we want the ceil of number
  int position[bufferSize];       // for every buffer the position in block of the element we check
  int curr_block[bufferSize];      // for every buffer current block number using

  for(int i = 0; i < bufferSize; i++) // even for the last block (output) know which record this is
    position[i] = 0;

  // blocks array // blocks loading
  BF_Block *block[bufferSize];            // last block is output block
	for(int i = 0; i < bufferSize; i++)
    BF_Block_Init(&(block[i]));

  // data array from blocks loaded
  char* data[bufferSize];

  BF_GetBlock(fileDesc,0,block[0]);     //
  data[0] = BF_Block_GetData(block[0]); // write block
  BF_GetBlock(fd,0,block[1]);           // 0 as it is
  data[1] = BF_Block_GetData(block[1]); //
  memcpy(data[1],data[0],BF_BLOCK_SIZE);//
  BF_Block_SetDirty(block[1]);          //
  BF_UnpinBlock(block[1]);              //

  for(int i = 0; i < bufferSize-1; i++){
    curr_block[i] = i*(bufferSize-1)+1;    // starting block from every group
    BF_GetBlock(fileDesc,curr_block[i],block[i]);  // get the first block from the groups (as much, as the buffer can hold)
    data[i] = BF_Block_GetData(block[i]);
  }
  BF_AllocateBlock(fd,block[bufferSize-1]);             // for the output
  data[bufferSize-1] = BF_Block_GetData(block[bufferSize-1]); // block


  int prev_min = 0; // position of changed data, block, position
  //int l = ceil(log(blocks_num)/log(bufferSize-1));  // how many iterations should be done
int l = 10;

  int gend; // used for start and end of every iteration
  int var = bufferSize-1;  // limit for groups
  for(int k = 0; k < l; k++){

    // calculation of current groups
    for(int j = 0; j < k; j++){
      var *= var;
      //var++;  // avoid block 0 impact
    }
int groups = 0; // iterator for groups
    for(int w = 1; w < blocks_num&& groups < blocks_num; w += var){   // iteration for whole merge of a level
      gend = w;  // ?
      while(prev_min != -1){
        prev_min = minIndex(data,position,curr_block,bufferSize,fieldNo); // find min
        if(prev_min == -1)
          break;
        check_block(fileDesc,fd,prev_min,data,position,block,bufferSize,curr_block,blocks_num,gend);
      }
      //next group Initializations
      for(int i = 0; i < bufferSize-1; i++){
        position[i] = 0;
        curr_block[i] = i*var*groups + 1;
        BF_GetBlock(fileDesc,curr_block[i],block[i]);
        data[i] = BF_Block_GetData(block[i]);
      }
      groups++;
    }

    for(int i = 0; i < bufferSize; i++){  // re initiallize
      position[i] = 0;
      //if(var > blocks_num) break;
      if(i != bufferSize-1) curr_block[i] = i*var + 1;
      BF_GetBlock(fileDesc,curr_block[i],block[i]);
      data[i] = BF_Block_GetData(block[i]);
    }
  }

  //SR_CloseFile(fileDesc);
  //SR_CloseFile(fd);

}
