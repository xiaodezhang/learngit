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
*  �ڴ���������������ٵ�����ʽ�����������ٵ��ڴ��������buddy system
*********************************************************************************************************
*/
#include <string.h>
#include"cknl.h"

#include<stdio.h>  //for test

//ע���޸�buddy_mem�ṹ���С��Ҫ�޸�cknl�е�BUDDYST_SIZE(level)��С
typedef struct __buddy_mem {
	u1* memaddr;   //��ʾ���ݵ��׵�ַ
	u1 reserved;
	u1 sizelog;    // ��ʾ���Ĵ洢��ֵ��n ��ʾ2^n��С����ֵ
	u2 blksizelog;  //ÿ����Ĵ�С
	u1 longest[0]; //���������ĴӶ��������������������ľ��е����size
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

//addr��ʾ���ݿ�ĵ�ַ,sizelog��ʾ2^sizelog��С������ڴ�ֵ
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
  self->longest[0] = 0; //�����ͷ��ڴ�ʱ���ҵ���ֵ��ֹͣ
  self->longest[1] = sizelog; //���ڴ�����ֵsizelog+1
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

//�����ڴ�ĺ���
void* buddy_alloc(void* memaddr, u4 size) {
  u4 index;
  u4 offset;
  u1* addr = (u1*)0;
  buddy_mem* self = (buddy_mem*)memaddr;
  u1 longest, buddy_longest, node_size, sizelog;
  if (size==0)
    return (void*)0;
  //��ȡsize����blocksize��Ĵ�С��ȡlogֵ����Ϊ��������ȡ����ͬʱsize_logֵ��+1ʹʵ��ֵΪ2^(sizelog-1)
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
  offset = index*(1<<(node_size-1)) - (1<<self->sizelog); // �׵�ַ 2^x*2^node_size- 2^ max_size = 0

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
//�ͷ��ڴ�ĺ���
u1 buddy_free(void* memaddr, void* addr) {
  u4 node_size, index, offset;
  buddy_mem* self = (buddy_mem*)memaddr;
  u1 longest, buddy_longest,buddy_flag;
  offset = (((u1*)addr - self->memaddr)>>self->blksizelog);

  if (offset > (1u << self->sizelog) || ((u1*)(addr) < self->memaddr)) {
	  return CDEBUG_ON(BUDDY_INVALID_MEM);
  }

  index = offset + (1 << self->sizelog); //����ʼ·����Ϊ���²�·�����
  node_size = 1;

  CKNL_ENTER_CRITICAL();
  for (; self->longest[index]; index = PARENT(index)) {
	//�ӵײ���߲��ȡindex���ڵĵ�һ����ֵΪ0�Ľڵ㣬��ȷ��ʵ��size�ͽڵ�λ��
    node_size++;
  }
  
  if (index == 0) {
	  CKNL_EXIT_CRITICAL();
	  return CDEBUG_ON(BUDDY_INVALID_MEM);
  }

  self->longest[index] = node_size;//���ڵ���ֵ��Ϊԭʼ��size
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
//��ʾbuddy�ṹ��ĺ���
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