/*
*********************************************************************************************************
*                                              EmFrmwork
*                                        The cross platform Frmwork
*
*
* File    : cmemory.c
* By      : wangchao
* Version : V0.1.1
* Date    : 2016-09-25
* --------------------
*  内存管理函数，包括快速的链表式管理和相对慢速但内存管理灵活的buddy system
*********************************************************************************************************
*/
#include <string.h>
#include"cknl.h"

#include<stdio.h>  //for test

//注意修改buddy_mem结构体大小需要修改cknl中的BUDDYST_SIZE(level)大小
typedef struct __buddy_mem {
	u1* memaddr;   //表示数据的首地址
	u1 reserved;
	u1 sizelog;    // 表示最大的存储数值，n 表示2^n大小的数值
	u2 blksizelog;  //每个块的大小
	u1 longest[0]; //用于描述的从二叉树根部分像后端索引的具有的最大size
}buddy_mem;

//#define LEFT_LEAF(index) ((index) * 2 + 1)
//#define RIGHT_LEAF(index) ((index) * 2 + 2)
//#define PARENT(index) ( ((index) + 1) / 2 - 1)


#define LEFT_LEAF(index)  ((index)<<1)
#define RIGHT_LEAF(index) (((index)<<1) + 1)
#define PARENT(index)     ((index)>>1)
#define BUDDY_INDEX(index) ((index)^0x1)
#define IS_POWER_OF_2(x) (!((x)&((x)-1)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MAX_DATASIZE_LOG    12
#define BUDDY_BLOCK_SIZE    1024
static _inline u4 fixsize(u4 size) {
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  return size+1;
}

static _inline u1 fixsizelog(u4 size){
	u1 sizelog=0;
	if (size == 0) return 0;
	size -= 1;
	while(size)
	{
		size = size >> 1;
		sizelog++;
	};
	return sizelog;
}

void buddy_showTest(buddy_mem* self);

//addr表示数据块的地址,sizelog表示2^sizelog大小的最大内存值
u1 buddy_create(u1* addr, u1 sizelog, u2 blksizelog) {
  buddy_mem* self;
  u4 node_size;
  u4 i;

  if (sizelog > MAX_DATASIZE_LOG)
    return CDEBUG_ON(BUDDY_SIZE_OVERFLOW);

  self = (buddy_mem*)addr;
  self->sizelog = sizelog;
  self->blksizelog = blksizelog;
  sizelog += 1;
  node_size = (1<<sizelog);
  self->longest[0] = 0; //用于释放内存时查找到该值后停止
  self->longest[1] = sizelog; //用于存放最大值sizelog+1
  for (i = 2; i <node_size; i++) {
	  if (IS_POWER_OF_2(i)) {
		  sizelog -= 1;
	  }
    self->longest[i] = sizelog;
  }
  self->memaddr = self->longest + node_size;
#ifdef BUDDYMEM_DEBUG
  buddy_showTest(self);
#endif
  return CKNL_ERROR_NONE;
}

//申请内存的函数
void* buddy_alloc(void* memaddr, u4 size) {
  u4 index;
  u4 offset;
  u1* addr = (u1*)0;
  buddy_mem* self = (buddy_mem*)memaddr;
  u1 longest, buddy_longest, node_size, sizelog;
  if (size==0)
    return (void*)0;
  //获取size除以blocksize后的大小并取log值，均为整数向上取整，同时size_log值再+1使实际值为2^(sizelog-1)
  size = ((size - 1) >> self->blksizelog) + 1;
  sizelog = fixsizelog(size)+1;
  index = 1;
  CKNL_ENTER_CRITICAL();
  if (self->longest[1] < sizelog) {
	  CKNL_EXIT_CRITICAL();
	  CDEBUG_ON(BUDDY_SIZE_OVERFLOW);
	  return (void*)0;
  }
    

  for(node_size = self->sizelog+1; node_size > sizelog; node_size--) {
	index = LEFT_LEAF(index);
    if (self->longest[index] < size)
      index = BUDDY_INDEX(index);
  }

  self->longest[index] = 0;
  offset = index*(1<<(node_size-1)) - (1<<self->sizelog); // 首地址 2^x*2^node_size- 2^ max_size = 0

  while (index>1) {
	longest = self->longest[index];
	buddy_longest = self->longest[BUDDY_INDEX(index)];
    index = PARENT(index);
    self->longest[index] = MAX(longest, buddy_longest);
  }
  CKNL_EXIT_CRITICAL();
  addr = self->memaddr + (offset<<self->blksizelog);
#ifdef BUDDYMEM_DEBUG
  buddy_showTest(self);
#endif
  return addr;
}
//释放内存的函数
u1 buddy_free(void* memaddr, void* addr) {
  u4 node_size, index, offset;
  buddy_mem* self = (buddy_mem*)memaddr;
  u1 longest, buddy_longest,buddy_flag;
  offset = (((u1*)addr - self->memaddr)>>self->blksizelog);

  if (offset > (1u << self->sizelog) || ((u1*)(addr) < self->memaddr)) {
	  return CDEBUG_ON(BUDDY_INVALID_MEM);
  }

  index = offset + (1 << self->sizelog); //将起始路径设为最下层路径序号
  node_size = 1;

  CKNL_ENTER_CRITICAL();
  for (; self->longest[index]; index = PARENT(index)) {
	//从底层向高层获取index所在的第一个数值为0的节点，以确定实际size和节点位置
    node_size++;
  }
  
  if (index == 0) {
	  CKNL_EXIT_CRITICAL();
	  return CDEBUG_ON(BUDDY_INVALID_MEM);
  }

  self->longest[index] = node_size;//将节点数值置为原始的size
  buddy_flag = 1;
  longest = node_size; 
  while (index>1) {
	buddy_longest = self->longest[BUDDY_INDEX(index)];
	index = PARENT(index);

	if (buddy_flag && (buddy_longest == node_size)) {
		longest= ++node_size;
		self->longest[index] = longest;
	}		
	else if (longest > buddy_longest) {
		self->longest[index] = longest;
		buddy_flag = 0;
	}		
	else
		break;
  }
  CKNL_EXIT_CRITICAL();

#ifdef BUDDYMEM_DEBUG
  buddy_showTest(self);
#endif
  return CKNL_ERROR_NONE;
}

u4 buddy_GetUsedMem(void* memaddr)
{
	buddy_mem* self = (buddy_mem*)memaddr;
	u4 i, node_size,size;
	u1  sizelog;
	size = 0;
	node_size = (1 << (self->sizelog + 1));
	sizelog = self->sizelog + 1;
	for (i = 1; i < node_size; i++) 
	{
		if (IS_POWER_OF_2(i)) 
		{
			sizelog--;
		}
		if (self->longest[i] == 0)
		{
			size += 1 << sizelog;
		}
	}
	size *= (1<<self->blksizelog);
		return size;
}

u4 buddy_GetFreeMem(void* memaddr)
{
	buddy_mem* self = (buddy_mem*)memaddr;
	u4 size;
	size = (1 << self->sizelog)*(1 << self->blksizelog) - buddy_GetUsedMem(memaddr);
	return size;
}
//显示buddy结构体的函数
void buddy_showTest(buddy_mem* self)
{
	u4 i, node_size;
	u1  level;
	level = 0;
	node_size = (1 << (self->sizelog+1));
	printf("buddy_mem_struct");
	for (i = 1; i <node_size; i++) {
		if (IS_POWER_OF_2(i)) {
			level++;
			printf("\r\n node_level%d: ", level);
		}
		printf("%d ", self->longest[i]);
	}
	printf("\r\n");
}