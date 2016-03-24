// Define.h

#ifndef _COMMAND_DEFINE_H_
#define _COMMAND_DEFINE_H_

enum CommandType {
	MGC_COMMAND_HEARTBEAT		= 65,	// ����
	MGC_COMMAND_REGISTER		= 66,	//	ע��
	MGC_COMMAND_REGITER_RESP	= 67,	// ע�᷵��
	MGC_COMMAND_INVITE			= 68,	// ����
	MGC_COMMAND_INVITE_RESP		= 69,	// ���뷵��
	MGC_COMMAND_BYE_REQ			= 75,	// ����������
	MGC_COMMAND_BYE_RESP		= 76,	// ������������Ӧ
	MGC_TYPE_TURN_OK			= 81,	// TURN ��Ӧ��������ͨ��
};

enum InviteCodeType {
	MGC_CODE_RING			= 180,	// ����ȴ�����
	MGC_CODE_REJECT			= 487,	// �ܾ�����
	MGC_CODE_ACCEPT			= 200,	// ��������
	MGC_CODE_NOT_FOUND		= 404,	// δ�ҵ����ܷ�
};

enum TurnCommands {
	TURN_TYPE_TURN_REQ		= 513, // UAC ��ҪTURNΪUAC��������ͨ��
	TURN_TYPE_TURN_RESP		= 514, // TURN ��Ӧ��������ͨ��
};

#endif // _COMMAND_DEFINE_H_
