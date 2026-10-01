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
#define LED_THREAD_COUNT         3U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* TraceX requires ULONG-aligned storage. The buffer is still 64000 bytes. */
ULONG tracex_buffer[TRACEX_BUFFER_SIZE / sizeof(ULONG)];
TX_THREAD led_threads[LED_THREAD_COUNT];
TX_MUTEX led_mutex;

/* Keep each measurement available in the debugger as well as the terminal. */
ULONG available_before[LED_THREAD_COUNT];
ULONG available_after[LED_THREAD_COUNT];
ULONG allocation_overhead[LED_THREAD_COUNT];

static CHAR *led_thread_names[LED_THREAD_COUNT] = {
    "LED1 green", "LED2 green", "LED3 green"
};
static GPIO_TypeDef *led_ports[LED_THREAD_COUNT] = {
    LED1_G_GPIO_Port, LED2_G_GPIO_Port, LED3_G_GPIO_Port
};
static const uint16_t led_pins[LED_THREAD_COUNT] = {
    LED1_G_Pin, LED2_G_Pin, LED3_G_Pin
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID led_thread_entry(ULONG initial_input);
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
  VOID *stack_ptr;
  UINT i;

  /* USER CODE END App_ThreadX_MEM_POOL */

  /* USER CODE BEGIN App_ThreadX_Init */
  /* Start tracing before allocation so those calls appear in TraceX too. */
  ret = tx_trace_enable(tracex_buffer, sizeof(tracex_buffer), 30);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  for (i = 0; i < LED_THREAD_COUNT; i++)
  {
    ret = tx_byte_pool_info_get(byte_pool, TX_NULL, &available_before[i],
                                TX_NULL, TX_NULL, TX_NULL, TX_NULL);
    if (ret != TX_SUCCESS)
    {
      return ret;
    }

    ret = tx_byte_allocate(byte_pool, &stack_ptr, THREAD_STACK_SIZE, TX_NO_WAIT);
    if (ret != TX_SUCCESS)
    {
      return ret;
    }

    ret = tx_byte_pool_info_get(byte_pool, TX_NULL, &available_after[i],
                                TX_NULL, TX_NULL, TX_NULL, TX_NULL);
    if (ret != TX_SUCCESS)
    {
      tx_byte_release(stack_ptr);
      return ret;
    }

    /* Pool consumption includes both the requested stack and allocator overhead. */
    allocation_overhead[i] = available_before[i] - available_after[i]
                             - THREAD_STACK_SIZE;

    /* Only LED1 starts automatically; it creates the mutex before resuming peers. */
    ret = tx_thread_create(&led_threads[i], led_thread_names[i], led_thread_entry,
                           i, stack_ptr, THREAD_STACK_SIZE,
                           LED_THREAD_PRIORITY, LED_THREAD_PRIORITY,
                           TX_NO_TIME_SLICE,
                           (i == 0U) ? TX_AUTO_START : TX_DONT_START);
    if (ret != TX_SUCCESS)
    {
      tx_byte_release(stack_ptr);
      return ret;
    }
  }

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
    if (initial_input == 0U)
    {
        if (tx_mutex_create(&led_mutex, "Shared LED mutex", TX_NO_INHERIT) != TX_SUCCESS)
        {
            Error_Handler();
        }

        /* Print once, with the scheduler running and before the other threads start. */
        for (UINT i = 0; i < LED_THREAD_COUNT; i++)
        {
            printf("LED%u stack: before=%lu, after=%lu, requested=%lu, "
                   "used=%lu, overhead=%lu bytes\r\n",
                   (unsigned int)(i + 1U), available_before[i], available_after[i],
                   (ULONG)THREAD_STACK_SIZE, available_before[i] - available_after[i],
                   allocation_overhead[i]);
        }
        fflush(stdout);

        for (UINT i = 1; i < LED_THREAD_COUNT; i++)
        {
            if (tx_thread_resume(&led_threads[i]) != TX_SUCCESS)
            {
                Error_Handler();
            }
        }
    }

    while (1)
    {
        if (tx_mutex_get(&led_mutex, TX_WAIT_FOREVER) != TX_SUCCESS)
        {
            Error_Handler();
        }

        /* Active-low LEDs: retain ownership throughout the one-second sleep. */
        HAL_GPIO_WritePin(led_ports[initial_input], led_pins[initial_input], GPIO_PIN_RESET);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
        HAL_GPIO_WritePin(led_ports[initial_input], led_pins[initial_input], GPIO_PIN_SET);

        if (tx_mutex_put(&led_mutex) != TX_SUCCESS)
        {
            Error_Handler();
        }
    }
}
/* USER CODE END 1 */
