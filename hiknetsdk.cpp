#include "hiknetsdk.h"
#include "ui_hiknetsdk.h"
#include <QDebug>
#include <QDateTime>

HikNetSdk::HikNetSdk(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HikNetSdk)
{
    ui->setupUi(this);

}

HikNetSdk::~HikNetSdk()
{
    NET_DVR_Cleanup();
    delete ui;
}

void HikNetSdk::LoginInfo(qint16 Port,QString sDeviceAddress,QString sUserName, QString sPassword)
{	//AT last need init SDK
    if (!NET_DVR_Init())
    {
        int err = NET_DVR_GetLastError();
        qDebug()<< NET_DVR_GetErrorMsg(&err);
        return;
    }
    NET_DVR_SetConnectTime(10000, 3);
    NET_DVR_SetReconnect(50000, true);
    NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack, NULL);
    //Login
    int ret = -1;

    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
    struLoginInfo.bUseAsynLogin = false;

    struLoginInfo.wPort = Port;
    strcpy(struLoginInfo.sDeviceAddress, sDeviceAddress.toUtf8().data());
    strcpy(struLoginInfo.sUserName, sUserName.toUtf8().data());
    strcpy(struLoginInfo.sPassword, sPassword.toUtf8().data());

    ret = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    memcpy(&m_deviceinfo, &struDeviceInfoV40.struDeviceV30, sizeof(NET_DVR_DEVICEINFO_V30));


        if (-1 == ret )
        {
            int err = NET_DVR_GetLastError();
            qDebug()<< NET_DVR_GetErrorMsg(&err);
            return;
        }
        else
        {
            m_iuserid = ret;
        }
        //memcpy(&m_gcurrentdeviceinfo, &m_deviceinfo,sizeof(NET_DVR_DEVICEINFO_V30));

}

void HikNetSdk::Play()
{

    long realhandle;
    struPlayInfo.lChannel     = 1;  //channel NO
    struPlayInfo.dwLinkMode   = 0;
    struPlayInfo.dwStreamType   = 1;// Stream type 0-main stream,1-sub stream,2-third stream,3-forth
    struPlayInfo.byRecvMetaData = 1;
    struPlayInfo.hPlayWnd = winId();
    struPlayInfo.bBlocked = 1;
    struPlayInfo.dwDisplayBufNum = 1;
    realhandle = NET_DVR_RealPlay_V40(0, &struPlayInfo, RealDataCallBack, NULL);
    qDebug() << "realhandle" << realhandle;
    if (!PlayM4_SetDecCallBack(0,DecCBFun))
    {
      qDebug() << PlayM4_GetLastError(0);
    }
    if (!PlayM4_RenderPrivateData(0, PLAYM4_RENDER_ANA_INTEL_DATA, true))
    {
      qDebug() << PlayM4_GetLastError(0);
    }

}


/**  @fn  void __stdcall  RealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD  dwBufSize, void* dwUser)
 *   @brief data callback funtion
 *   @param (OUT) LONG lRealHandle
 *   @param (OUT) DWORD dwDataType
 *   @param (OUT) BYTE *pBuffer
 *   @param (OUT) DWORD  dwBufSize
 *   @param (OUT) void* dwUser
 *   @return none
 */
void __stdcall  HikNetSdk::RealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD  dwBufSize, void* dwUser)
{
    qDebug() << "pBuffer" << pBuffer;
    if (dwUser != NULL)
    {
        qDebug() << "init error..." << NET_DVR_GetLastError();
        qDebug("Demmo lRealHandle[%d]: Get StreamData! Type[%d], BufSize[%d], pBuffer:%p\n", lRealHandle, dwDataType, dwBufSize, pBuffer);
    }
}

void __stdcall HikNetSdk::g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = {0};
    switch(dwType)
    {
    case EXCEPTION_RECONNECT:
        qDebug() << "init error..." << NET_DVR_GetLastError();
        qDebug() <<"reconnect--------" << QDateTime::currentDateTime().toString();
    break;
    default:
    break;
    }
}

void __stdcall HikNetSdk::DecCBFun(int nPort,char * pBuf,int nSize,FRAME_INFO * pFrameInfo, void* nReserved1,int nReserved2)
{
//    qDebug("TYPE:%d-[%d*%d]",pFrameInfo->nType,pFrameInfo->nWidth,pFrameInfo->nHeight);
    switch (pFrameInfo->nType) {
    case T_YV12:
    {

    }
        break;
    case T_AUDIO8:
    case T_AUDIO16:

        break;
    default:
        break;
    }
}
