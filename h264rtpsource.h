#ifndef H264RTPSOURCE_H
#define H264RTPSOURCE_H
#include "HikRtsp.h"


class H264RTPSource
{
public:
    H264RTPSource();
    virtual ~H264RTPSource();


protected:
    virtual void processFrame(QByteArray *packet);

    int parseSpropParameterSets(char *spropParameterSets);
};

#endif // H264RTPSOURCE_H
