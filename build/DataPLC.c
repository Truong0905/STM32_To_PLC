#include "DataPLC.h"

volatile static uint8_t I[3][8] = {};
volatile uint16_t AI[3] = {};
volatile static uint8_t Q[3][8] = {};
volatile static uint8_t M[3][8] = {};
volatile static uint16_t u16_VM[8] = {};
volatile static uint32_t u32_VM[8] = {};
volatile static float f32_VM[8] = {};
volatile static uint8_t T37 = 0;
volatile static uint32_t countT37 = 0;
volatile static uint32_t T37reset = 0;
volatile static uint8_t vaoT37 = 0;
volatile static const uint32_t datT37 = 10;
volatile static uint8_t checkT37 = 0;
volatile static uint8_t C1 = 0;
volatile static uint32_t countC1 = 0;
volatile static uint8_t vaoC1 = 0;
volatile static uint8_t resetC1 = 0;
volatile static const uint32_t datC1 = 50;
volatile static uint8_t startC1 = 0;
volatile static uint8_t checkC1 = 0;
void Main_task(void *param)
{
	while (1)
	{
		read_Pin_Input();
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
		volatile uint8_t I0_1sl1 = 1;
		volatile static uint8_t checkEU1 = 1;
		if (!(!I0_1))
		{
			checkEU1 = 0;
		}
		if ((!checkEU1) && (!I0_1))
		{
			checkEU1 = 1;
			I0_1sl1 = 0;
		}
		M0_0 = (((I0_0sl0 + M0_0)) * !I0_1sl1);
		if (M0_0 > 0)
		{
			M0_0 = 1;
		}
		else
		{
			M0_0 = 0;
		}
		/*--------------NetWork 2 -----------*/

		if (!T37reset)
		{
			vaoT37 = (M0_0);
			T37;
			if (vaoT37)
			{
				if (countT37 >= datT37)
					T37 = 1;

				if ((T37 == 0) && (checkT37 == 0))
				{
					xTimerStart(handle_timerPLC[0], portMAX_DELAY);
					checkT37 = 1;
				}
			}
			else
			{
				if (checkT37 == 1)
				{
					xTimerStop(handle_timerPLC[0], portMAX_DELAY);
					checkT37 = 0;
				}
				T37 = 0;
				countT37 = 0;
			}
		}
		else
		{
			xTimerStop(handle_timerPLC[0], portMAX_DELAY);
			countT37 = 0;
			T37 = 0;
			checkT37 = 0;
		}
		/*--------------NetWork 3 -----------*/

		volatile uint8_t T37sl2 = 0;
		volatile static uint8_t checkEU2 = 1;
		if (!(T37))
		{
			checkEU2 = 0;
		}
		if ((!checkEU2) && (T37))
		{
			checkEU2 = 1;
			T37sl2 = 1;
		}
		volatile uint8_t tempC1_0 = 0;
		if (countC1 >= 50)
		{
			tempC1_0 = 1;
		}
		vaoC1 = (T37sl2);
		resetC1 = (tempC1_0 + !M0_0);
		if (resetC1)
		{
			countC1 = 0;
			C1 = 0;
			startC1 = 1;
		}
		else
		{
			if (vaoC1)
			{
				if (((checkC1 == 1) || (startC1 == 1)) && (countC1 <= 4294967295))
				{
					countC1++;
					startC1 = 0;
				}
				if (countC1 >= datC1)
				{
					C1 = 1;
				}
			}
			else
			{
				checkC1 = 1;
			}
		}
		/*--------------NetWork 4 -----------*/

		volatile uint8_t tempC1_1 = 0;
		if (countC1 <= 20)
		{
			tempC1_1 = 1;
		}
		Q1_6 = (tempC1_1 * M0_0);
		if (Q1_6 > 0)
		{
			Q1_6 = 1;
		}
		else
		{
			Q1_6 = 0;
		}
		/*--------------NetWork 5 -----------*/

		volatile uint8_t tempC1_2 = 0;
		if (countC1 > 20)
		{
			tempC1_2 = 1;
		}
		volatile uint8_t tempC1_3 = 0;
		if (countC1 <= 25)
		{
			tempC1_3 = 1;
		}
		Q2_2 = (tempC1_2 * tempC1_3 * M0_0);
		if (Q2_2 > 0)
		{
			Q2_2 = 1;
		}
		else
		{
			Q2_2 = 0;
		}
		/*--------------NetWork 6 -----------*/

		volatile uint8_t tempC1_4 = 0;
		if (countC1 > 25)
		{
			tempC1_4 = 1;
		}
		Q2_0 = (tempC1_4 * M0_0);
		if (Q2_0 > 0)
		{
			Q2_0 = 1;
		}
		else
		{
			Q2_0 = 0;
		}
		/*--------------NetWork 7 -----------*/

		T37reset = (T37);
		if (T37reset > 0)
		{
			T37reset = 1;
		}
		else
		{
			T37reset = 0;
		}
		write_Pin_Output();
	}
}
void TimerCallBack(TimerHandle_t xTimer)
{
	int id;
	id = (uint32_t)pvTimerGetTimerID(xTimer);
	switch (id)
	{
	case 1:
		countT37++;
		break;
	}
}
void initTimer(void)
{
	handle_timerPLC[0] = xTimerCreate("timerTONT37", pdMS_TO_TICKS(100), pdTRUE, (static void *)(0 + 1), TimerCallBack);
}

void read_Pin_Input(void)
{
	I0_0 = !HAL_GPIO_ReadPin(I0_0_PORT, I0_0_PIN);
	I0_1 = !HAL_GPIO_ReadPin(I0_1_PORT, I0_1_PIN);
}
void write_Pin_Output(void)
{
	if (Q1_6 >= 1)
	{
		HAL_GPIO_WritePin(Q1_6_PORT, Q1_6_PIN, ENABLE);
	}
	else
	{
		HAL_GPIO_WritePin(Q1_6_PORT, Q1_6_PIN, DISABLE);
	}
	if (Q2_0 >= 1)
	{
		HAL_GPIO_WritePin(Q2_0_PORT, Q2_0_PIN, ENABLE);
	}
	else
	{
		HAL_GPIO_WritePin(Q2_0_PORT, Q2_0_PIN, DISABLE);
	}
	if (Q2_2 >= 1)
	{
		HAL_GPIO_WritePin(Q2_2_PORT, Q2_2_PIN, ENABLE);
	}
	else
	{
		HAL_GPIO_WritePin(Q2_2_PORT, Q2_2_PIN, DISABLE);
	}
}
