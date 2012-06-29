#pragma once

#include "FlyCapture2.h"
#include "opencv\cv.h"
#include "opencv\highgui.h"
#include <stdio.h>
#include <tchar.h>

using namespace FlyCapture2;

class flyCap
{
public:
	flyCap(void);
	~flyCap(void);

	void PrintBuildInfo();
	void PrintCameraInfo(CameraInfo* pCamInfo );
	void PrintError( Error error );
	int RunSingleCamera( PGRGuid guid );

	int start();
	int stop();

	cv::Mat grab();
	

	Camera mCam;
	PGRGuid mGuid;
	bool mCapturing;
};

