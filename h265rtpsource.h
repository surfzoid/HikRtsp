#ifndef H265RTPSOURCE_H
#define H265RTPSOURCE_H
#include "HikRtsp.h"


#include "h264rtpsource.h"

class H265RTPSource : public H264RTPSource
{
public:
    H265RTPSource();
    virtual ~H265RTPSource();
protected:
    virtual void processFrame(QByteArray* packet);
};

#endif // H265RTPSOURCE_H
