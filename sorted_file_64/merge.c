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

char* get_rec(char* data,int pos ,int fieldNo)
{
  int offset = sizeof(int) + pos*sizeof(Record) + get_offset(fieldNo);

  char* temp = (char*)malloc(sizeof(char));
  memcpy(temp,&data[offset],sizeof(get_field));
}

int compare(char* d1, char* d2, int fieldNo)
{
  return strncmp(d1,d2,get_field(fieldNo));
}

int check_block(int fd, int prev_min,char** data,int* position,BF_Block** block,int bufferSize,int* curr_block,int blocks_num)
{
  if(curr_block[prev_min] != -1)  // set -1 to know that we stopped using this group
    if(position[prev_min] == limit)  // end of block
      if(curr_block[prev_min] + 1 == (prev_min+1)*bufferSize+1) //we enterd other group
        if(++curr_block[prev_min] < blocks_num - 1)
          BF_GetBlock(fd,curr_block[prev_min],block[prev_min]);
        else
          curr_block[prev_min] = -1;
  return 0;
}

void merge(int fileDesc, int bufferSize, int fieldNo)
{
  int blocks_num;   // total blocks number
  BF_GetBlockCounter(fileDesc,&blocks_num);

  int numberOfGroups = blocks_num/bufferSize + 1;   // +1 because we want the ceil of number
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
  for(int i = 0; i < bufferSize; i++){
    if(i != bufferSize-1) curr_block[i] = i*bufferSize+1;    // starting block from every group
    BF_GetBlock(fileDesc,i*bufferSize+1,block[i]);  // get the first block from the groups (as much, as the buffer can hold)
    data[i] = BF_Block_GetData(block[i]);
  }

  int prev_min = 0; // position of changed data, block, position
  int l = ceil(log(blocks_num)/log(bufferSize-1));

  while(loops > 1){
    while(prev_min != -1){
      prev_min = minIndex(data,position,bufferSize,fieldNo); // find min
      check_block(fileDesc,prev_min,data,position,block,bufferSize,curr_block,blocks_num);
    }

    for(int i = 0; i < bufferSize; i++){
      position[i] = 0;
      if(i != bufferSize-1) curr_block[i] = i*bufferSize+1 + l;
      BF_GetBlock(fileDesc,i*bufferSize+1+l,block[i]);
      data[i] = BF_Block_GetData(block[i]);
    }
  }

}

int minIndex(char** data,int* position,int* curr_block, int bufferSize, int fieldNo)
{

  char* min;

  int counter = 0, indicator = 0;   // indicator points to min block using
  for(int i = 0; i < bufferSize-1; i++){
    if(curr_block[i] == -1) // end of the blocks in this group
      counter++;    // if counter == bufferSize-1 ---> end of iteration
    else{
      indicator = i;
      min = data[indicator];  // !!!! sth thesh position[indicator]
      break;
    }
  }

  if(counter == bufferSize-1) // all blocks sorted
    return -1;

  int minpos = indicator;
  for(int i = indicator+1; i < bufferSize-1; i++)
    if(curr_block[i] != -1){
      if(compare(min,get_rec(data[i],position[i],fieldNo) < 0) // see what compare returns and what means // i want min > cur_block[i]
        min = get_rec(data[i],position[i],fieldNo);
        minpos = i; // position in data array
    }

  //data[minpos]-->move one record
  position[minpos]++; // move it, it will be checked in check_block

  //data[bufferSize-1]...   // last place takes the new record
  write_rec(data[bufferSize-1],data[minpos],position[bufferSize-1],position[minpos]);

  return minpos;
}

//int compare(Record r1,Record r2, int field) // r2-r1
//void minIndex(char** data,int* position);
//compare;
