#ifndef RTPPARSER_H
#define RTPPARSER_H

#include <QString>
#include <QBitArray>
#include <QDebug>



/** struct RTSP_header : 3 Bytes.*/
typedef struct _RTSP_header
{
    /** RTSP Chanel : 1 Byte.*/
    quint8 Chanel;
    /** RTP header + payload Length : 2 Bytes.*/
    quint32 Length;
} RTSP_header;

/** struct RTP_header : 14 Bytes + payload.*/
typedef struct _RTP_header
{
    /** RTP header Version : 2 Bits; currently, it is 2*/
    quint8 Version;
    /** RTP header padding : 1 Bit.If its value is1, the message end will be padded with a or multiple 8-bit
array(s), but the padded array(s) is (are) not a part of payload.*/
    bool padding;
    /** RTP header Extension : 1 Bit.set to 0 and reserved for further extension requirement.*/
    bool Extension;
    /** RTP header Contrib srce id count : 4 Bits.CSRC counter, which indicates the number of CSRC identifiers*/
    quint8 CSIC;
    /** RTP header Marker : 1 Bit.when there is a lot of stream data, the data will be packaging into multiple
subpackets to be sent, for subpacket, the value of maker flag is 0, and for end packet, the value
of maker flag is 1.*/
    bool Marker;
    /** RTP header PayloadType : 7 Bits.payload type, which is obtained via DESCRIBE operation.*/
    quint8 PayloadType;
    /** RTP header SeqNum : 2 Bytes.serial No., which identifies the sent RTP message. The No. starts from 0 in HELIX server
and will plus 1 when sending RTP message for once. The No. of video and audio message is
counted separately.
When the lower-layer adopts UDP, or the network condition is bad, the serial No. can be used to
check the packet loss; when the network jitter occurred, the serial No. can be used to reorder
the data.*/
    quint32 SeqNum;
    /** RTP header TimeStamp : 4 Bytes.32-bit timestamp, which indicates the sampling time of the first 8-bit array in the RTP message.
The receiver can calculate the delay or delay jitter, and control according to the timestamp.*/
    quint64 TimeStamp;
    /** RTP header SyncSrcId : 4 Bytes.synchronization source identifier, which is randomly selected, and the identifier value of
two synchronization sources in same RTP session cannot be same.*/
    quint64 SyncSrcId;//ssrc from SDP
    /** RTP header The CSRC  : 4 Bytes * CSIC.contributing source identifier, up to 16 identifiers are supported, its value is between 0
and 15, and each CSRC identifier identifies all contributing sources in the payload of the RTP
message.
      contained in this packet*/
    QByteArray CSRC;
    /** RTP header Payload : RTSP length - 4 Bytes.The payload consists of binary data, which transmits the detailed information and useful content.*/
    QByteArray Payload;
} RTP_header;

class RTPParser
{
public:
    RTPParser();

static RTSP_header *RTSPheader;
static RTP_header *RTPheader;

static void ParseRTSP(QByteArray Data);
static void DebugRTSP_header();
static void DebugRTP_header();

static quint16 castToUint16(const QByteArray bytes);
static quint32 castToUint16(const QString bytes);

static quint32 castToUint32(const QByteArray bytes);
static quint32 castToUint32(const QString bytes);
static quint64 castToUint64(const QByteArray bytes);
static quint32 castToUint64(const QString bytes);

/** GET value of BIT*/
static QBitArray getBit(int TheByteref);
static bool getBit(int TheByteref, quint8 NumBit);

static QByteArray processFrame(QByteArray payload);
static QByteArray putStartNAL(QByteArray PLoad);


static QString DbgToWrite;

};

#endif // RTPPARSER_H
