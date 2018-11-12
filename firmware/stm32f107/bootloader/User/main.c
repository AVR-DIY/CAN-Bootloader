/**
  ******************************************************************************
  * @file    main.c
  * $Author: wdluo $
  * $Revision: 17 $
  * $Date:: 2012-07-06 11:16:48 +0800 #$
  * @brief   ������.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, ViewTool</center>
  *<center><a href="http:\\www.viewtool.com">http://www.viewtool.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can_bootloader.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
//��������������λ��������������һ��
CBL_CMD_LIST CMD_List = 
{
  .Erase = 0x00,      //����APP��������
  .WriteInfo = 0x01,  //���ö��ֽ�д������ز�����д��ʼ��ַ����������
  .Write = 0x02,      //�Զ��ֽ���ʽд����
  .Check = 0x03,      //���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
  .SetBaudRate = 0x04,//���ýڵ㲨����
  .Excute = 0x05,     //ִ�й̼�
  .CmdSuccess = 0x08, //����ִ�гɹ�
  .CmdFaild = 0x09,   //����ִ��ʧ��
};
extern CanRxMsg CAN1_RxMessage;
extern volatile uint8_t CAN1_CanRxMsgFlag;//���յ�CAN���ݺ�ı�־
extern volatile uint8_t TimeOutFlag;				///<��ʱ����ʱ��־
volatile uint16_t BOOT_TimeOutCount;
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
/**
  * @brief  ���ݵ�ַ�����������ó���������ʽ
  * @param  None
  * @retval None
  */
void BOOT_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
	/*Configure Addr Pin*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	if(CAN_BOOT_GetAddrData()==0x00){
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
		// ʹ������ʱ��
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
		/* ʱ�Ӽ���Ƶ���� */
		/* 30M/30000 = 1ms */
		TIM_TimeBaseStructure.TIM_Prescaler = 72-1;// Ԥ��Ƶ����Ƶ���Ƶ��Ϊ1K    
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //����ģʽ:���ϼ���
		// TIM������ֵ��������ʱʱ�䳤�ȣ�
		TIM_TimeBaseStructure.TIM_Period =1000;	  // ��������ֵ�����ϼ���ʱ����������ֵ��������¼���ʱ���Ӹ�ֵ��ʼ����
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // �������˲����Ĳ������й�       
		TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;  //���¼�������ʼֵ
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

		TimeOutFlag = 0;
		
		/* Disable the TIM Counter */
		TIM2->CR1 &= (uint16_t)(~TIM_CR1_CEN);

		TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); //��������ж�
		
		/* Enable the TIM2 global Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;							// �ж�Դ
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 	// ��ռ���ȼ�
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;					// �����ȼ�
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 						// ʹ���ж�
		NVIC_Init(&NVIC_InitStructure);		
		
		/* 	Enable the TIM Counter */
		TIM2->CR1 |= (uint16_t)TIM_CR1_CEN;
		while(CAN_BOOT_GetAddrData()==0x00){
			if(TimeOutFlag){
				TimeOutFlag = 0;
				__set_PRIMASK(1);//�ر������ж�
				FLASH_Unlock();
				CAN_BOOT_ErasePage(APP_EXE_FLAG_START_ADDR,APP_START_ADDR);
				__set_PRIMASK(0);//���������ж�
			}
		}
		/* Disable the TIM Counter */
		TIM2->CR1 &= (uint16_t)(~TIM_CR1_CEN);
	}
	if((*((uint32_t *)APP_EXE_FLAG_START_ADDR)==0x12345678)&&(*((uint32_t *)APP_START_ADDR)!=0xFFFFFFFF)){
		CAN_BOOT_JumpToApplication(APP_START_ADDR);	
	}
	__set_PRIMASK(0);//�������ж�
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000); //����ӳ���ж�������
}

/**
  * @brief  TIM��ʱ����ʱ�жϴ�����
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)== SET)
	{
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update); //��������־
		BOOT_TimeOutCount++;
	}
}

/**
  * @brief  ���ݵ�ַ�����������ó���������ʽ
  * @param  None
  * @retval None
  */
void BOOT_Delay_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	__set_PRIMASK(0);//�������ж�
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000); //����ӳ���ж�������
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	// ʹ������ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	/* ʱ�Ӽ���Ƶ���� */
	/* 30M/60000 = 1ms */
	TIM_TimeBaseStructure.TIM_Prescaler = 72-1;// Ԥ��Ƶ����Ƶ���Ƶ��Ϊ1M   
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //����ģʽ:���ϼ���
	// TIM������ֵ��������ʱʱ�䳤�ȣ�1
	TIM_TimeBaseStructure.TIM_Period =1000;	  // ��������ֵ�����ϼ���ʱ����������ֵ��������¼���ʱ���Ӹ�ֵ��ʼ����
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // �������˲����Ĳ������й�       
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;  //���¼�������ʼֵ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TimeOutFlag = 0;
		
	/* Disable the TIM Counter */
	TIM3->CR1 &= (uint16_t)(~TIM_CR1_CEN);

	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //��������ж�
	
	/* Enable the TIM2 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;							// �ж�Դ
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 	// ��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;					// �����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 						// ʹ���ж�
	NVIC_Init(&NVIC_InitStructure);		
		
	/* 	Enable the TIM Counter */
	TIM3->CR1 |= (uint16_t)TIM_CR1_CEN;
	BOOT_TimeOutCount = 0;
}

/**
  * @brief  ��������ʵ��LED�Ƶ���˸
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
  this is done through SystemInit() function which is called from startup
  file (startup_stm32fxxx_xx.s) before to branch to application main.
  To reconfigure the default setting of SystemInit() function, refer to
  system_stm32fxxx.c file
  */
	//ʱ�����ú�����system_stm32f10x.c�ļ���SetSysClockTo72()������
  if(*((uint32_t *)APP_EXE_FLAG_ADDR)==0x78563412){
    CAN_BOOT_JumpToApplication(APP_START_ADDR);
  }
  __set_PRIMASK(0);//�������ж�
  CAN_Configuration(1000000);

// 	/* Enable the Flash option control register access */
// 	FLASH_OB_Unlock();
// 	/* Enable FLASH_WRP_SECTORS write protection */
// 	FLASH_OB_WRPConfig(OB_WRP_Sector_0, ENABLE); 
// 	FLASH_OB_Launch();
// 	FLASH_OB_Lock();	
	
// 	if(FLASH_OB_GetRDP() != SET)
// 	{
// 		FLASH_OB_Unlock();
// 		FLASH_OB_RDPConfig(OB_RDP_Level_1); 
// 		FLASH_OB_Launch();
// 		FLASH_OB_Lock();		
// 	}
  while (1)
  {
    if(CAN1_CanRxMsgFlag){
      CAN_BOOT_ExecutiveCommand(&CAN1_RxMessage);
      CAN1_CanRxMsgFlag = 0;
    }
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/***********************************�ļ�����***********************************/
