
#include "CNN_Detector.h"

/**

 ffmpeg -y -i 1208060776-1-192.mp4 -filter_complex \
"[0:v]trim=start_frame=25:end_frame=100,setpts=PTS-STARTPTS[v0]; \
 [0:v]trim=start_frame=200:end_frame=300,setpts=PTS-STARTPTS[v1]; \
 [0:v]trim=start_frame=600:end_frame=800,setpts=PTS-STARTPTS[v2]; \
 [0:v]trim=start_frame=1200:end_frame=1300,setpts=PTS-STARTPTS[v300]; \
 [v0][v1][v2][v300]concat=n=4:v=1:a=0[v]" \
-map "[v]" output.mp4

*/