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


/*
g++ -o TinyRetail_HVCP2 TinyRetail_HVCP2_main.cpp HVCApi.c STBWrap.c uart.c Post_curl.cpp JSON.cpp libHVCP2.a -lcurl -std=c++11 -I/usr/local/include/opencv2 -I/usr/local/include/opencv -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -fsigned-char
*/
#include "TinyRetail_HVCP2_main.h"
#define RetryCount 10

/*----------------------------------------------------------------------------*/
/* UART send signal                                                           */
/* param    : int   inDataSize  send signal data                              */
/*          : UINT8 *inData     data length                                   */
/* return   : int               send signal complete data number              */
/*----------------------------------------------------------------------------*/
int UART_SendData(int inDataSize, UINT8 *inData)
{
    /* Send Data */
    int ret = com_send(inData, inDataSize);
    return ret;
}

/*----------------------------------------------------------------------------*/
/* UART receive signal                                                        */
/* param    : int   inTimeOutTime   timeout time (ms)                         */
/*          : int   *inDataSize     receive signal data size                  */
/*          : UINT8 *outResult      receive signal data                       */
/* return   : int                   receive signal complete data number       */
/*----------------------------------------------------------------------------*/
int UART_ReceiveData(int inTimeOutTime, int inDataSize, UINT8 *outResult)
{
    /* Receive Data */
    int ret = com_recv(inTimeOutTime, outResult, inDataSize);
    return ret;
}

/* Print Log Message */
static void PrintLog(char *pStr)
{
    puts(pStr);
}


int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

/* HVC Execute Processing  */
int main(int argc, char *argv[])
{
    //変数名は変える
    //-------------------------------
    double timeResidence = 0;       //総滞在時間
    int m_countTimeResidence = 0;
	bool startFlgTR = false;
	time_t startTimeTR;
	
    int m_countStbHuman = 0;
    int m_prevStbBody = 0;          //前回の安定化した人数
    
    
    
    //-------------------------------
    
    //-------------------------------
    int m_countAvg = 0;
    int m_countHuman = 0;             //通った人カウント
    int m_prevBody = 0;               //前回の人数
    
    int m_countAvg2 = 0;    //TODO: 変数名は変える
    int m_countInterestHuman = 0;     //ブースに興味を持った人カウント
    //-------------------------------
    
    //-------------------------------
    ofstream outputfile("/home/pi/TinyRetail-hvcp2/log/log.txt" ,ios::app);
    time_t timer;
    struct tm *t_st;

    time_t timeElapsed;
    double elapsedTimeSecond;
    timeElapsed = time(NULL);
    //-------------------------------
    
	int count = 0;
	
	CPost_curl *p_postCurl, *p_postCurlCameraCount;
	CJSON *p_listJSON;
	
	p_postCurl = new CPost_curl();
	p_postCurl->Begin(POST_URL_CAMERA);
	
    p_postCurlCameraCount = new CPost_curl();
	p_postCurlCameraCount->Begin(POST_URL_CAMERA_COUNT);

	p_listJSON = new CJSON();

#if 0
	cv::Mat	image1(SIZE_HEIGHT, SIZE_WIDTH, CV_8UC1);
	cv::namedWindow("Image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
#endif

    INT32 ret = 0;  /* Return code */

    INT32 inRate;
    int listBaudRate[] = {
                              9600,
                             38400,
                            115200,
                            230400,
                            460800,
                            921600
                         };

    UINT8 status;
    HVC_VERSION version;
    HVC_RESULT *pHVCResult = NULL;

    int nSTBFaceCount;
    STB_FACE *pSTBFaceResult;
    int nSTBBodyCount;
    STB_BODY *pSTBBodyResult;

    INT32 agleNo;
    HVC_THRESHOLD threshold;
    HVC_SIZERANGE sizeRange;
    INT32 pose;
    INT32 angle;
    INT32 timeOutTime;
    INT32 execFlag;
    INT32 imageNo;

    const char *pExStr[] = {"?", "Neutral", "Happiness", "Surprise", "Anger", "Sadness"};

    int i;
    int ch = 0;
    int revision;
    char *pStr;                     /* String Buffer for logging output */
    int nIndex;

    S_STAT serialStat;              /* Serial port set value*/

    serialStat.com_num = 0;
    serialStat.BaudRate = 0;        /* Default Baudrate = 9600 */
    
    if ( com_init(&serialStat) == 0 ) {
        printf("Failed to open COM port.\n");
        return (-1);
    }
    
    serialStat.BaudRate = 921600;
    for ( inRate = 0; inRate<sizeof(listBaudRate); inRate++ ) {
        if ( listBaudRate[inRate] == (int)serialStat.BaudRate ) {
            break;
        }
    }
    
    if ( inRate >= sizeof(listBaudRate) ) {
        printf("Failed to set baudrate.\n");
        return (-1);
    }

    /* Change Baudrate */
    ret = HVC_SetBaudRate(UART_SETTING_TIMEOUT, inRate, &status);
    if ( (ret != 0) || (status != 0) ) {
        printf("HVCApi(HVC_SetBaudRate) Error.\n");
        return (-1);
    }

    if ( com_init(&serialStat) == 0 ) {
        printf("Failed to open COM port.\n");
        return (-1);
    }

    /*****************************/
    /* Logging Buffer allocation */
    /*****************************/
    pStr = (char *)malloc(LOGBUFFERSIZE);
    if ( pStr == NULL ) {
        printf("Failed to allocate Logging Buffer.\n");
        return (-1);
    }
    memset(pStr, 0, LOGBUFFERSIZE);
    printf(" HVC-P2 Start\n");
    do {
        /*********************************/
        /* Result Structure Allocation   */
        /*********************************/
        pHVCResult = (HVC_RESULT *)malloc(sizeof(HVC_RESULT));
        if ( pHVCResult == NULL ) { /* Error processing */
            //sprintf(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nMemory Allocation Error : %08x\n", sizeof(HVC_RESULT));
			sprintf(&pStr[strlen(pStr)], "\nMemory Allocation Error : %08x\n", sizeof(HVC_RESULT));
			break;
        }

        /*********************************/
        /* STB Initialize                */
        /*********************************/
        ret = STB_Init(STB_FUNC_BD | STB_FUNC_DT | STB_FUNC_PT | STB_FUNC_AG | STB_FUNC_GN);
        if ( ret != 0 ) {
            //sprintf(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nSTB_Init Error : %d\n", ret);
            sprintf(&pStr[strlen(pStr)], "\nSTB_Init Error : %d\n", ret);
            break;
        }

        /*********************************/
        /* Get Model and Version         */
        /*********************************/
        ret = HVC_GetVersion(UART_SETTING_TIMEOUT, &version, &status);
        if ( ret != 0 ) {
            //sprintf(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVCApi(HVC_GetVersion) Error : %d\n", ret);
			sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_GetVersion) Error : %d\n", ret);
			break;
        }
        if ( status != 0 ) {
            //sprintf(&pStr[strlen(pStr)], LOGBUFFERSIZE-strlen(pStr), "\nHVC_GetVersion Response Error : 0x%02X\n", status);
			sprintf(&pStr[strlen(pStr)], "\nHVC_GetVersion Response Error : 0x%02X\n", status);
            break;
        }
        sprintf(&pStr[strlen(pStr)], "\nHVC_GetVersion : ");
        for(i = 0; i < 12; i++){
            sprintf(&pStr[strlen(pStr)], "%c", version.string[i] );
        }
        revision = version.revision[0] + (version.revision[1]<<8) + (version.revision[2]<<16) + (version.revision[3]<<24);
        sprintf(&pStr[strlen(pStr)], "%d.%d.%d.%d", version.major, version.minor, version.relese, revision);

        /*********************************/
        /* Set Camera Angle              */
        /*********************************/
        agleNo = SENSOR_ROLL_ANGLE_DEFAULT;
        ret = HVC_SetCameraAngle(UART_SETTING_TIMEOUT, agleNo, &status);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_SetCameraAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVC_SetCameraAngle Response Error : 0x%02X\n", status);
            break;
        }
        agleNo = 0xff;
        ret = HVC_GetCameraAngle(UART_SETTING_TIMEOUT, &agleNo, &status);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_GetCameraAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVC_GetCameraAngle Response Error : 0x%02X\n", status);
            break;
        }
        sprintf(&pStr[strlen(pStr)], "\nHVC_GetCameraAngle : 0x%02x", agleNo);
        /*********************************/
        /* Set Threshold Values          */
        /*********************************/
        threshold.bdThreshold = BODY_THRESHOLD_DEFAULT;
        threshold.hdThreshold = HAND_THRESHOLD_DEFAULT;
        threshold.dtThreshold = FACE_THRESHOLD_DEFAULT;
        threshold.rsThreshold = REC_THRESHOLD_DEFAULT;
        ret = HVC_SetThreshold(UART_SETTING_TIMEOUT, &threshold, &status);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_SetThreshold) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVC_SetThreshold Response Error : 0x%02X\n", status);
            break;
        }
        threshold.bdThreshold = 0;
        threshold.hdThreshold = 0;
        threshold.dtThreshold = 0;
        threshold.rsThreshold = 0;
        ret = HVC_GetThreshold(UART_SETTING_TIMEOUT, &threshold, &status);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_GetThreshold) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVC_GetThreshold Response Error : 0x%02X\n", status);
            break;
        }
        sprintf(&pStr[strlen(pStr)], "\nHVC_GetThreshold : Body=%4d Hand=%4d Face=%4d Recognition=%4d",
                 threshold.bdThreshold, threshold.hdThreshold, threshold.dtThreshold, threshold.rsThreshold);
        /*********************************/
        /* Set Detection Size            */
        /*********************************/
        sizeRange.bdMinSize = BODY_SIZE_RANGE_MIN_DEFAULT;
        sizeRange.bdMaxSize = BODY_SIZE_RANGE_MAX_DEFAULT;
        sizeRange.hdMinSize = HAND_SIZE_RANGE_MIN_DEFAULT;
        sizeRange.hdMaxSize = HAND_SIZE_RANGE_MAX_DEFAULT;
        sizeRange.dtMinSize = FACE_SIZE_RANGE_MIN_DEFAULT;
        sizeRange.dtMaxSize = FACE_SIZE_RANGE_MAX_DEFAULT;
        ret = HVC_SetSizeRange(UART_SETTING_TIMEOUT, &sizeRange, &status);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_SetSizeRange) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVC_SetSizeRange Response Error : 0x%02X\n", status);
            break;
        }
        sizeRange.bdMinSize = 0;
        sizeRange.bdMaxSize = 0;
        sizeRange.hdMinSize = 0;
        sizeRange.hdMaxSize = 0;
        sizeRange.dtMinSize = 0;
        sizeRange.dtMaxSize = 0;
        ret = HVC_GetSizeRange(UART_SETTING_TIMEOUT, &sizeRange, &status);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_GetSizeRange) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVC_GetSizeRange Response Error : 0x%02X\n", status);
            break;
        }
        sprintf(&pStr[strlen(pStr)], "\nHVC_GetSizeRange : Body=(%4d,%4d) Hand=(%4d,%4d) Face=(%4d,%4d)",
                                                            sizeRange.bdMinSize, sizeRange.bdMaxSize,
                                                            sizeRange.hdMinSize, sizeRange.hdMaxSize,
                                                            sizeRange.dtMinSize, sizeRange.dtMaxSize);
        /*********************************/
        /* Set Face Angle                */
        /*********************************/
        pose = FACE_POSE_DEFAULT;
        angle = FACE_ANGLE_DEFAULT;
        ret = HVC_SetFaceDetectionAngle(UART_SETTING_TIMEOUT, pose, angle, &status);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_SetFaceDetectionAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVC_SetFaceDetectionAngle Response Error : 0x%02X\n", status);
            break;
        }
        pose = 0xff;
        angle = 0xff;
        ret = HVC_GetFaceDetectionAngle(UART_SETTING_TIMEOUT, &pose, &angle, &status);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_GetFaceDetectionAngle) Error : %d\n", ret);
            break;
        }
        if ( status != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVC_GetFaceDetectionAngle Response Error : 0x%02X\n", status);
            break;
        }
        sprintf(&pStr[strlen(pStr)], "\nHVC_GetFaceDetectionAngle : Pose = 0x%02x Angle = 0x%02x", pose, angle);

        /*********************************/
        /* Set STB Parameters            */
        /*********************************/
        ret = STB_SetTrParam(STB_RETRYCOUNT_DEFAULT, STB_POSSTEADINESS_DEFAULT, STB_SIZESTEADINESS_DEFAULT);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(STB_SetTrParam) Error : %d\n", ret);
            break;
        }
        ret = STB_SetPeParam(STB_PE_THRESHOLD_DEFAULT, STB_PE_ANGLEUDMIN_DEFAULT, STB_PE_ANGLEUDMAX_DEFAULT, STB_PE_ANGLELRMIN_DEFAULT, STB_PE_ANGLELRMAX_DEFAULT, STB_PE_FRAME_DEFAULT);
        if ( ret != 0 ) {
            sprintf(&pStr[strlen(pStr)], "\nHVCApi(STB_SetPeParam) Error : %d\n", ret);
            break;
        }

        do {
            sprintf(&pStr[strlen(pStr)], "\nPress Space Key to end: ");

            /******************/
            /* Log Output     */
            /******************/
            PrintLog(pStr);

            memset(pStr, 0, LOGBUFFERSIZE);

            /*********************************/
            /* Execute Detection             */
            /*********************************/
            timeOutTime = UART_EXECUTE_TIMEOUT;
            execFlag = HVC_ACTIV_BODY_DETECTION | HVC_ACTIV_HAND_DETECTION | HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_DIRECTION |
                     HVC_ACTIV_AGE_ESTIMATION | HVC_ACTIV_GENDER_ESTIMATION | HVC_ACTIV_GAZE_ESTIMATION | HVC_ACTIV_BLINK_ESTIMATION |
                     HVC_ACTIV_EXPRESSION_ESTIMATION;
            imageNo = HVC_EXECUTE_IMAGE_QVGA; /* HVC_EXECUTE_IMAGE_NONE; */
            ret = HVC_ExecuteEx(timeOutTime, execFlag, imageNo, pHVCResult, &status);
            if ( ret != 0 ) {
                sprintf(&pStr[strlen(pStr)], "\nHVCApi(HVC_ExecuteEx) Error : %d\n", ret);
                continue;
                //break;
            }
            if ( status != 0 ) {
                sprintf(&pStr[strlen(pStr)], "\nHVC_ExecuteEx Response Error : 0x%02X\n", status);
                continue;
                //break;
            }
            
            if ( imageNo == HVC_EXECUTE_IMAGE_QVGA ) {
                //SaveBitmapFile(pHVCResult->image.width, pHVCResult->image.height, pHVCResult->image.image, "SampleImage.bmp");
				//
#if 0
				int countPic = 0;
				for (int y = 0; y < SIZE_HEIGHT; y++) {
					for (int x = 0; x < SIZE_WIDTH; x++) {
						image1.at<uchar>(y, x) = (uchar)pHVCResult->image.image[x + countPic];
						//image1.at<uchar>(y, x) = (uchar)255;
						//std::cout << x << ":" << (int)pHVCResult->image.image[x + countPic] << endl;
					}
					countPic += 320;
				}

				cv::imshow("Image", image1);
				cv::waitKey(1);
				//
#endif
            }
            
            if ( STB_Exec(pHVCResult->executedFunc, pHVCResult, &nSTBFaceCount, &pSTBFaceResult, &nSTBBodyCount, &pSTBBodyResult) == 0 ) {
                for ( i = 0; i < nSTBBodyCount; i++ )
                {
                    if ( pHVCResult->bdResult.num <= i ) break;

                    nIndex = pSTBBodyResult[i].nDetectID;
                    pHVCResult->bdResult.bdResult[nIndex].posX = (short)pSTBBodyResult[i].center.x;
                    pHVCResult->bdResult.bdResult[nIndex].posY = (short)pSTBBodyResult[i].center.y;
                    pHVCResult->bdResult.bdResult[nIndex].size = pSTBBodyResult[i].nSize;
                }
                for ( i = 0; i < nSTBFaceCount; i++ )
                {
                    if ( pHVCResult->fdResult.num <= i ) break;

                    nIndex = pSTBFaceResult[i].nDetectID;
                    pHVCResult->fdResult.fcResult[nIndex].dtResult.posX = (short)pSTBFaceResult[i].center.x;
                    pHVCResult->fdResult.fcResult[nIndex].dtResult.posY = (short)pSTBFaceResult[i].center.y;
                    pHVCResult->fdResult.fcResult[nIndex].dtResult.size = pSTBFaceResult[i].nSize;

                    if(pHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION){
                        pHVCResult->fdResult.fcResult[nIndex].ageResult.confidence += 10000; // During
                        if ( pSTBFaceResult[i].age.status >= STB_STATUS_COMPLETE ) {
                            pHVCResult->fdResult.fcResult[nIndex].ageResult.age = pSTBFaceResult[i].age.value;
                            pHVCResult->fdResult.fcResult[nIndex].ageResult.confidence += 10000; // Complete
                        }
                    }
                    if(pHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION){
                        pHVCResult->fdResult.fcResult[nIndex].genderResult.confidence += 10000; // During
                        if ( pSTBFaceResult[i].gender.status >= STB_STATUS_COMPLETE ) {
                            pHVCResult->fdResult.fcResult[nIndex].genderResult.gender = pSTBFaceResult[i].gender.value;
                            pHVCResult->fdResult.fcResult[nIndex].genderResult.confidence += 10000; // Complete
                        }
                    }
                }
            }
            

			/* Body Detection result string */
            if(pHVCResult->executedFunc & HVC_ACTIV_BODY_DETECTION){
                sprintf(&pStr[strlen(pStr)], "\n Body result count:%d", pHVCResult->bdResult.num);
                for(i = 0; i < pHVCResult->bdResult.num; i++){
                    sprintf(&pStr[strlen(pStr)], "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                pHVCResult->bdResult.bdResult[i].posX, pHVCResult->bdResult.bdResult[i].posY,
                                pHVCResult->bdResult.bdResult[i].size, pHVCResult->bdResult.bdResult[i].confidence);
                }
            }

            /* 売り場に来た人数 */
            if(pHVCResult->executedFunc & HVC_ACTIV_BODY_DETECTION)
            {
                if(pHVCResult->bdResult.num > m_prevBody)
                {
                    m_prevBody = pHVCResult->bdResult.num;
                }
                else if(pHVCResult->bdResult.num < m_prevBody)
                {
                    m_countAvg++;
                }
                
                if(m_countAvg >= RetryCount)
                {
                    int tmpCount = m_prevBody - pHVCResult->bdResult.num;// 前回から減った人数
                    for (int i = 0; i < tmpCount; i++)
                    {
                        m_countHuman++;
                        printf("売り場に来た人 %d\n", m_countHuman);
                    }
                    m_prevBody = pHVCResult->bdResult.num;
                    m_countAvg= 0;

                    p_listJSON->init();
                    p_listJSON->push("camera_count", "1");
                    p_listJSON->push("count", to_string(tmpCount));
                    string tmp = p_listJSON->pop();
                
                    if (!(tmp.empty()))
                        p_postCurlCameraCount->send_post(tmp.c_str());

                    }
            }
            
            if(pHVCResult->executedFunc & HVC_ACTIV_BODY_DETECTION)
            {
                if(pHVCResult->bdResult.num > 0)
                {
                    m_countTimeResidence++;
                }
                else if(pHVCResult->bdResult.num == 0)
                {
                    m_countTimeResidence = 0;
                }
                
                if(m_countTimeResidence >= RetryCount && startFlgTR == false)
                {
                    startTimeTR = time(NULL);
                    startFlgTR = true;
                }
                
                if(pHVCResult->bdResult.num == 0 && startFlgTR == true)
                {
                    m_countAvg2++;
                    
                    if(m_countAvg2 >= RetryCount)
                    {
                        printf("経過時間：%.1f秒\n", difftime(time(NULL), startTimeTR));
                        timeResidence = difftime(time(NULL), startTimeTR);
                        startFlgTR = false;
                        m_countTimeResidence = 0;
                        m_countAvg2 = 0;
                    }
                }
            }
            
		    /* Hand Detection result string */
            if(pHVCResult->executedFunc & HVC_ACTIV_HAND_DETECTION){
                sprintf(&pStr[strlen(pStr)], "\n Hand result count:%d", pHVCResult->hdResult.num);
                for(i = 0; i < pHVCResult->hdResult.num; i++){
                    sprintf(&pStr[strlen(pStr)], "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                pHVCResult->hdResult.hdResult[i].posX, pHVCResult->hdResult.hdResult[i].posY,
                                pHVCResult->hdResult.hdResult[i].size, pHVCResult->hdResult.hdResult[i].confidence);
                }
            }
			/* Face Detection result string */
            if(pHVCResult->executedFunc &
                    (HVC_ACTIV_FACE_DETECTION | HVC_ACTIV_FACE_DIRECTION |
                     HVC_ACTIV_AGE_ESTIMATION | HVC_ACTIV_GENDER_ESTIMATION |
                     HVC_ACTIV_GAZE_ESTIMATION | HVC_ACTIV_BLINK_ESTIMATION |
                     HVC_ACTIV_EXPRESSION_ESTIMATION | HVC_ACTIV_FACE_RECOGNITION)){
                sprintf(&pStr[strlen(pStr)], "\n Face result count:%d", pHVCResult->fdResult.num);
                
                for(i = 0; i < pHVCResult->fdResult.num; i++){
					//if ( pHVCResult->fdResult.fcResult[i].dtResult.posX < EoS_LEFT || pHVCResult->fdResult.fcResult[i].dtResult.posX > EoS_RIGHT)
						//continue;
						
                    if(pHVCResult->executedFunc & HVC_ACTIV_FACE_DETECTION){
                        /* Detection */
                        sprintf(&pStr[strlen(pStr)], "\n      Index:%d \t\tX:%d Y:%d Size:%d Confidence:%d", i,
                                    pHVCResult->fdResult.fcResult[i].dtResult.posX, pHVCResult->fdResult.fcResult[i].dtResult.posY,
                                    pHVCResult->fdResult.fcResult[i].dtResult.size, pHVCResult->fdResult.fcResult[i].dtResult.confidence);
                    }
                    if(pHVCResult->executedFunc & HVC_ACTIV_FACE_DIRECTION){
						/* Face Direction */
                        sprintf(&pStr[strlen(pStr)], "\n      Face Direction\tLR:%d UD:%d Roll:%d Confidence:%d",
                                    pHVCResult->fdResult.fcResult[i].dirResult.yaw, pHVCResult->fdResult.fcResult[i].dirResult.pitch,
                                    pHVCResult->fdResult.fcResult[i].dirResult.roll, pHVCResult->fdResult.fcResult[i].dirResult.confidence);
                    }
                    if(pHVCResult->executedFunc & HVC_ACTIV_AGE_ESTIMATION){
                        /* Age */
                        if(-128 == pHVCResult->fdResult.fcResult[i].ageResult.age){
                            sprintf(&pStr[strlen(pStr)], "\n      Age\t\tEstimation not possible");
                        } else {
                            if ( pHVCResult->fdResult.fcResult[i].ageResult.confidence >= 20000 ) { // Complete
                                sprintf(&pStr[strlen(pStr)], "\n      Age\t\tAge:%d Confidence:%d (*)",
                                            pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence - 20000);
                            } else
                            if ( pHVCResult->fdResult.fcResult[i].ageResult.confidence >= 10000 ) { // During
                                sprintf(&pStr[strlen(pStr)], "\n      Age\t\tAge:%d Confidence:%d (-)",
                                            pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence - 10000);
                            } else {
                                sprintf(&pStr[strlen(pStr)], "\n      Age\t\tAge:%d Confidence:%d (x)",
                                            pHVCResult->fdResult.fcResult[i].ageResult.age, pHVCResult->fdResult.fcResult[i].ageResult.confidence);
                            }
                        }
                    }
                    if(pHVCResult->executedFunc & HVC_ACTIV_GENDER_ESTIMATION){
                        /* Gender */
                        if(-128 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                            sprintf(&pStr[strlen(pStr)], "\n      Gender\t\tEstimation not possible");
                        }
                        else{
                            if ( pHVCResult->fdResult.fcResult[i].genderResult.confidence >= 20000 ) { // Complete
                                if(1 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                    sprintf(&pStr[strlen(pStr)], "\n      Gender\t\tGender:%s Confidence:%d (*)",
                                                "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 20000);
                                }
                                else{
                                    sprintf(&pStr[strlen(pStr)], "\n      Gender\t\tGender:%s Confidence:%d (*)",
                                                "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 20000);
                                }
                            } else
                            if ( pHVCResult->fdResult.fcResult[i].genderResult.confidence >= 10000 ) { // During
                                if(1 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                    sprintf(&pStr[strlen(pStr)], "\n      Gender\t\tGender:%s Confidence:%d (-)",
                                                "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 10000);
                                }
                                else{
                                    sprintf(&pStr[strlen(pStr)], "\n      Gender\t\tGender:%s Confidence:%d (-)",
                                                "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence - 10000);
                                }
                            } else {
                                if(1 == pHVCResult->fdResult.fcResult[i].genderResult.gender){
                                    sprintf(&pStr[strlen(pStr)], "\n      Gender\t\tGender:%s Confidence:%d (x)",
                                                "Male", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                                }
                                else{
                                    sprintf(&pStr[strlen(pStr)], "\n      Gender\t\tGender:%s Confidence:%d (x)",
                                                "Female", pHVCResult->fdResult.fcResult[i].genderResult.confidence);
                                }
                            }
                        }
                    }
                    if(pHVCResult->executedFunc & HVC_ACTIV_GAZE_ESTIMATION){
                        /* Gaze */
                        if((-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR) ||
                            (-128 == pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD)){
                            sprintf(&pStr[strlen(pStr)], "\n      Gaze\t\tEstimation not possible");
                        }
                        else{
                            sprintf(&pStr[strlen(pStr)], "\n      Gaze\t\tLR:%d UD:%d",
                                        pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR, pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD);
                        }
                    }
                    if(pHVCResult->executedFunc & HVC_ACTIV_BLINK_ESTIMATION){
                        /* Blink */
                        if((-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioL) ||
                            (-128 == pHVCResult->fdResult.fcResult[i].blinkResult.ratioR)){
                            sprintf(&pStr[strlen(pStr)], "\n      Blink\t\tEstimation not possible");
                        }
                        else{
                            sprintf(&pStr[strlen(pStr)], "\n      Blink\t\tLeft:%d Right:%d",
                                        pHVCResult->fdResult.fcResult[i].blinkResult.ratioL, pHVCResult->fdResult.fcResult[i].blinkResult.ratioR);
                        }
                    }
                    if(pHVCResult->executedFunc & HVC_ACTIV_EXPRESSION_ESTIMATION){
                        /* Expression */
                        if(-128 == pHVCResult->fdResult.fcResult[i].expressionResult.score[0]){
                            sprintf(&pStr[strlen(pStr)], "\n      Expression\tEstimation not possible");
                        }
                        else{
                            if(pHVCResult->fdResult.fcResult[i].expressionResult.topExpression > EX_SADNESS){
                                pHVCResult->fdResult.fcResult[i].expressionResult.topExpression = 0;
                            }
                            sprintf(&pStr[strlen(pStr)], "\n      Expression\tExpression:%s Score:%d, %d, %d, %d, %d Degree:%d",
                                        pExStr[pHVCResult->fdResult.fcResult[i].expressionResult.topExpression],
                                        pHVCResult->fdResult.fcResult[i].expressionResult.score[0],
                                        pHVCResult->fdResult.fcResult[i].expressionResult.score[1],
                                        pHVCResult->fdResult.fcResult[i].expressionResult.score[2],
                                        pHVCResult->fdResult.fcResult[i].expressionResult.score[3],
                                        pHVCResult->fdResult.fcResult[i].expressionResult.score[4],
                                        pHVCResult->fdResult.fcResult[i].expressionResult.degree);
                        }
                    }
                }
            }
            
            for (i = 0; i < pHVCResult->fdResult.num; i++)
            {
                if (pHVCResult->fdResult.fcResult[i].ageResult.confidence >= 20500 && pHVCResult->fdResult.fcResult[i].genderResult.confidence >= 20500) {
                    p_listJSON->init();
                    p_listJSON->push("camera_id", "1");
                    if(pHVCResult->fdResult.fcResult[i].genderResult.gender == 1)
                    {
                        p_listJSON->push("sex_id", "1");
                    }
                    else if(pHVCResult->fdResult.fcResult[i].genderResult.gender == 0){
                        p_listJSON->push("sex_id", "2");
                    }
                    p_listJSON->push("age",to_string(pHVCResult->fdResult.fcResult[i].ageResult.age));
                    
                    p_listJSON->push("neutral",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[0]));
                    p_listJSON->push("happiness",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[1]));
                    p_listJSON->push("surprise",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[2]));
                    p_listJSON->push("anger",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[3]));
                    p_listJSON->push("sadness",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[4]));
                    p_listJSON->push("emotion",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.degree));
                    
                    p_listJSON->push("face_x",to_string(pHVCResult->fdResult.fcResult[i].dtResult.posX));
                    p_listJSON->push("face_y",to_string(pHVCResult->fdResult.fcResult[i].dtResult.posY));
                    p_listJSON->push("face_size",to_string(pHVCResult->fdResult.fcResult[i].dtResult.size));
                    p_listJSON->push("face_rbd",to_string(pHVCResult->fdResult.fcResult[i].dtResult.confidence));

                    p_listJSON->push("gaze_lr",to_string(pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR));
                    p_listJSON->push("gaze_ud",to_string(pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD));
                    p_listJSON->push("stabilization","1");

                    string tmp = p_listJSON->pop();
                    
                    if (!(tmp.empty())){
                        p_postCurl->send_post(tmp.c_str());
                        printf("送信完了 ログに書き込み\n");
                            
                            //--------------------
                            //ここからログ書き込み
                            time( &timer );
                            t_st = localtime(&timer);
                            
                            outputfile<< t_st->tm_year+1900 << "/" << t_st->tm_mon+1 << "/" << t_st->tm_mday << "_" << t_st->tm_hour << ":" << t_st->tm_min <<":" << t_st->tm_sec << "|" << tmp <<endl;
                    }
                 }
                 else if (pHVCResult->fdResult.fcResult[i].ageResult.confidence >= 10500 && pHVCResult->fdResult.fcResult[i].genderResult.confidence >= 10400)
                 {
                    p_listJSON->init();
                    p_listJSON->push("camera_id", "1");
                    if(pHVCResult->fdResult.fcResult[i].genderResult.gender == 1)
                    {
                        p_listJSON->push("sex_id", "1");
                    }
                    else if(pHVCResult->fdResult.fcResult[i].genderResult.gender == 0){
                        p_listJSON->push("sex_id", "2");
                    }
                    p_listJSON->push("age",to_string(pHVCResult->fdResult.fcResult[i].ageResult.age));
                    
                    p_listJSON->push("neutral",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[0]));
                    p_listJSON->push("happiness",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[1]));
                    p_listJSON->push("surprise",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[2]));
                    p_listJSON->push("anger",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[3]));
                    p_listJSON->push("sadness",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.score[4]));
                    p_listJSON->push("emotion",to_string(pHVCResult->fdResult.fcResult[i].expressionResult.degree));
                    
                    p_listJSON->push("face_x",to_string(pHVCResult->fdResult.fcResult[i].dtResult.posX));
                    p_listJSON->push("face_y",to_string(pHVCResult->fdResult.fcResult[i].dtResult.posY));
                    p_listJSON->push("face_size",to_string(pHVCResult->fdResult.fcResult[i].dtResult.size));
                    p_listJSON->push("face_rbd",to_string(pHVCResult->fdResult.fcResult[i].dtResult.confidence));
                    
                    p_listJSON->push("gaze_lr",to_string(pHVCResult->fdResult.fcResult[i].gazeResult.gazeLR));
                    p_listJSON->push("gaze_ud",to_string(pHVCResult->fdResult.fcResult[i].gazeResult.gazeUD));

                    p_listJSON->push("stabilization","0");
                    
                    string tmp = p_listJSON->pop();
                    if (!(tmp.empty()))
                    {
                        p_postCurl->send_post(tmp.c_str());
                        //printf("送信完了 ログに書き込み\n");
                        
                        //--------------------
                        //ここからログ書き込み
                        time( &timer );
                        t_st = localtime(&timer);
                        
                        outputfile<< t_st->tm_year+1900 << "/" << t_st->tm_mon+1 << "/" << t_st->tm_mday << "_" << t_st->tm_hour << ":" << t_st->tm_min <<":" << t_st->tm_sec << "|" << tmp <<endl;
                        
                        }
                    }
                }
            //}

            //一定時間立ったらlogファイルをcloseしてopenする
            elapsedTimeSecond = difftime(time(NULL) ， timeElapsed);
                
            //10分経過
            if( elapsedTimeSecond >= 120 )
            {
                cout << "自動書き込み完了" << endl;
                outputfile.close();
                outputfile.open("/home/pi/TinyRetail-hvcp2/log/log.txt" ,ios::app);
                timeElapsed = time(NULL);
            }

            if (kbhit())
            {
                printf("キーボードが押されたので終了します。\n", getchar());
                break;
            }
            
            //usleep(100000);
        } while( ch != ' ' );
    } while(0);
    
    outputfile.close();
    
    /******************/
    /* Log Output     */
    /******************/
    PrintLog(pStr);

    /********************************/
    /* Free result area             */
    /********************************/
    if( pHVCResult != NULL ){
        free(pHVCResult);
    }

    /*********************************/
    /* STB Finalize                  */
    /*********************************/
    STB_Final();

    com_close();
    /* Free Logging Buffer */
    if ( pStr != NULL ) {
        free(pStr);
    }
    return (0);
}
