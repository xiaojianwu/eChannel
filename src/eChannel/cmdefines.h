// Define.h

#ifndef _COMMAND_DEFINE_H_
#define _COMMAND_DEFINE_H_

enum CommandType {
	MGC_COMMAND_HEARTBEAT		= 65,	// 心跳
	MGC_COMMAND_REGISTER		= 66,	//	注册
	MGC_COMMAND_REGITER_RESP	= 67,	// 注册返回
	MGC_COMMAND_INVITE			= 68,	// 邀请
	MGC_COMMAND_INVITE_RESP		= 69,	// 邀请返回
	MGC_COMMAND_BYE_REQ			= 75,	// 请求解除连接
	MGC_COMMAND_BYE_RESP		= 76,	// 请求解除连接响应
	MGC_TYPE_TURN_OK			= 81,	// TURN 响应处理请求通道
};

enum InviteCodeType {
	MGC_CODE_RING			= 180,	// 邀请等待处理
	MGC_CODE_REJECT			= 487,	// 拒绝邀请
	MGC_CODE_ACCEPT			= 200,	// 接受邀请
	MGC_CODE_NOT_FOUND		= 404,	// 未找到接受方
};

enum TurnCommands {
	TURN_TYPE_TURN_REQ		= 513, // UAC 需要TURN为UAC创建数据通道
	TURN_TYPE_TURN_RESP		= 514, // TURN 响应处理请求通道
};

#endif // _COMMAND_DEFINE_H_
