#include "rtpparser.h"
#include <QFile>

QFile FsDbg("/tmp/tmp.csv");
QString RTPParser::DbgToWrite;
RTSP_header *RTPParser::RTSPheader = new RTSP_header;
RTP_header *RTPParser::RTPheader = new RTP_header;
RTPParser::RTPParser()
{
    if (FsDbg.isOpen())
        FsDbg.close();
    FsDbg.open(QIODevice::OpenModeFlag::WriteOnly);

}

void RTPParser::ParseRTSP(QByteArray Data)
{
    RTPheader->Payload.clear();
    if(Data.length() >= 3 && Data.length() < 12)
    {
        qDebug() << "Header is 12 Bytes, but got :" << Data.toHex(' ');
        RTSPheader->Chanel = Data.at(0);
        RTSPheader->Length = castToUint32(Data.mid(1,2));
        DebugRTSP_header();
        return;
    }
    RTSPheader->Chanel = Data.at(0);
    RTSPheader->Length = castToUint32(Data.mid(1,2));
    DebugRTSP_header();

    QBitArray bits(8);
    bits = getBit(Data.at(3));
    RTPheader->Version = (quint8)(bits[7]*2 + bits[6]*1);
    RTPheader->padding = (bool)bits[5];
    RTPheader->Extension = (bool)bits[4];
    RTPheader->CSIC = (quint8)(bits[3]*8 + bits[2]*4 + bits[1]*2 + bits[0]*1);
    bits = getBit(Data.at(4));
    RTPheader->Marker = (bool)bits[7];
    RTPheader->PayloadType = (quint8)(bits[6]*64 + bits[5]*32 + bits[4]*16 + bits[3]*8 + bits[2]*4 + bits[1]*2 + bits[0]*1);

    RTPheader->SeqNum = castToUint16(Data.mid(5,2));
    RTPheader->TimeStamp = castToUint64(Data.mid(7,4));
    RTPheader->SyncSrcId = castToUint64(Data.mid(11,4));
    QString hexadecimal;
    hexadecimal = hexadecimal.setNum(RTPheader->SyncSrcId,16);
    QString datastr = Data.toHex();
    int SyncSrcIdcnt = datastr.count(hexadecimal);
    if (SyncSrcIdcnt > 1) {
        qDebug() << "SyncSrcId count"  << SyncSrcIdcnt;
        qDebug() << "SyncSrcId"  << hexadecimal;
    }
    /*RTPheader->CSRC.clear();
    if (RTPheader->CSIC > 0) {
        RTPheader->CSRC = Data.mid(15,RTPheader->CSIC);
        RTPheader->Payload = Data.mid(15 + RTPheader->CSIC + 1,Data.length() - (15 + RTPheader->CSIC));
    } else {
        RTPheader->Payload = Data.mid(15,RTSPheader->Length - 12);
    }

    DebugRTP_header();

    if (RTPParser::RTPheader->padding) {
        qDebug() <<"next packets are padding ";
    }
    if(RTPheader->Version == 2 && RTSPheader->Chanel == 0)
    {
        RTPheader->Payload = processFrame(RTPheader->Payload);
    }else{
        //RTPheader->Payload = processFrame(Data);
        return NULL;
    }
    return RTPheader->Payload;*/
    Data.clear();
}

void RTPParser::DebugRTSP_header()
{

    qDebug() << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+";
    qDebug() << "------------RTSPheader--------------------";

    qDebug() << "Chanel"  << RTSPheader->Chanel;
    qDebug() << "Length"  << RTSPheader->Length;
    qDebug() << "-----------------------------------------";
}

void RTPParser::DebugRTP_header()
{
    qDebug() << "///////////RTPheader------------------";
    qDebug() << "Version"  << RTPheader->Version;
    qDebug() << "padding"  << RTPheader->padding;
    qDebug() << "Extension"  << RTPheader->Extension;
    qDebug() << "CSIC"  << RTPheader->CSIC;
    qDebug() << "Marker"  << RTPheader->Marker;
    qDebug() << "PayloadType"  << RTPheader->PayloadType;
    qDebug() << "SeqNum"  << RTPheader->SeqNum;
    qDebug() << "TimeStamp"  << RTPheader->TimeStamp;
    QString hexadecimal;
    qDebug() << "SyncSrcId"  << RTPheader->SyncSrcId  <<  hexadecimal.setNum(RTPheader->SyncSrcId,16);
    qDebug() << "CSRC"  << RTPheader->CSRC.toHex(' ');
    qDebug() << "CSRC Len"  << RTPheader->CSRC.length();
    //qDebug() << "Payload"  << RTPheader->Payload;
    qDebug() << "Payload length"  << RTPheader->Payload.length();
    qDebug() << "-----------------------------------------";
}


quint16 RTPParser::castToUint16(const QByteArray bytes)
{
    return (quint16)bytes.toHex().toUInt(nullptr,16);
}
quint32 RTPParser::castToUint16(const QString bytes)
{
    return castToUint16( QByteArray::fromHex(bytes.toUtf8()));
}

quint32 RTPParser::castToUint32(const QByteArray bytes)
{
    return (quint32)bytes.toHex().toUInt(nullptr,16);
}
quint32 RTPParser::castToUint32(const QString bytes)
{
    return castToUint32( QByteArray::fromHex(bytes.toUtf8()));
}
quint64 RTPParser::castToUint64(const QByteArray bytes)
{
    return (quint64)bytes.toHex().toUInt(nullptr,16);
}
quint32 RTPParser::castToUint64(const QString bytes)
{
    return castToUint64( QByteArray::fromHex(bytes.toUtf8()));
}

/** GET value of BIT*/
QBitArray RTPParser::getBit(int TheByteref)
{
    QBitArray Result(8);
    for (int z=0; z< 8; z++) {
        Result.setBit(z,(bool)(TheByteref & (1 << z)))  ;
    }
    return Result;
}

bool RTPParser::getBit(int TheByteref, quint8 NumBit)
{
    QBitArray Result(8);
    for (int z=0; z< 8; z++) {
        Result.setBit(z,(bool)(TheByteref & (1 << z)))  ;
    }
    return Result[NumBit];
}

QByteArray RTPParser::processFrame(QByteArray payload)
{
    QByteArray payloadRaw = payload;
    /*uint8_t* buf = (uint8_t*)payload.data();
    int len = payload.length();

    uint8_t* buf_ptr = buf;
    uint8_t* headerStart = buf;*/
    bool isCompleteFrame = false;

    int64_t media_timestamp = RTPheader->TimeStamp;

    uint8_t nalUnitType = (payload[0] & 0x7E) >> 1;
    //qint16 nalUnitType = castToUint16(payload.mid(0,2));

    qDebug() << "NALUNIT TYPE " <<  nalUnitType;
    switch (nalUnitType) {
    case 48: {	// Aggregation Packet (AP)
        qDebug() << "Aggregation Packet (AP)";
        /*buf_ptr += 2; len -= 2;
        payload.remove(0,2);
        while (len > 3)
        {
            uint16_t nalUSize = (buf_ptr[0] << 8) | (buf_ptr[1]);
            if (nalUSize > len) {
                qDebug() << "Aggregation Packet process error, staplen: %d, len\n" << nalUSize << len;
                break;
            }

            buf_ptr += 2; len -= 2;
            payload.remove(0,2);
            nalUnitType = (buf_ptr[0] & 0x7E) >> 1;


            payload = putStartNAL(payload);

            buf_ptr += nalUSize; len -= nalUSize;

            if (fFrameHandlerFunc)
                 fFrameHandlerFunc(fFrameHandlerFuncData, fFrameType, media_timestamp, fFrameBuf, fFrameBufPos);
             resetFrameBuf();
        }*/
    } break;
    case 49: {	// Fragmentation Unit (FU)
        uint8_t fu_header = payload.at(2);
        uint8_t fu_head_se = (fu_header & 0xC0) >> 6;
        //uint8_t nal_type = fu_header & 0x3F;

        switch(fu_head_se){

        case 0x02:         //nal start
        {
            payload.remove(2,1);
            payload[0] = (fu_header << 1);
            payload[1] = 0x01;
            payload = putStartNAL(payload);
        }
            break;

        case 0x00:     //nal middle
        {
            payload.remove(0,3);
        }
            break;

        case 0x01:     //nal end
        {
            payload.remove(0,3);
        }
            break;

        default:
            printf("Unknown fu head\n");
            return NULL;
        }

    } break;
    case 50: {	//PACI Packets
        qDebug() << "PACI Packets";
    } break;
    case 32: { // video parameter set (VPS)
        payload = putStartNAL(payload);
    } break;
    case 33: { // sequence parameter set (SPS)
        payload = putStartNAL(payload);
    } break;
    case 34: { // picture parameter set (PPS)
        payload = putStartNAL(payload);
    } break;
    case 39: { // supplemental enhancement information (SEI)
        payload = putStartNAL(payload);
    } break;
    case 1: { // single NAL
        payload = putStartNAL(payload);
    } break;
    case 16: { // single NAL??????
        payload = putStartNAL(payload);
    } break;
    default: {	// This packet contains one complete NAL unit:
        /*putStartCode();
         copyToFrameBuffer(buf_ptr, len);*/
        //if (payload.length() > 3)
        //payload.remove(0,2);
        if (payload.length() > 3)
            //payload = putStartNAL(payload);
        isCompleteFrame = true;
    } break;
    }


    if (payload.length() > 3)
    {
        /*DbgToWrite = "<" + QString::number(nalUnitType) +  "> " + payload.toHex(' ') + "\n";
    FsDbg.write(DbgToWrite.toUtf8());
    FsDbg.flush();*/
    }

    if (isCompleteFrame) {
        /* if (fFrameHandlerFunc)
             fFrameHandlerFunc(fFrameHandlerFuncData, fFrameType, media_timestamp, fFrameBuf, fFrameBufPos);
         resetFrameBuf();*/
    }
    return payload;
}

QByteArray RTPParser::putStartNAL(QByteArray PLoad)
{
    //QByteArray NAL("\x00\x00\x00\x01",4);//standard NAL
    QByteArray NAL("\x00\x00\x01",3);//HIK NAL
    NAL.append(PLoad);
    return NAL;
}
