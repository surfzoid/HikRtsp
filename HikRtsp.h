#ifndef HIKRTSP_H
#define HIKRTSP_H

#include <QMainWindow>

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QIODevice>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <include/PlayM4.h>
#include <QVideoWidget>
#include <QBitArray>
#include "h264rtpsource.h"
#include "rtpparser.h"
#include "settingsform.h"
#include "hiknetsdk.h"
#define  BOOL  int
typedef  unsigned int       DWORD;
typedef  unsigned short     WORD;
typedef  unsigned short     USHORT;
typedef  short              SHORT;
typedef  int                LONG;
typedef  unsigned char      BYTE;
typedef  unsigned int       UINT;
typedef  void*              LPVOID;
typedef  void*              HANDLE;
typedef  unsigned int*      LPDWORD;
typedef  unsigned long long UINT64;
typedef  signed long long   INT64;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#define __stdcall
#define CALLBACK

#ifndef __HWND_defined
#define __HWND_defined
#if defined(__linux__)
typedef unsigned int HWND;
#else
typedef void* HWND;
#endif
#endif

class readyRead;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Connect();
    int *m_rpuserid;
    void NMfinished();
    static void CALLBACK DecCBFun(int nPort, char* pBuf, int nSize, FRAME_INFO* pFrameInfo, void* nUser, int nReserved2);
    static void CALLBACK SourceBufCallBack(int nPort, unsigned int nBufSize, unsigned int dwUser, void* pResvered);

signals:
public slots:
    void replyFinished (QNetworkReply *reply);
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

private:
    Ui::MainWindow *ui;
    QString Token;
    static QVideoWidget *videoWidget;
    void Play();
    QTcpSocket *m_socket;
    QNetworkReply *reply = nullptr;

    //static QByteArray Resp;
    void readData(QByteArray Resp);
    void FromDocEx();
    void Sleep(int MSecs);
    bool EndRead = false;
    void WaitEndRead();
    QNetworkAccessManager *manager;

    /********Rtsp Headers******/
    unsigned int CSeq = 0;
    unsigned int LastCSeq = 1;
    QString RtspUri = "rtsp://admin:pass@192.168.0.0:554/ISAPI/streaming/channels/102";
    QString realm="";
    QString nonce="";
    QString SessionID="";
    QString response="";
    QString public_method="";

    void RtspHeaders();
    void ParseResp(QByteArray Resp, QString public_method);
    QString FindRTSPVar(QString Str, QString Line);
    QStringList FindRTSPVar(QString Line);
    QByteArray MD5Compute(QByteArray Str);
    void Authanticate(QString public_method);
    QString ReqSetUrl="";
    QString Reqdest="";
    QString ReqdestNoDigest="";
    QString Reqsetu="";
    QString ReqOption="";
    QString Reqplay="";
    QString ReqStop="";
    void SendReq(const QString &Req);
    QStringList ReqList;
    void ParseSdpResp(QByteArray Resp);
    bool NeedAuth = false;
    int FaillAuthCnt = 0;
    bool IsRTSB = false;
    bool IsPlayM4 = false;
    bool GotSdp = false;
    QByteArray Magic = QByteArray::fromHex("2400");
    int Nexti = 0;
    int NextiHead = 0;
    QString HIKHeader;
    QString Transport ="RTP/AVP/TCP;broadcast;interleaved=0-1";

    PLAYM4_SESSION_INFO* pstSessionInfo;
    unsigned char* SdpData;

    unsigned int nThreShold;
    unsigned int dwUser;
    void* pReserved;
    QStringList trackID = { "/trackID=1", "/trackID==3" };
    QString ssrc;//synchronization source (SSRC) identifier
    QString sps;//Sequence Parameter Set
    QString pps;//Picture Parameter Set
    static const int RtpHeaderLength = 12;
    static QByteArray PayLTmpBuff;
    int NeedMoreData = 0;
    QByteArray PadingRemove(QByteArray Payload);
    bool FullPayload = false;

    void InitM4();
    void PlayM4(QByteArray Payload);

    QByteArray SocketRead;

    static SettingsForm *Settings;
    void InitSettings();

    HikNetSdk *VideoV;
    QString MP4Header = "49 4d 4b 48 01 02 00 00 02 00 05 00 00 00 00 00 00 00 00 00 00 00 00 00 81 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 ba 5f 8a 5d 19 a4 01 03 ac d3 fe ff ff 00 2b 51 36 00 00 01 bb 00 12 81 d6 69 04 e1 7f e0 e0 80 c0 c0 08 bd e0 80 bf e0 80 00 00 01 bc 00 5a f6 ff 00 24 40 0e 48 4b 01 00 16 ce 04 6b 74 97 00 ff ff ff 41 12 48 4b 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 00 2c 24 e0 00 28 42 0e 07 10 10 ea 0a 20 07 98 11 10 c0 00 23 29 44 0a 00 00 80 00 00 00 00 00 ff ff 2a 0a 7f ff 00 00 08 ca 1f fe 44 f3 b1 8c 5f f8 00 00 01 e0 00 26 8c 80 08 27 e2 97 46 69 ff ff fc 00 00 00 01 40 01 0c 01 ff ff 01 60 00 00 03 00 b0 00 00 03 00 00 03 00 99 ac 09 00 00 01 e0 00 2e 8c 00 04 ff ff ff fc 00 00 00 01 42 01 01 01 60 00 00 03 00 b0 00 00 03 00 00 03 00 99 a0 01 44 20 07 99 63 6b 92 45 2f cd c0 40 40 40 20 00 00 01 e0 00 12 8c 00 02 ff fc 00 00 00 01 44 01 c0 f2 c6 8d 03 b2 40 00 00 01 e0 00 12 8c 00 02 ff fc 00 00 00 01 4e 01 e5 04 4a 25 00 00 80 00 00 01 e0 ff c6 8c 00 03 ff ff fd 00 00 00 01 26 01 af 25 e0 f9 85 3e 39 57 9e 6d 88 9b 10 8d c2 ba df 05 50 6c 87 54 b4 2a 07 a4 c1 96 76 4f 79 52 ce 68 ed a5 e5 a1 80 12 aa bb f9 19 74 d5 4a cb 21 cf 8a 1b 85 94 75 b2 2b 17 4e 35 59 4a 1b b2 89 52 b2 66 78 c5 8e f5 bf 7b 30 2d e9 0a 80 26 56 2c ee 47 29 45 2f e8 02 ac 29 49 39 e5 51 05 f3 ff 57 7a 07 97 07 5d 6a 1e 97 f7 02 b0 06 a7 44 c2 f9 01 9d 48 c1 cc 32 e6 9c 33 3e e4 05 f8 d4 32 31 20 0a 6c 00 73 f0 3f";
    protected slots:
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
    void slotError(QNetworkReply::NetworkError code);
    void slotSslErrors(const QList<QSslError> &errors);
protected:

private slots:
    void on_actionPlay_triggered();
    void on_actionDESCRI_triggered();
    void on_actionSETUP_URL_triggered();
    void on_actionSETUP_triggered();
    void on_actionPLAY_triggered();
    void on_actionOPTION_triggered();
    void on_actiontest_triggered();
    void on_actionAuthor_triggered();
    void on_actionTEARDOWN_triggered();

    void on_actionDESC_no_DIGEST_triggered();

    void on_actionSettings_triggered();

    void on_comboBoxUris_currentIndexChanged(const QString &arg1);

    void on_comboBoxChanel_currentIndexChanged(const QString &arg1);

    void on_comboBoxUris_currentIndexChanged(int index);

protected:
    void resizeEvent(QResizeEvent *event) override;
};
#endif // HIKRTSP_H
