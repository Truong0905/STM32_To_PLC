#include"DataPLC.h"

volatile static uint8_t I[3][8]={};
volatile uint16_t AI[3]={};
volatile static uint8_t Q[3][8]={};
volatile static uint8_t M[3][8]={};
volatile static uint16_t u16_VM[8]={};
volatile static uint32_t u32_VM[8]={};
volatile static float f32_VM[8]={};
void Main_task( void *param)
{
while(1)
{
read_Pin_Input();
/*--------------NetWork 1 -----------*/

if((M0_0))
  (memcpy(&u16VW1,&AIW0,2)) ;
M0_1=(M0_0) ;
if (M0_1>0)
{
M0_1 = 1;
}
else
{
M0_1 = 0 ;
}
/*--------------NetWork 2 -----------*/

if((M0_0))
  u32VD1 = (uint32_t)u16VW1;
/*--------------NetWork 3 -----------*/

if((M0_0))
  f32VD1 = (float)u32VD1;
/*--------------NetWork 4 -----------*/

if((M0_0))
  u32VD1 = u32VD1/32000.0;
/*--------------NetWork 5 -----------*/

if((M0_0))
  u32VD1 = u32VD1*100.0;
 write_Pin_Output();
}
}

void read_Pin_Input(void)
{
}
void write_Pin_Output(void)
{
}
