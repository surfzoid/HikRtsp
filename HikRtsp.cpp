#include "HikRtsp.h"
#include "ui_HikRtsp.h"
#include <QFile>
#include <QResource>
#include <QAuthenticator>
#include <cstring>
#include <QBuffer>
#include <QMediaPlayer>
#include <QUrl>
#include <QDataStream>

LONG  g_nPort  = -1;
QFile Fs("/tmp/tmp.h265");
#define READ_BUF_SIZE 200
#define HIK_HEAD_LEN 40

QByteArray MainWindow::PayLTmpBuff;
QVideoWidget *MainWindow::videoWidget;
SettingsForm *MainWindow::Settings;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //settings//
    QCoreApplication::setOrganizationName("Surfzoid");
    QCoreApplication::setOrganizationDomain("https://github.com/surfzoid");
    QCoreApplication::setApplicationName("HikRtsp");
    InitSettings();

    /*videoWidget = new QVideoWidget(this);
    setCentralWidget(videoWidget);*/

    ReqList << "OPTIONS" << "DESCRIBE" << "SETUP" << "PLAY" << "TEARDOWN" << "DESCRIBENOA" ;

    if (Fs.isOpen())
        Fs.close();
    Fs.open(QIODevice::OpenModeFlag::WriteOnly);
    RTPParser Init = RTPParser();


    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            this, SLOT(authenticationRequired(QNetworkReply*, QAuthenticator*)));

    VideoV = new HikNetSdk();
    //MainWindow::Connect();

}

MainWindow::~MainWindow()
{
    if (public_method == "PLAY")
        MainWindow::on_actionTEARDOWN_triggered();
    delete ui;
}

int MaxRetry = 0;
void MainWindow::Connect()
{

    QUrl url(RtspUri);


    m_socket = new QTcpSocket(this);
    connect(m_socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    //old method before QT5
    //connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten()));
    //new method since QT5
    connect(m_socket, &QIODevice::bytesWritten, this, &MainWindow::bytesWritten);

    quint16 RPort = 554;
    if (url.port() != -1)RPort = url.port();
    //m_socket->connectToHost(url.host(),RPort);
    m_socket->connectToHost(Settings->CamIp,8000);
    //m_socket->setReadBufferSize(10240);
    //m_socket->setProtocolTag("RTSP");
    //char *RBuff ;
    //qint64 LenRead = 0;
    if(m_socket->waitForConnected(3000))
    {
        /*while (m_socket->ConnectedState == QAbstractSocket::ConnectedState) {
            if (IsRTSB) {
                LenRead = m_socket->read(RBuff,-1);
                SocketRead = RBuff;
                if (!SocketRead.isEmpty())
                {
                    readData(SocketRead);
                    //SocketRead.remove(0,LenRead);
                    Sleep(100);
                }
            }else
            {
                SocketRead = m_socket->readAll();
                if (!SocketRead.isEmpty())
                {
                    readData(SocketRead);
                    //SocketRead.clear();
                    //Sleep(1000);
                }
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }*/

        //m_socket->close();*/
    }
    else
    {
        qDebug() << "Not connected!" << m_socket->errorString();
        if (MaxRetry < 6) {
            MaxRetry += 1;
            qDebug() << "Try again to connect :" << MaxRetry;
            FaillAuthCnt = 0;
            IsPlayM4 = false;
            GotSdp = false;
            MainWindow::Connect();
        }
    }

}

void MainWindow::connected()
{
    qDebug() << "Connected!";
    qDebug() << ":" << m_socket->readAll();
    CSeq = 1;
    realm="";
    nonce="";
    SessionID="";
    response="";
    RtspHeaders();
    m_socket->setSocketOption(QAbstractSocket::KeepAliveOption,true);

    SendReq("DESCRIBENOA");
    //SendReq("OPTIONS");
}

void MainWindow::disconnected()
{
    qDebug() << "Disconnected!";
}

void MainWindow::bytesWritten(qint64 bytes)
{
    qDebug() << "We wrote: " << bytes;
}

//QByteArray TmpRead;
void MainWindow::readyRead()
{
    if(m_socket->bytesAvailable() == 0)
    {
        return;
    }
    /*while (m_socket->bytesAvailable()) {
        TmpRead = m_socket->read(1440);
        if (IsRTSB) {
            //TmpRead.remove(0,TmpRead.indexOf(Magic));
        }
       readData(TmpRead);
        Sleep(100);
    }*/

    qDebug() << "Reading..." << m_socket->bytesAvailable();

    if (!IsPlayM4)
        readData(m_socket->readAll());
    if (m_socket->bytesAvailable() < 144000)
        return;
    readData(m_socket->readAll());

}

int dbgtmp = 0;
int PayNum = 0;
void MainWindow::readData(QByteArray Resp)
{

    QByteArray RTPPacket;
    QByteArray Payload;

    IsRTSB = false;

    if (!Resp.startsWith("RTSP") && public_method == "PLAY") {
        IsRTSB = true;
        InitM4();

        //PlayM4(Payload);
        if ( IsPlayM4 && Resp.length() > RtpHeaderLength ) {
            QByteArray ssrcB = QByteArray::fromHex(ssrc.toUtf8());
            QByteArray RTSPHeader;
            int IdxTotal = Resp.count(ssrcB);
            int IdxCur = 1;
            for (int i = 0; i< Resp.length(); i++) {
                if (NeedMoreData > 0) {
                    if (Resp.startsWith(Magic)) {
                        qDebug() <<"$ at 0 ";
                        NeedMoreData = 0;
                        i = 0;
                    }else
                    {
                        if (Nexti > 0 && NextiHead == 0) {
                            i = Nexti;
                            NextiHead = 12 - Nexti;
                            Nexti = 0;
                        }

                        if (NeedMoreData == Resp.length()) {
                            qDebug() << Resp.length() << "=" << NeedMoreData;
                        }
                        if (NeedMoreData > Resp.length()) {
                            qDebug() << ">>>>>>>>>>Got" << Resp.length() << "need more" << NeedMoreData;
                            PayLTmpBuff.append(Resp.mid(i , NeedMoreData ));
                            NeedMoreData = (i + NeedMoreData) - Resp.length() - NextiHead;
                            NextiHead = 0;
                            return;
                        } else {
                            PayLTmpBuff.append(Resp.mid(i , NeedMoreData ));
                            if(RTPParser::RTPheader->padding)
                                PayLTmpBuff = PadingRemove(PayLTmpBuff);
                            PayLTmpBuff = RTPParser::processFrame(PayLTmpBuff);

                            RTPParser::RTPheader->Payload = PayLTmpBuff;
                            RTPParser::DebugRTP_header();
                            qDebug() <<"Got" << Resp.length() << "needed" << NeedMoreData;
                            if (Resp.length() > NeedMoreData) {
                                qDebug() << ">>>>>>>>>>more data to parse ";

                            }
                            Payload.append(PayLTmpBuff);
                            dbgtmp = i +=  NeedMoreData;
                            if (i < 0) return; ;
                            NeedMoreData = 0;
                            FullPayload = true;
                            PayLTmpBuff.clear();
                        }

                    }
                }
                if(i >= Resp.length())
                {
                    break;
                }
                qDebug() << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+";
                qDebug() << "idx  " << IdxCur <<"/" << IdxTotal;
                if (IdxCur == 3) {
                    //qDebug() <<"last IdxCur";
                }
                //if (IdxCur >= IdxTotal)
                IdxCur += 1;
                PayNum += 1;
                qDebug() << "<<<<<<<<XXXXXXXXX>>>>>>> Frame " << PayNum;
                if (Resp.at(i) != '$') {
                    qDebug() << "XXXXXXXXX not $ at i - 12 ";
                    //break;
                    //response start whith 4 bytes from previous payload but rtp len is ok !
                    i = Resp.indexOf(ssrcB,i ) - 12;
                    if (i < 0)
                        return;
                }
                if (Resp.length() > 16 )
                {
                    RTSPHeader.clear();
                    RTSPHeader = Resp.mid(i + 1, 15);
                    RTPParser::ParseRTSP(RTSPHeader);
                    //need to handle dynamic byte of RTPheader->CSRC here
                }
                if ((quint32)Resp.length() - i < RTPParser::RTSPheader->Length)
                {
                    if (Resp.length() - i < 16) {
                        Nexti = 16 - (Resp.length() - i);
                        NeedMoreData = RTPParser::RTSPheader->Length - Nexti;
                        break;
                    }else
                    {
                        Nexti = 0;
                    }
                    NeedMoreData = (i + 4 + RTPParser::RTSPheader->Length) - (Resp.length());
                    PayLTmpBuff = Resp.mid(i + 16, RTPParser::RTSPheader->Length);
                    qDebug() << ">>>>>>>>>>need " << NeedMoreData << "byte more.";
                    break;
                } else {
                    PayLTmpBuff = Resp.mid(i + 16, RTPParser::RTSPheader->Length - 12);
                    if(RTPParser::RTPheader->padding)
                        PayLTmpBuff = PadingRemove(PayLTmpBuff);
                    PayLTmpBuff = RTPParser::processFrame(PayLTmpBuff);

                    RTPParser::RTPheader->Payload = PayLTmpBuff;
                    RTPParser::DebugRTP_header();
                    Payload.append(PayLTmpBuff);
                    FullPayload = true;
                    i =  i + RTPParser::RTSPheader->Length + 3;
                    if (i < 0) return;
                }

            }//for
            if (!Payload.isEmpty() && FullPayload)
            {
                Fs.write(Payload);
                Fs.flush();
                PlayM4(Payload);
                FullPayload = false;
            }

        }//if
    }
    else {
        ParseResp(Resp, public_method);
        if (NeedAuth && FaillAuthCnt < 4) {

            RtspHeaders();
            SendReq("DESCRIBE");
            NeedAuth = false;
            LastCSeq = CSeq;
        }
    }//else

    EndRead = true;
    /*if (public_method == "OPTIONS") {

        SendReq("DESCRIBE");
    }*/
    if (NeedAuth == false && (public_method == "DESCRIBE" ||  public_method == "DESCRIBENOA") && GotSdp == true && LastCSeq == CSeq - 1) {
        LastCSeq = CSeq;
        //Sleep(5);
        SendReq("SETUP");
    }
    if (NeedAuth == false && public_method == "SETUP" && GotSdp == true && LastCSeq == CSeq - 1 && trackID[0] == "/trackID=1" ) {
        LastCSeq = CSeq;
        //Sleep(5);
        trackID[0] = "/trackID=1";
        SendReq("PLAY");
    }
    if (public_method == "SETUP" && Transport != "" && LastCSeq == CSeq - 1 && trackID[0] == "/trackID=3" ) {
        LastCSeq = CSeq;
        SendReq("PLAY");
    }
    if (public_method == "TEARDOWN") {
        Fs.close();
    }

    //PlayM4(Payload);
}

QByteArray MainWindow::PadingRemove(QByteArray Payload)
{
    int pos = Payload.length();
    int cnt = Payload.at(pos - 1);
    qDebug() <<"PadingRemove" << cnt;
    return Payload.remove(pos - cnt, cnt);
}

void MainWindow::InitM4()
{
    if (public_method == "PLAY" && IsPlayM4 == false) {
        IsPlayM4 = true;

        if (!PlayM4_GetPort(&g_nPort))
        {
            qDebug() << "PlayM4_GetPort error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }

        if (!PlayM4_SetTimerType(g_nPort,TIMER_2,0))
        {
            qDebug() << "PlayM4_SetTimerType error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }

        if (!PlayM4_SetStreamOpenMode(g_nPort,STREAME_REALTIME))
        {
            qDebug() << "PlayM4_SetStreamOpenMode error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }
        if (!PlayM4_OpenStreamAdvanced(g_nPort, PLAYM4_PROTOCOL_RTSP, pstSessionInfo, 6 * 1024 * 1024))
        {
            qDebug() << "PlayM4_OpenStreamAdvanced error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }
        if (!PlayM4_SetDecCBStream(g_nPort,1))
        {
            qDebug() << "PlayM4_SetDecCBStream error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }
        if (!PlayM4_SetDecCallBackExMend(g_nPort,DecCBFun,NULL,0,NULL))
        {
            qDebug() << "PlayM4_SetDecCallBackExMend error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }

        if (!PlayM4_SetDisplayBuf(g_nPort,6 * 1024 * 1024))
        {
            qDebug() << "PlayM4_SetDisplayBuf error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }


        QByteArray HIKHeaderB = QByteArray::fromHex(HIKHeader.toUtf8());
        //QByteArray HIKHeaderB = QByteArray::fromHex(MP4Header.toUtf8());
        Fs.write(HIKHeaderB);
        Fs.flush();
        /*if (!PlayM4_InputData(g_nPort,(BYTE*)HIKHeaderB.data(),HIK_HEAD_LEN))
        {
            qDebug() << "PlayM4_InputData HIKHeader Error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }*/
        //---------------------------------------
        // Get the window handle to display
        HWND hWnd = (HWND)centralWidget()->winId();
        if (!PlayM4_Play(g_nPort,hWnd))
        {
            qDebug() << "PlayM4_Play error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }
    }
}

void MainWindow::PlayM4(QByteArray Payload)
{
    if (IsRTSB && IsPlayM4 == true) {

        qDebug() << "PlayM4(Payload)" << Payload.length();
        if (Payload.length() <= 0)return;
        //qDebug() << Payload.toHex();
        BOOL bFlag  = FALSE;
        int  nError = 0;
        QByteArray Extract;
        const  char* pBuffer = NULL;
        /*for (int i = 0; i< Payload.length(); i++)
        {
            Extract = Payload.mid(i,READ_BUF_SIZE);
            pBuffer = Extract.data();*/

        //while (1)
        //{
        if (!PlayM4_InputData(g_nPort,(BYTE*)Payload.constData(),6 * 1024 * 1024))
        {
            //qDebug() << "PlayM4_InputData error...";
            //qDebug() << PlayM4_GetLastError(g_nPort);
            bFlag = false;
        }else
        {
            bFlag = true;
        }

        if (bFlag == FALSE)
        {
            nError = PlayM4_GetLastError(g_nPort);

            //If the buffer is full, input the data repeatedly
            if (nError == PLAYM4_BUF_OVER)
            {
                if (!PlayM4_ResetSourceBuffer(g_nPort))
                {
                    qDebug() << "PlayM4_ResetSourceBuffer error...";
                }
                /*Sleep(2);
                        continue;*/
            }
        }

        //If inputting the data successfully, then read the data from the file and input data to the player buffer
        /*break;
            }
            i +=READ_BUF_SIZE - 1;
        }*/
        //Sleep(10000);
        /*if (!PlayM4_InputData(g_nPort,(BYTE*)Payload.data(),Payload.length()))
        {
            qDebug() << "PlayM4_InputData...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }*/
        if (!PlayM4_WndResolutionChange(g_nPort))
        {
            qDebug() << "PlayM4_WndResolutionChange error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }
        if (!PlayM4_RefreshPlay(g_nPort))
        {
            //qDebug() << "PlayM4_RefreshPlay error...";
            //qDebug() << PlayM4_GetLastError(g_nPort);
        }
        if (!PlayM4_SetVideoWindow(g_nPort,0,(HWND)centralWidget()->winId()))
        {
            qDebug() << "PlayM4_SetVideoWindow error...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }

        //lType: 1  refers to get the PTZ information of displayed frame. It is stored in the pInfo with specified structure type, and plLen returns length information. First you should set pInfo = null to get the memory length (plLen) needed to allocate.
        /*int lType = 1;
        unsigned char pInfo = '\0';
        int plLen ;
        if (!PlayM4_GetStreamAdditionalInfo(g_nPort, lType, &pInfo, &plLen))
        {
            qDebug() << "PlayM4_GetStreamAdditionalInfo...";
            qDebug() << PlayM4_GetLastError(g_nPort);
        }*/


    }
}

void MainWindow::Play()
{

    /*QFile file(":/xml/ReqXml.xml");
    file.open(QIODevice::ReadOnly);
    QString SetXMLReq = file.readAll();
    file.close();*/
    QSettings settings;
    //QUrl Adresse("http://admin:pass@" + Settings->CamIp + ":" + Settings->CamPortHttp + "/SDK/play/100/004");
    QUrl Adresse("http://" + Settings->CamIp + ":" + Settings->CamPortHttp + "/codebase/version.xml?version=V4.0.1build191111");


    //manager->put((QNetworkRequest)Adresse,SetXMLReq.toUtf8());
    QIODevice * outgoingData = 0;
    //manager->put((QNetworkRequest)Adresse,outgoingData);
    //manager->get((QNetworkRequest)Adresse);
    manager->get((QNetworkRequest)Adresse);
    manager->get((QNetworkRequest)(QUrl)("http://" + Settings->CamIp + ":" + Settings->CamPortHttp + "/ISAPI/Security/token?format=json"));
}

void MainWindow::slotError(QNetworkReply::NetworkError code)
{
    qDebug() << code;
}

void MainWindow::slotSslErrors(const QList<QSslError> &errors)
{
    qDebug() << errors;
}

void MainWindow::FromDocEx()
{
    PlayM4_Stop(g_nPort);

    //Close the stream ,release the source buffer
    PlayM4_CloseStream(g_nPort);
    //Get the PlayCtrl library port No.
    PlayM4_FreePort(g_nPort);

    BOOL bFlag  = FALSE;
    int  nError = 0;
    FILE* fp    =  NULL;
    char* pBuffer = NULL;

    // Get the PlayCtrl library port No.
    PlayM4_GetPort(&g_nPort);
    //Open the file
    fp = fopen( "/tmp/tmp.mp4", ("rb") );
    if (fp == NULL)
    {
        printf("cannot open the file !\n");
        return;
    }
    pBuffer = new char[READ_BUF_SIZE];
    if (pBuffer == NULL)
    {
        return;
    }
    //Read the hik file head
    fread( pBuffer, 40, 1, fp );
    //Set stream mode

    if (!PlayM4_SetStreamOpenMode(g_nPort,STREAME_REALTIME))
    {
        qDebug() << "PlayM4_SetStreamOpenMode error...";
        qDebug() << PlayM4_GetLastError(g_nPort);
    }
    //Open stream mode
    PlayM4_OpenStream(g_nPort,(BYTE*)pBuffer,40,1024 * 1024);
    //Set decode callback function
    //int  PlayM4_SetDecCallBackExMend(int nPort, void (CALLBACK* DecCBFun)(int nPort, char* pBuf, int nSize, FRAME_INFO* pFrameInfo, void* nUser, int nReserved2), char* pDest, int nDestSize, void* nUser);
    PlayM4_SetDecCallBackExMend(g_nPort,DecCBFun,NULL,0,NULL);
    //---------------------------------------
    // Get the window handle to display
    HWND hWnd = (HWND)centralWidget()->winId();

    if (!PlayM4_Play(g_nPort,hWnd))
    {
        qDebug() << "PlayM4_Play error...";
        qDebug() << PlayM4_GetLastError(g_nPort);
    }

    while (!feof(fp))
    {
        fread( pBuffer, READ_BUF_SIZE, 1, fp );

        while (1)
        {
            if (!PlayM4_InputData(g_nPort,(BYTE*)pBuffer,200))
            {
                qDebug() << "PlayM4_InputData error...";
                qDebug() << PlayM4_GetLastError(g_nPort);
                bFlag = false;
            }else
            {
                bFlag = true;
            }

            if (bFlag == FALSE)
            {
                nError = PlayM4_GetLastError(g_nPort);

                //If the buffer is full, input the data repeatedly
                if (nError == PLAYM4_BUF_OVER)
                {
                    Sleep(2);
                    continue;
                }
            }

            //If inputting the data successfully, then read the data from the file and input data to the player buffer
            break;
        }
    }
    Sleep(10000);


    //---------------------------------------
    // Stop decode
    PlayM4_Stop(g_nPort);

    //Close the stream ,release the source buffer
    PlayM4_CloseStream(g_nPort);
    //Get the PlayCtrl library port No.
    PlayM4_FreePort(g_nPort);
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
    if (pBuffer != NULL)
    {
        delete [] pBuffer;
        pBuffer = NULL;
    }
    return;
}

void MainWindow::DecCBFun(int nPort, char* pBuf, int nSize, FRAME_INFO* pFrameInfo, void* nUser, int nReserved2)
{
    //Get the decoded audio data
    if (pFrameInfo->nType == T_AUDIO16)
    {
        printf("test:: get audio data !\n");
    }
    //Get the decoded video data
    else if ( pFrameInfo->nType == T_YV12 )
    {
        //printf("test:: get video data !\n");
    }
}

void MainWindow::SourceBufCallBack(int nPort, unsigned int nBufSize, unsigned int dwUser, void* pResvered)
{
    if (!PlayM4_ResetSourceBuffer(g_nPort))
    {
        qDebug() << "PlayM4_ResetSourceBuffer error...";
    }

}
void MainWindow::Sleep(int MSecs)
{
    QTime dieTime= QTime::currentTime().addMSecs(MSecs);
    while (QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

void MainWindow::on_actionPlay_triggered()
{
    FaillAuthCnt = 0;
    IsPlayM4 = false;
    GotSdp = false;
    NeedMoreData = false;
    IsRTSB = false;
    MaxRetry = 0;
    PayNum = 0;
    trackID[0] = "/trackID=1";
    if(!Fs.isOpen())
        Fs.open(QIODevice::OpenModeFlag::WriteOnly);
    /*QString url =
            QInputDialog::getText(this, tr("Open Url"), tr("Enter the URL you want to play"));
    if (url.isEmpty())
        return;*/
    //FromDocEx();
    //MainWindow::Connect();
    Play();
}
void MainWindow::WaitEndRead()
{

    /* QTime dieTime= QTime::currentTime().addMSecs(3000);
    while (!m_socket->waitForBytesWritten(10000)) {
        qDebug() << "waitForBytesWritten";
        if(QTime::currentTime() < dieTime)break;
    }
    dieTime= QTime::currentTime().addMSecs(10000);
    while (!m_socket->waitForReadyRead(3000)) {
        qDebug() << "waitForReadyRead";
        if(QTime::currentTime() < dieTime)break;
    }*/
}



void MainWindow::replyFinished(QNetworkReply *reply)
{

    if(reply->error())
    {
        qDebug() << "ERROR!";
        printf("finish : %s\n\r", reply->errorString().toUtf8().data());
        qDebug() << reply->readAll();
    }
    else
    {

        QString Header = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        qDebug() << Header;
        qDebug() << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString();;
        qDebug() << reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
        qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        qDebug() << reply->header(QNetworkRequest::UserAgentHeader).toString();

        QByteArray Response = reply->readAll();

        qDebug() << Response;
        QString ResponseChr = QString(Response);
        if (ResponseChr.contains("Token")) {
            QStringList Vals = ResponseChr.split(":");
            QString TmpToken = Vals.at(Vals.length() - 1);
            TmpToken = TmpToken.remove("\n");
            TmpToken = TmpToken.remove("\t");
            TmpToken = TmpToken.remove("}");
            TmpToken = TmpToken.remove("\"");
            Token = TmpToken;
            manager->head((QNetworkRequest)(QUrl)("http://" + Settings->CamIp + ":" + Settings->CamPortHttp + "/ISAPI/Security/advanced?format=json&security=1&iv=" + Token));
            manager->head((QNetworkRequest)(QUrl)("http://" + Settings->CamIp + ":" + Settings->CamPortHttp + "/SDK/play?modelErrorCode=true&token=" + Token));
            manager->get((QNetworkRequest)(QUrl)("http://" + Settings->CamIp + ":" + Settings->CamPortHttp + "/SDK/play/100/004?modelErrorCode=true&token=" + Token));
        }

    }

    reply->deleteLater();
}

void MainWindow::authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{

    qDebug()<< "authenticationRequired";
    if(reply->error())
    {
        qDebug()<< "ERROR!" << reply->errorString();
    }
    else
    {
        qDebug() << reply->header(QNetworkRequest::ContentTypeHeader).toString();
        qDebug() << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString();;
        qDebug() << reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
        qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

        QSettings settings;
        qDebug()<< "CamUser!" << Settings->CamUser;
        authenticator->setUser(Settings->CamUser);
        authenticator->setPassword(Settings->CamPass);
        manager->get((QNetworkRequest)(QUrl)("http://" + Settings->CamIp + ":" + Settings->CamPortHttp + "/ISAPI/Security/token?format=json"));


    }

    reply->deleteLater();
}


void MainWindow::RtspHeaders()
{
    if(trackID.length() == 0)trackID.append("/stream=0");

    ReqSetUrl="SETUP " + RtspUri + trackID[0] + " RTSP/1.0\r\nCSeq: " + QString::number(CSeq) + "\r\nTransport: " + Transport + ";\r\nUser-Agent: QtVsPlayer\r\n\r\n";

    ReqdestNoDigest="DESCRIBE " + RtspUri + " RTSP/1.0\r\nCSeq: " + QString::number(CSeq) + "\r\nUser-Agent: QtVsPlayer\r\n\r\n";

    Reqdest="DESCRIBE " + RtspUri + " RTSP/1.0\r\nCSeq: " + QString::number(CSeq) + "\r\nAccept: application/sdp\r\nAuthorization: Digest username=\"admin\", realm=" + realm + ", nonce=\"" + nonce + "\", uri=\"" + RtspUri + "\", response=\"" + response + "\";\r\nUser-Agent: QtVsPlayer\r\n\r\n";

    Reqsetu="SETUP " + RtspUri + trackID[0] + " RTSP/1.0\r\nCSeq: " + QString::number(CSeq) + "\r\nTransport: " + Transport + "\r\nAuthorization: Digest username=\"admin\", realm=" + realm + ", nonce=\"" + nonce + "\", uri=\"" + RtspUri + "\", response=\"" + response + "\";\r\nUser-Agent: QtVsPlayer\r\n\r\n";

    ReqOption="OPTIONS " + RtspUri + " RTSP/1.0\r\nCSeq: " + QString::number(CSeq) + "\r\nUser-Agent: QtVsPlayer\r\n\r\n";

    Reqplay="PLAY " + RtspUri + " RTSP/1.0\r\nCSeq: " + QString::number(CSeq) + "\r\nSession: " + SessionID + "\r\nRange: npt=0,000-\r\nAuthorization: Digest username=\"admin\", realm=" + realm + ", nonce=" + nonce + ", uri=\"" + RtspUri + "\", response=\"" + response + "\";\r\nUser-Agent: QtVsPlayer\r\n\r\n";

    ReqStop="TEARDOWN " + RtspUri + " RTSP/1.0\r\nCSeq: " + QString::number(CSeq) + "\r\nSession: " + SessionID + "\r\nAuthorization: Digest username=\"admin\", realm=" + realm + ", nonce=" + nonce + ", uri=\"" + RtspUri + "\", response=\"" + response + "\";\r\nUser-Agent: QtVsPlayer\r\n\r\n";
}


void MainWindow::ParseResp(QByteArray Resp, QString public_method)
{
    QString DataAsString = QString(Resp);
    QStringList DataAsStrList = DataAsString.split("\r\n");

    foreach ( QString line, DataAsStrList)
    {
        if (public_method == "DESCRIBE" && line.contains("Transport")) {
            QStringList TransportStrList = line.split(":");
            if(TransportStrList.length() > 1)
                Transport = TransportStrList[1].remove(" ");
            RtspHeaders();

        }else{
            qDebug() << line;
        }

        //line = line.replace(QString('\\'), QString(' '));
        if (line.contains("200 OK") && (public_method == "DESCRIBE" || public_method == "DESCRIBENOA")) {
            ParseSdpResp(Resp);
            break;
        }
        if (line.contains("CSeq")) {
            CSeq = line.right(1).toInt() + 1;
        }
        if (line.contains("realm")) {

            realm = FindRTSPVar("realm", line);
            //realm = "IP Camera(F2348)";
        }
        if (line.contains("nonce")) {

            nonce = FindRTSPVar("nonce", line);
        }
        if (line.contains("Session")) {

            SessionID = FindRTSPVar("Session", line);
        }
        if (line.contains("401 Unauthorized"))
        {
            Authanticate(public_method);
            NeedAuth = true;
            FaillAuthCnt += 1;
        }
        if (line.contains("ssrc")) {

            ssrc = FindRTSPVar("ssrc", line);
        }

    }

    //CSeq += 1;
    RtspHeaders();
}
void MainWindow::ParseSdpResp(QByteArray Resp)
{
    QString DataAsString = QString(Resp);
    QStringList DataAsStrList = DataAsString.split("\r\n");
    QByteArray Buf;
    trackID.clear();
    for (int i=0; i< 6; i++) {
        qDebug() << DataAsStrList.at(i);
        if (DataAsStrList.at(i).contains("CSeq")) {
            CSeq = DataAsStrList.at(i).right(1).toInt() + 1;
        }
    }

    for (int i=6; i< DataAsStrList.length(); i++) {
        qDebug() << DataAsStrList.at(i);
        Buf.append(DataAsStrList.at(i).toUtf8() + "\r\n");
        //SdpData += DataAsStrList.at(i).toUtf8().data();
        if (DataAsStrList.at(i).contains("MEDIAINFO")) {

            HIKHeader = FindRTSPVar("MEDIAINFO", DataAsStrList.at(i));
        }
        if (DataAsStrList.at(i).contains("sps")) {

            sps = FindRTSPVar("sps", DataAsStrList.at(i));
            qDebug() << "sps:" << sps.toUtf8().toHex(' ');
        }
        if (DataAsStrList.at(i).contains("pps")) {

            pps = FindRTSPVar("pps", DataAsStrList.at(i));
            qDebug() << "pps:" << pps.toUtf8().toHex(' ');
        }
        if (DataAsStrList.at(i).contains("trackID")) {

            trackID = FindRTSPVar(DataAsStrList.at(i));
        }
    }

    /*const std::size_t count = Buf.size();
    SdpData = new char[count];
    std::memcpy(SdpData,Buf.constData(),count);*/

    SdpData = (unsigned char*)Buf.data();
    pstSessionInfo = new PLAYM4_SESSION_INFO;
    pstSessionInfo->nSessionInfoLen = strlen((const char*)SdpData);
    pstSessionInfo->nSessionInfoType = PLAYM4_SESSION_INFO_SDP;
    pstSessionInfo->pSessionInfoData = SdpData;
    /*qDebug() << Buf.size();
    qDebug() << strlen((const char*)SdpData) ;*/
    GotSdp = true;

}

QStringList MainWindow::FindRTSPVar(QString Line)
{
    QStringList DataAsStrList = Line.split("=");
    if (DataAsStrList.length() == 3) {
        QString Id = DataAsStrList.at(2);
        if(!trackID.contains(Id))
            trackID.append("/trackID=" + Id);
        return trackID;
    }
    trackID.append("");
    return trackID;
}

QString MainWindow::FindRTSPVar(QString Str, QString Line)
{
    QString Result = "";
    QStringList DataAsStrList = Line.split("=");
    if (DataAsStrList.length() == 2) {
        QStringList DataSess = DataAsStrList[0].split("       ");
        if (DataSess.length() == 2)
        {
            QStringList DataSess2 = DataSess[1].split(";");
            return DataSess2[0].remove(" ");
        }
    }
    if (DataAsStrList.length() == 3 && Str == "MEDIAINFO") {
        return DataAsStrList[2].remove(";");
        //return DataAsStrList[2].left(40);
    }
    bool bFound = false;
    int i = 0;
    while ((!bFound) && (i<DataAsStrList.count())) {
        if (DataAsStrList.at(i).contains(QString(Str))) {
            if (DataAsStrList.length() > 1) {
                DataAsStrList[i + 1] = DataAsStrList[i + 1].replace(";",",");
                QStringList SubResult = DataAsStrList[i + 1].split(",");
                Result = SubResult[0];

            }
            bFound = true;
        } else {
            i++;
        }
    }
    return Result;
}

QByteArray MainWindow::MD5Compute(QByteArray Str)
{

    QByteArray BResult = QCryptographicHash::hash(Str, QCryptographicHash::Algorithm::Md5);

    return BResult;
}

void MainWindow::Authanticate(QString( public_method))
{

    Settings->FillVars(ui->comboBoxUris->currentIndex());
    QString StrHA1 = Settings->CamUser + ":" + realm + ":" + Settings->CamPass;
    QString StrHA2 = public_method + ":" + RtspUri;
    StrHA1 = StrHA1.remove("\"");
    StrHA2 = StrHA2.remove("\"");

    //Authorization:Digest username=%s,realm=%s,nonce=%s,uri=%s,response=%s \r\n;
    //"md5(md5(username:realm:password):nonce:md5(public_method:url))"

    QByteArray HA1 = MD5Compute(StrHA1.toUtf8().data());
    QByteArray HA2 = MD5Compute(StrHA2.toUtf8().data());
    QString digestData = HA1.toHex() + ":" + nonce + ":" + HA2.toHex();

    digestData = digestData.remove("\"");
    QByteArray BResult = MD5Compute(digestData.toUtf8().data());
    response = BResult.toHex();
    //qDebug() << "responseMD5:" << response;
}

void MainWindow::on_actionDESCRI_triggered()
{
    SendReq("DESCRIBE");
    return;

    qDebug() << "Send DESCRIBE :";
    public_method = "DESCRIBE";
    Authanticate("DESCRIBE");
    RtspHeaders();
    /*QStringList Lines = Reqdest.split("\r\n");
    foreach(QString Line,Lines)
    {
        m_socket->write(Line.toUtf8());
        m_socket->write("\r\n");
        qDebug() << Line;
    }

    m_socket->write("\r\n\r\n");*/
    m_socket->write(Reqdest.toUtf8());
    WaitEndRead();
}

void MainWindow::on_actionSETUP_URL_triggered()
{

    qDebug() << "Send SETUP to set URL :";
    public_method = "SETUP";
    qDebug() << ReqSetUrl.toUtf8();
    m_socket->write(ReqSetUrl.toUtf8());
    RtspHeaders();
    LastCSeq = CSeq;
}

void MainWindow::on_actionSETUP_triggered()
{
    /*trackID = ui->TxtTrackID->text();
    Transport = "";*/
    RtspHeaders();
    SendReq("SETUP");
    RtspHeaders();
    LastCSeq = CSeq;
    return;

    qDebug() << "Send SETUP : ";
    public_method = "SETUP";
    Authanticate("SETUP");
    RtspHeaders();
    qDebug() << Reqsetu.toUtf8();
    m_socket->write(Reqsetu.toUtf8());
    WaitEndRead();
}

void MainWindow::on_actionPLAY_triggered()
{
    SendReq("PLAY");
    return;

    qDebug() << "Send PLAY :";
    public_method = "PLAY";
    Authanticate("PLAY");
    RtspHeaders();
    qDebug() << Reqplay.toUtf8();
    m_socket->write(Reqplay.toUtf8());
    WaitEndRead();
}

void MainWindow::on_actionOPTION_triggered()
{
    SendReq("OPTIONS");
    return;
    qDebug() << "Send OPTIONS : ";
    public_method = "OPTIONS";
    qDebug() << ReqOption.toUtf8();
    m_socket->write(ReqOption.toUtf8());
}

void MainWindow::on_actiontest_triggered()
{
    QSettings settings;
    VideoV = new  HikNetSdk();
    VideoV->show();
    VideoV->LoginInfo(8001,Settings->CamIp,Settings->CamUser,Settings->CamPass);
    VideoV->Play();
    return;
    ui->comboBoxUris->clear();
    ui->comboBoxUris->addItems(Settings->FillCmbUris());
    return;
    FromDocEx();
    return;
    qDebug() << "need:";
    nonce = "df570ea6dcfb0efb5398eb7641c3f343";
    realm = "\"IP Camera(F2348)\"";
    Authanticate("DESCRIBE");
    qDebug() << "responseMD5B:" << response;

    if (!PlayM4_SetDecCBStream(g_nPort,1))
    {
        qDebug() << "PlayM4_SetDecCBStream error...";
        qDebug() << PlayM4_GetLastError(g_nPort);
    }
    if (!PlayM4_SetDecCallBackExMend(g_nPort,DecCBFun,NULL,0,NULL))
    {
        qDebug() << "PlayM4_SetDecCallBackExMend error...";
    }
    /*if (!PlayM4_SetSourceBufCallBack(g_nPort, nThreShold, SourceBufCallBack,0,NULL))
    {
        qDebug() << "PlayM4_SetSourceBufCallBack error...";
    }*/

    //---------------------------------------
    // Get the window handle to display
    HWND hWnd = (HWND)videoWidget->winId();
    if (!PlayM4_Play(g_nPort,hWnd))
    {
        qDebug() << "PlayM4_Play error...";
        qDebug() << PlayM4_GetLastError(g_nPort);
    }

}

void MainWindow::on_actionAuthor_triggered()
{

    RtspHeaders();
}

void MainWindow::SendReq(const QString &Req)
{
    qDebug() << "====>Send " + Req + " : ";
    public_method = Req;
    Authanticate(Req);
    RtspHeaders();

    //"OPTIONS" << "DESCRIBE" << "SETUP" << "PLAY" << "TEARDOWN" << "DESCRIBENOA" ;
    switch (ReqList.indexOf(Req)) {
    case 0:
        qDebug() << ReqOption.toUtf8();
        m_socket->write(ReqOption.toUtf8());
        break;
    case 1:
        qDebug() << Reqdest.toUtf8();
        m_socket->write(Reqdest.toUtf8());
        break;
    case 2:
        //Sleep(1000);
        qDebug() << Reqsetu.toUtf8();
        m_socket->write(Reqsetu.toUtf8());
        break;
    case 3:
        qDebug() << Reqplay.toUtf8();
        m_socket->write(Reqplay.toUtf8());
        break;
    case 4:
        qDebug() << ReqStop.toUtf8();
        m_socket->write(ReqStop.toUtf8());
        break;
    case 5:
        qDebug() << ReqdestNoDigest.toUtf8();
        m_socket->write(ReqdestNoDigest.toUtf8());
        break;
    }
    WaitEndRead();

    m_socket->waitForBytesWritten();
}

void MainWindow::on_actionTEARDOWN_triggered()
{
    IsPlayM4 = false;
    SendReq("TEARDOWN");
    return;

}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    //videoWidget->resize(centralWidget()->size());
}

void MainWindow::on_actionDESC_no_DIGEST_triggered()
{

    RtspHeaders();
    qDebug() << ReqdestNoDigest.toUtf8();
    m_socket->write(ReqdestNoDigest.toUtf8());
    RtspHeaders();
    CSeq += 1;
    LastCSeq = CSeq;
    return;

    qDebug() << "Send SETUP : ";
    public_method = "SETUP";
    Authanticate("SETUP");
    RtspHeaders();
    qDebug() << Reqsetu.toUtf8();
    m_socket->write(Reqsetu.toUtf8());
    WaitEndRead();
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
void __stdcall  RealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD  dwBufSize, void* dwUser)
{
    if (dwUser != NULL)
    {
        qDebug("Demmo lRealHandle[%d]: Get StreamData! Type[%d], BufSize[%d], pBuffer:%p\n", lRealHandle, dwDataType, dwBufSize, pBuffer);
    }
}

void MainWindow::on_actionSettings_triggered()
{

    Settings->show();
    Settings->FillCmbUris();
}

void MainWindow::InitSettings()
{

    QSettings settings;
    Settings = new SettingsForm();
    if (Settings->CamName == "CamNamehik12345") Settings->show();
    ui->comboBoxUris->clear();
    ui->comboBoxUris->addItems(Settings->FillCmbUris());
}

void MainWindow::on_comboBoxUris_currentIndexChanged(const QString &arg1)
{
    RtspUri = arg1 + ui->comboBoxChanel->currentText();

    if (Settings->CamName == "localhost") {
        RtspUri.replace(ui->comboBoxChanel->currentText(),"");
    }
}

void MainWindow::on_comboBoxChanel_currentIndexChanged(const QString &arg1)
{
    RtspUri =  ui->comboBoxUris->currentText() + arg1;
    if (Settings->CamName == "localhost") {
        RtspUri.replace(arg1,"");
    }
}

void MainWindow::on_comboBoxUris_currentIndexChanged(int index)
{
    Settings->FillVars(index);
}
