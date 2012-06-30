#include "flyCap.hpp"


flyCap::flyCap(void)
{
	PrintBuildInfo();

    Error error;

	mCapturing = false;

    BusManager busMgr;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
    }

    printf( "Number of cameras detected: %u\n", numCameras );

    for (unsigned int i=0; i < numCameras; i++)
    {
        PGRGuid guid;
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != PGRERROR_OK)
        {
            PrintError( error );
        } else {
			// found out camera
			mGuid = guid;
			break;
		}
    }
}


flyCap::~flyCap(void)
{
}

void flyCap::PrintBuildInfo()
{
    FC2Version fc2Version;
    Utilities::GetLibraryVersion( &fc2Version );
    char version[128];
    sprintf( 
        version, 
        "FlyCapture2 library version: %d.%d.%d.%d\n", 
        fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build );

    printf( version );

    char timeStamp[512];
    sprintf( timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__ );

    printf( timeStamp );
}

void flyCap::PrintCameraInfo( CameraInfo* pCamInfo )
{
    printf(
        "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "Resolution - %s\n"
        "Firmware version - %s\n"
        "Firmware build time - %s\n\n",
        pCamInfo->serialNumber,
        pCamInfo->modelName,
        pCamInfo->vendorName,
        pCamInfo->sensorInfo,
        pCamInfo->sensorResolution,
        pCamInfo->firmwareVersion,
        pCamInfo->firmwareBuildTime );
}

void flyCap::PrintError( Error error )
{
    error.PrintErrorTrace();
}

int flyCap::start() {
	Error error;
	error = mCam.Connect(&mGuid);

	 if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

	// Get the camera information
    CameraInfo camInfo;
    error = mCam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    PrintCameraInfo(&camInfo);        
	
    // Start capturing images
    error = mCam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

	mCapturing = true;

	return 0;
}

int flyCap::stop() {
	Error error;

	// Stop capturing images
    error = mCam.StopCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }      

    // Disconnect the camera
    error = mCam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

	mCapturing = false;
    return 0;
}

cv::Mat flyCap::grab(bool color) {
	cv::Mat m;
	if (mCapturing) {
		Error error;
		Image rawImage;  
		IplImage *cvImage = NULL;

		// Retrieve an image
		error = mCam.RetrieveBuffer( &rawImage );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return m;
		}

		printf( "Grabbed image\n" );


		// Get the raw image dimensions
		FlyCapture2::PixelFormat pixFormat;
		unsigned int rows, cols, stride;
		rawImage.GetDimensions( &rows, &cols, &stride, &pixFormat );

		// Create a converted image
		FlyCapture2::Image convertedImage;

		// Convert the raw image
		if (color)
			error = rawImage.Convert( FlyCapture2::PIXEL_FORMAT_RGB8, &convertedImage );
		else
			error = rawImage.Convert( FlyCapture2::PIXEL_FORMAT_MONO8, &convertedImage );
		if (error != FlyCapture2::PGRERROR_OK)
		{
			error.PrintErrorTrace();
			return m;
		}

		//Copy the image into the IplImage of OpenCV
		if (color)
			cvImage = cvCreateImage(cvSize(rawImage.GetCols(), rawImage.GetRows()), IPL_DEPTH_8U, 3);
		else
			cvImage = cvCreateImage(cvSize(rawImage.GetCols(), rawImage.GetRows()), IPL_DEPTH_8U, 1);
		memcpy(cvImage->imageData, convertedImage.GetData(), convertedImage.GetDataSize());
		
		m = cvImage;
		// rgb -> bgr
		if (color)
			cv::cvtColor(m,m,CV_RGB2BGR);

		return m;
	} else {
		return m;
	}
}

int flyCap::RunSingleCamera( PGRGuid guid )
{
    const int k_numImages = 10;

    Error error;
    Camera cam;

    // Connect to a camera
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    // Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    PrintCameraInfo(&camInfo);        

    // Start capturing images
    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    Image rawImage;    
    for ( int imageCnt=0; imageCnt < k_numImages; imageCnt++ )
    {                
        // Retrieve an image
        error = cam.RetrieveBuffer( &rawImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            continue;
        }

        printf( "Grabbed image %d\n", imageCnt );

        // Create a converted image
        Image convertedImage;

        // Convert the raw image
        error = rawImage.Convert( PIXEL_FORMAT_MONO8, &convertedImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }  

        // Create a unique filename
        char filename[512];
        sprintf( filename, "FlyCapture2Test-%u-%d.pgm", camInfo.serialNumber, imageCnt );

        // Save the image. If a file format is not passed in, then the file
        // extension is parsed to attempt to determine the file format.
        error = convertedImage.Save( filename );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }  
    }            

    // Stop capturing images
    error = cam.StopCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }      

    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    return 0;
}

