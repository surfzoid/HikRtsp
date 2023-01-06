Not a smart code, more like an proof of concept, just trying to make a basic RTSP/RTP client and use Hikvision Playctrl/ player SDK to display the strean with PlayM4_OpenStreamAdvanced(int nPort, int nProtocolType, PLAYM4_SESSION_INFO* pstSessionInfo, unsigned int nBufPoolSize).  

Status is, TCP RTSP Digest author is okay.  
Parse hevc/h265 h265+ payload work with "Video: hevc (Main), yuvj420p(pc), 1280x720 [SAR 1:1 DAR 16:9], 10 fps, 10 tbr, 1200k tbn, 10 tbc".  
