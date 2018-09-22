/*
 * CAN_APP.c
 *
 *  Created on: 2017��5��21��
 *      Author: admin
 */
#include "CAN_APP.h"
typedef  void (*pFunction)(void);

Boot_CMD_LIST cmd_list =
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

Device_INFO DEVICE_INFO =
{
	.FW_TYPE = CAN_BL_APP,
	.FW_Version = 0x0010001,
};

void __disable_irq(void)
{
	DINT;
	DRTM;
}
void __enable_irq(void)
{
	EINT;
	ERTM;
}
void __set_PRIMASK(u8 state)
{
	if(state == 1)
	{
		__disable_irq();
	}
	else if(state == 0)
	{
		__enable_irq();
	}
	else
	{
		return;
	}
}

void CAN_BOOT_JumpToApplication(uint32_t Addr)
{
	(*((void(*)(void))(Addr)))();
}

#pragma CODE_SECTION(CAN_BOOT_ExecutiveCommand,"ramfuncs");
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage)
{
	bootloader_data Boot_ID_info;
	u8 can_cmd    = 0x00;//ID��bit0~bit3λΪ������
	u16 can_addr  = 0x00;//ID��bit4~bit15λΪ�ڵ��ַ
	uint32_t exe_type = 0x00;
	CanTxMsg TxMessage;//���Ͷ�Ӧ��Ϣ
	TxMessage.CAN_num = CANA;
	TxMessage.DLC = 1;
	TxMessage.ExtId.bit.resved = 0x00;
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.MBox_num = 0x03;
	TxMessage.Tx_timeout_cnt = 0x00;
	TxMessage.SAE_J1939_Flag = 0;
	//��ȡ��ַ��Ϣ
	Boot_ID_info.ExtId.all = pRxMessage->ExtId;
	can_cmd  = Boot_ID_info.ExtId.bit.cmd;
	can_addr = Boot_ID_info.ExtId.bit.addr;
	//�жϽ��յ����ݵ�ַ�Ƿ�ͱ��ڵ��ַƥ�䣬����ƥ����ֱ�ӷ��أ������κ�����
	if((can_addr!=CAN_BOOT_GetAddrData())&&(can_addr!=0)){
		return;
	}
	//CMD_List.Check���ڵ����߼��
	//�ڵ��յ�������󷵻ع̼��汾��Ϣ�͹̼����ͣ�
	//��������Bootloader�����APP���򶼱���ʵ��
	if(can_cmd == cmd_list.Check)//DSP����δʵ��,��ԱȽ�����ʵ��,��Ҫ��Ϊʵ��APP�ٴθ���Ӧ�ó���
	{
		if(can_addr != 0x00)
		{
			DEVICE_INFO.Device_addr.bits.Device_addr = CAN_BOOT_GetAddrData();
			TxMessage.ExtId.bit.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdSuccess;
			TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = (u8)(DEVICE_INFO.FW_Version>>24);;//���汾�ţ����ֽ�
			TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = (u8)(DEVICE_INFO.FW_Version>>16);
			TxMessage.CAN_Tx_msg_data.msg_byte.data[2] = (u8)(DEVICE_INFO.FW_Version>>8);//�ΰ汾�ţ����ֽ�
			TxMessage.CAN_Tx_msg_data.msg_byte.data[3] = (u8)(DEVICE_INFO.FW_Version>>0);
			TxMessage.CAN_Tx_msg_data.msg_byte.data[4] = (u8)(DEVICE_INFO.FW_TYPE>>24);
			TxMessage.CAN_Tx_msg_data.msg_byte.data[5] = (u8)(DEVICE_INFO.FW_TYPE>>16);
			TxMessage.CAN_Tx_msg_data.msg_byte.data[6] = (u8)(DEVICE_INFO.FW_TYPE>>8);
			TxMessage.CAN_Tx_msg_data.msg_byte.data[7] = (u8)(DEVICE_INFO.FW_TYPE>>0);
			TxMessage.DLC = 8;
			CAN_Tx_Msg(&TxMessage);
		}
	}
	//CMD_List.Excute�����Ƴ�����ת��ָ����ִַ��
	//��������Bootloader��APP�����ж�����ʵ��
	if(can_cmd == cmd_list.Excute)//��������DSP���Ѿ�ʵ��
	{
		exe_type  = (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[0])&0xFFFFFFFF)<<24)|\
					(((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[1])&0x00FFFFFF)<<16)|\
					(((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[2])&0x0000FFFF)<<8)|\
					(((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[3])&0x000000FF)<<0);
		if(exe_type == CAN_BL_BOOT)
		{
			if((*((uint32_t *)BOOT_START_ADDR)!=0xFFFFFFFF))
			{
				CAN_BOOT_JumpToApplication(BOOT_START_ADDR);
			}
		}
		return;
	}
	return;

}


