/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRACEX_BUFFER_SIZE		64000

#define THREAD_STACK_SIZE		1024
#define LED_THREAD_PRIORITY		13

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t tracex_buffer[TRACEX_BUFFER_SIZE];

//tohle udelam dynamicky pres byte pool???
//uint8_t led_thread_stack[THEAD_STACK_SIZE]
TX_THREAD led_thread;

ULONG available_before;
ULONG available_after;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID led_thread_entry(ULONG intial_input);
/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  /* USER CODE BEGIN App_ThreadX_MEM_POOL */

  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;
  char *stack_ptr;

  ret = tx_byte_allocate(byte_pool,
                             (VOID **)&stack_ptr,
							 THREAD_STACK_SIZE,
                             TX_NO_WAIT);
  if (ret != TX_SUCCESS) {
      return TX_POOL_ERROR;
  }

  /* USER CODE END App_ThreadX_MEM_POOL */

  /* USER CODE BEGIN App_ThreadX_Init */
  tx_trace_enable(&tracex_buffer, TRACEX_BUFFER_SIZE, 30);

  ret = tx_thread_create(&led_thread,
                       "LED thread",
                       led_thread_entry,
                       0,
                       stack_ptr,
					   THREAD_STACK_SIZE,
					   LED_THREAD_PRIORITY,
					   LED_THREAD_PRIORITY,
                       TX_NO_TIME_SLICE,
                       TX_AUTO_START);

  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN  Before_Kernel_Start */

  /* USER CODE END  Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN  Kernel_Start_Error */

  /* USER CODE END  Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */
void led_thread_entry(ULONG initial_input)
{
    while (1)
    {
        HAL_GPIO_TogglePin(LED1_G_GPIO_Port, LED1_G_Pin);

        //half a second - 500ms
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
    }
}
/* USER CODE END 1 */
