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
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

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
  if(*((uint32_t *)APP_EXE_FLAG_START_ADDR)==0x78563412){
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
