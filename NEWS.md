Changes between 0.0.4 and 0.0.3:
--------------------------------

Changes between 0.0.3 and 0.0.1:
--------------------------------
- ffmpeg: switch to tag n2.0
- ffmpeg: remove rarely used decoders, parsers, demuxers
- avformat/hls: fix many bugs
- avformat/http: support reading compressed data
- avformat/mov: optimize short seek
- player: fix AudioTrack latency
- player: refactor play/pause/step/buffering logic
- player: fix A/V sync
- yuv2rgb: treat YUVJ420P as YUV420P
- yuv2rgb: support zero copy of YUV420P frame output to YV12 surface
- ijksdl: fix SDL_GetTickHR() returns wrong time 
