#ifndef HIKNETSDK_H
#define HIKNETSDK_H

#include <QMainWindow>
#if defined(_WIN32)
#include <Windows.h>
#pragma warning( disable : 4996)
#else
#endif

#include "include/HCNetSDK.h"
#include "include/PlayM4.h"

namespace Ui {
class HikNetSdk;
}

class HikNetSdk : public QMainWindow
{
    Q_OBJECT

public:
    explicit HikNetSdk(QWidget *parent = nullptr);
    ~HikNetSdk();
    void LoginInfo(qint16 Port,QString sDeviceAddress,QString sUserName, QString sPassword);
    void Play();

private:
    Ui::HikNetSdk *ui;

    NET_DVR_CLIENTINFO clientinfo;
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    NET_DVR_DEVICEINFO_V30 m_gcurrentdeviceinfo;

    //device data

    //ÕýÔÚÔ¤ÀÀ»òÂ¼Ïñ0¼Ù1Õæ
    int     m_irealplaying;
    //µÇÂ½Éè±¸ºó·µ»ØµÄÓÃ»§ID£»²»´æÈëÎÄ¼þ
    int     m_iuserid;
    //µÇÂ½Éè±¸ºó·µ»ØÉè±¸ÐÅÏ¢£»²»´æÈëÎÄ¼þ
    NET_DVR_DEVICEINFO_V30 m_deviceinfo;
    //Éè±¸Ãû³Æ£¬´æÈëÎÄ¼þ
    QString m_qdevicename;
    //Éè±¸IP£¬´æÈëÎÄ¼þ
    QString m_qip;
    //Éè±¸¶Ë¿Ú,´æÈëÎÄ¼þ
    int     m_qiport;
    //ÓÃ»§Ãû£¬´æÈëÎÄ¼þ
    QString m_qusername;
    //ÓÃ»§ÃÜÂë£¬´æÈëÎÄ¼þ
    QString m_qpassword;
    //²¼·ÀÖÐ >=0  ·ñÔò -1
    int m_ideployed;
    //¶à²¥ipµØÖ·
    QString m_multiCast;
    //×ÓÊôÐÔÍ¨µÀ½ÚµãÁÐ±í£¬ÆäÄÚÈÝÒ²Òª´æÈëÎÄ¼þ£»
    //QList<ChannelData> m_qlistchanneldata;

    static void __stdcall  RealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD  dwBufSize, void* dwUser);
    static void __stdcall g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);
    static void __stdcall DecCBFun(int nPort,char * pBuf,int nSize,FRAME_INFO * pFrameInfo, void* nReserved1,int nReserved2);
};

#endif // HIKNETSDK_H
