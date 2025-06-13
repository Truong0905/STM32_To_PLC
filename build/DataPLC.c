#include "DataPLC.h"

volatile static uint8_t I[3][8] = {};
volatile uint16_t AI[8] = {};
volatile static uint8_t Q[3][8] = {};
volatile static uint8_t M[3][8] = {};
volatile static uint16_t u16VW[8] = {};
volatile static uint32_t u32VD[8] = {};
volatile static float f32VD[8] = {};
void PLC_Main_Task(void *param)
{
	(void)param;
	while (1)
	{
		PLC_Read_Pin_Input();
		/*--------------NetWork 1 -----------*/

		volatile uint8_t I0_0sl0 = 0;
		volatile static uint8_t checkEU0 = 1;
		if (!(I0_0))
		{
			checkEU0 = 0;
		}
		if ((!checkEU0) && (I0_0))
		{
			checkEU0 = 1;
			I0_0sl0 = 1;
		}
		M0_0 = (((I0_0sl0 + M0_0)) * !I0_1);
		if (M0_0 > 0)
		{
			M0_0 = 1;
		}
		else
		{
			M0_0 = 0;
		}
		/*--------------NetWork 2 -----------*/

		if ((M0_0))
			(memcpy((uint8_t *)&u16VW1, (uint8_t *)&AIW0, 2));
		M1_1 = (M0_0);
		if (M1_1 > 0)
		{
			M1_1 = 1;
		}
		else
		{
			M1_1 = 0;
		}
		/*--------------NetWork 3 -----------*/

		if ((M0_0))
			u32VD0 = (uint32_t)u16VW1;
		/*--------------NetWork 4 -----------*/

		if ((M0_0))
			f32VD10 = (float)u32VD0;
		/*--------------NetWork 5 -----------*/

		if ((M0_0))
			f32VD10 = f32VD10 * 100.0;
		/*--------------NetWork 6 -----------*/

		if ((M0_0))
			f32VD10 = f32VD10 / 4096.0;
		/*--------------NetWork 7 -----------*/

		volatile uint8_t u8Compare0 = 0;
		u8Compare0 = 0;
		if (f32VD10 >= 50.0)
		{
			u8Compare0 = 1;
		}
		Q0_0 = (M0_0 * u8Compare0);
		if (Q0_0 > 0)
		{
			Q0_0 = 1;
		}
		else
		{
			Q0_0 = 0;
		}
		PLC_Write_Pin_Output();
	}
}

void PLC_Read_Pin_Input(void)
{
	I0_0 = !HAL_GPIO_ReadPin(I0_0_PORT, I0_0_PIN);
	I0_1 = !HAL_GPIO_ReadPin(I0_1_PORT, I0_1_PIN);
}
void PLC_Write_Pin_Output(void)
{
	if (Q0_0 >= 1)
	{
		HAL_GPIO_WritePin(Q0_0_PORT, Q0_0_PIN, ENABLE);
	}
	else
	{
		HAL_GPIO_WritePin(Q0_0_PORT, Q0_0_PIN, DISABLE);
	}
}
