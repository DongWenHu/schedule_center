#ifndef _PROTOCOL_DEFINE_H_2016_04_28_
#define _PROTOCOL_DEFINE_H_2016_04_28_

//----------------------------LUA 命令-------------------------------------
const char* const CMD_LUA_EXIT = "90000001";    //退出脚本
const char* const CMD_UPLOAD_FILE = "90000002";    //上传文件
const char* const CMD_DOWNLOAD_FILE = "90000003";    //下载文件

const char* const CMD_WX_LOGIN = "00000001";    //登录
const char* const CMD_WX_LOGOUT = "00000002";    //登出
const char* const CMD_WX_CHANG_USR = "00000003";    //切换用户
const char* const CMD_WX_ADD_FRIEND = "00000004";    //添加好友
const char* const CMD_WX_CREATE_GROUP = "00000005";    //创建群聊
const char* const CMD_WX_SEND_MSG = "00000006";    //发送单聊
const char* const CMD_WX_SEND_GROUP_MSG = "00000007";    //发送群聊
const char* const CMD_WX_ADD_MP = "00000008";    //添加公众号
const char* const CMD_WX_COMMENT_SNS = "00000009";    //评论朋友圈
const char* const CMD_WX_UPVOTE_SNS = "00000010";    //点赞朋友圈
const char* const CMD_WX_VOTE = "00000011";    //微信投票
const char* const CMD_WX_SEND_SNS = "00000012";    //发朋友圈
const char* const CMD_WX_UPDATE_NICKNAME = "00000013";    //更改昵称
const char* const CMD_WX_ADD_NEAR_FRIEND = "00000014";    //通过附近的人添加好友
const char* const CMD_WX_UPDATE_SEX = "00000015";    //更改性别
const char* const CMD_WX_RANDOM_SEND_MSG = "00000016";    //随机给好友发消息
const char* const CMD_WX_RANDOM_SEND_SNS = "00000017";    //随机发朋友圈

typedef struct _MSG_CMD_LUA
{
    std::string len;
    std::string json_data;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & len & json_data;
    }
}MSG_CMD_LUA;


//----------------------------APP 命令-------------------------------------
const int CMD_APP_HEARTBEAT         =       0x00000000; // 心跳
const int CMD_APP_START_LUA         =       0x00000001; //  启动lua脚本
const int CMD_APP_RESTART_LUA       =       0x00000002; //  重启lua脚本
const int CMD_APP_STOP_LUA          =       0x00000003; //  停止lua脚本
const int CMD_APP_REBOOT            =       0x00000004; //  重启设备
const int CMD_APP_INSTALL_APK       =       0x00000005; //  安装APK

typedef struct _MSG_CMD_APP
{
    int cmd;
    int result;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & cmd & result;
    }
}MSG_CMD_APP;


//----------------------------schedule 命令--------------------------------
const int CMD_SCHE_WX_VOTE = 0x10000000;    //  投票命令


//----------------------------some port------------------------------------
const unsigned short PHONE_PORT     = 9528;
const unsigned short SCHEDULE_PORT  = 9529;
const unsigned short SCRIPT_PORT    = 9527;

#endif //    _PROTOCOL_DEFINE_H_2016_04_28_
