/*---------------------------------------------------------------------------*/
/* Copyright(C)  2017  OMRON Corporation                                     */
/*                                                                           */
/* Licensed under the Apache License, Version 2.0 (the "License");           */
/* you may not use this file except in compliance with the License.          */
/* You may obtain a copy of the License at                                   */
/*                                                                           */
/*     http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                           */
/* Unless required by applicable law or agreed to in writing, software       */
/* distributed under the License is distributed on an "AS IS" BASIS,         */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  */
/* See the License for the specific language governing permissions and       */
/* limitations under the License.                                            */
/*---------------------------------------------------------------------------*/

#ifndef main_H__
#define main_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <malloc.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "uart.h"
#include "HVCApi.h"
#include "HVCDef.h"
#include "HVCExtraUartFunc.h"
#include "STBWrap.h"

#include "inc/stdThread.h"
#include "Post_curl.h"
#include "JSON.h"

#include "string.h"


//#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>

using namespace std;

#define POST_URL "http://10.43.0.48/tinyretail_web/dbdev.php/?iam=camera"

#define SIZE_WIDTH   320
#define SIZE_HEIGHT   240

// 1600x1200
#define EoS_LEFT 400
#define EoS_RIGHT 1200

#define LOGBUFFERSIZE   8192

#define UART_SETTING_TIMEOUT              1000            /* HVC setting command signal timeout period */
#define UART_EXECUTE_TIMEOUT              ((10+10+6+3+15+15+1+1+15+10)*1000)
                                                          /* HVC execute command signal timeout period */

#define SENSOR_ROLL_ANGLE_DEFAULT            0            /* Camera angle setting (0°) */

#define BODY_THRESHOLD_DEFAULT             500            /* Threshold for Human Body Detection */
#define FACE_THRESHOLD_DEFAULT             500            /* Threshold for Face Detection */
#define HAND_THRESHOLD_DEFAULT             500            /* Threshold for Hand Detection */
#define REC_THRESHOLD_DEFAULT              500            /* Threshold for Face Recognition */

#define BODY_SIZE_RANGE_MIN_DEFAULT         30            /* Human Body Detection minimum detection size */
#define BODY_SIZE_RANGE_MAX_DEFAULT       8192            /* Human Body Detection maximum detection size */
#define HAND_SIZE_RANGE_MIN_DEFAULT         40            /* Hand Detection minimum detection size */
#define HAND_SIZE_RANGE_MAX_DEFAULT       8192            /* Hand Detection maximum detection size */
#define FACE_SIZE_RANGE_MIN_DEFAULT         64            /* Face Detection minimum detection size */
#define FACE_SIZE_RANGE_MAX_DEFAULT       8192            /* Face Detection maximum detection size */

#define FACE_POSE_DEFAULT                    0            /* Face Detection facial pose (frontal face)*/
#define FACE_ANGLE_DEFAULT                   0            /* Face Detection roll angle (±15°)*/

#define STB_RETRYCOUNT_DEFAULT               2            /* Retry Count for STB */
#define STB_POSSTEADINESS_DEFAULT           30            /* Position Steadiness for STB */
#define STB_SIZESTEADINESS_DEFAULT          30            /* Size Steadiness for STB */
#define STB_PE_FRAME_DEFAULT                10            /* Complete Frame Count for property estimation in STB */
#define STB_PE_ANGLEUDMIN_DEFAULT          -15            /* Up/Down face angle minimum value for property estimation in STB */
#define STB_PE_ANGLEUDMAX_DEFAULT           20            /* Up/Down face angle maximum value for property estimation in STB */
#define STB_PE_ANGLELRMIN_DEFAULT          -20            /* Left/Right face angle minimum value for property estimation in STB */
#define STB_PE_ANGLELRMAX_DEFAULT           20            /* Left/Right face angle maximum value for property estimation in STB */
#define STB_PE_THRESHOLD_DEFAULT           300            /* Threshold for property estimation in STB */

#endif