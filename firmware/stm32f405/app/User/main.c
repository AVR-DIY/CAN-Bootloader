/**
  ******************************************************************************
  * @file    main.c
  * $Author: �ɺ�̤ѩ $
  * $Revision: 17 $
  * $Date:: 2014-10-25 11:16:48 +0800 #$
  * @brief   ������.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, EmbedNet</center>
  *<center><a href="http:\\www.embed-net.com">http://www.embed-net.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "delay.h"
#include "can_app.h"
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
/**
  * @brief  ���ڴ�ӡ���
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
  if(*((uint32_t *)APP_EXE_FLAG_ADDR)==0xFFFFFFFF){
    __align(4) static unsigned char data[4]={0x12,0x34,0x56,0x78};
    FLASH_Unlock();
    CAN_BOOT_ProgramDatatoFlash(APP_EXE_FLAG_ADDR,data,4);
    FLASH_Lock();
  }
  __set_PRIMASK(0);//�������ж�
  delay_init(168);
  CAN_Configuration(1000000);
  while (1)
  {
    if(CAN1_CanRxMsgFlag){
      CAN_BOOT_ExecutiveCommand(&CAN1_RxMessage);
      CAN1_CanRxMsgFlag = 0;
    }
  }
} // end of main()


/*********************************END OF FILE**********************************/
