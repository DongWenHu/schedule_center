#ifndef _PROTOCOL_DEFINE_H_2016_04_28_
#define _PROTOCOL_DEFINE_H_2016_04_28_

//----------------------------LUA ����-------------------------------------
const char* const CMD_LUA_EXIT = "90000001";    //�˳��ű�
const char* const CMD_UPLOAD_FILE = "90000002";    //�ϴ��ļ�
const char* const CMD_DOWNLOAD_FILE = "90000003";    //�����ļ�

const char* const CMD_WX_LOGIN = "00000001";    //��¼
const char* const CMD_WX_LOGOUT = "00000002";    //�ǳ�
const char* const CMD_WX_CHANG_USR = "00000003";    //�л��û�
const char* const CMD_WX_ADD_FRIEND = "00000004";    //��Ӻ���
const char* const CMD_WX_CREATE_GROUP = "00000005";    //����Ⱥ��
const char* const CMD_WX_SEND_MSG = "00000006";    //���͵���
const char* const CMD_WX_SEND_GROUP_MSG = "00000007";    //����Ⱥ��
const char* const CMD_WX_ADD_MP = "00000008";    //��ӹ��ں�
const char* const CMD_WX_COMMENT_SNS = "00000009";    //��������Ȧ
const char* const CMD_WX_UPVOTE_SNS = "00000010";    //��������Ȧ
const char* const CMD_WX_VOTE = "00000011";    //΢��ͶƱ
const char* const CMD_WX_SEND_SNS = "00000012";    //������Ȧ
const char* const CMD_WX_UPDATE_NICKNAME = "00000013";    //�����ǳ�
const char* const CMD_WX_ADD_NEAR_FRIEND = "00000014";    //ͨ������������Ӻ���
const char* const CMD_WX_UPDATE_SEX = "00000015";    //�����Ա�
const char* const CMD_WX_RANDOM_SEND_MSG = "00000016";    //��������ѷ���Ϣ
const char* const CMD_WX_RANDOM_SEND_SNS = "00000017";    //���������Ȧ

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


//----------------------------APP ����-------------------------------------
const int CMD_APP_HEARTBEAT         =       0x00000000; // ����
const int CMD_APP_START_LUA         =       0x00000001; //  ����lua�ű�
const int CMD_APP_RESTART_LUA       =       0x00000002; //  ����lua�ű�
const int CMD_APP_STOP_LUA          =       0x00000003; //  ֹͣlua�ű�
const int CMD_APP_REBOOT            =       0x00000004; //  �����豸
const int CMD_APP_INSTALL_APK       =       0x00000005; //  ��װAPK

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


//----------------------------schedule ����--------------------------------
const int CMD_SCHE_WX_VOTE = 0x10000000;    //  ͶƱ����


//----------------------------some port------------------------------------
const unsigned short PHONE_PORT     = 9528;
const unsigned short SCHEDULE_PORT  = 9529;
const unsigned short SCRIPT_PORT    = 9527;

#endif //    _PROTOCOL_DEFINE_H_2016_04_28_
