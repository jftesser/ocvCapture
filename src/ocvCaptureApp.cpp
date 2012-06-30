#include "cinder/app/AppBasic.h"
#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
//#include "cinder/Capture.h"
#include "cinder/qtime/QuickTime.h"
#include "CinderOpenCV.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include <vector>
#include "OpenCV\cv.h"
#include "OpenCV\highgui.h"
#include "flyCap.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include "ProjectorControl.hpp"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace ci::qtime;

class ocvCaptureApp : public AppBasic {
public:
	void setup();
	void prepareSettings( Settings *settings );
	void update();
	void draw();
	void keyDown(KeyEvent event);

	void readOS();
	void saveOS();

	// replicated from gooze
	cv::Mat alpha(cv::Mat m);
	cv::Mat mul(cv::Mat m1, cv::Mat m2);
	cv::Mat mul(double amt, cv::Mat m);
	cv::Mat diff(cv::Mat m);
	cv::Mat blur(cv::Mat m, int amt);
	cv::Mat thresh(cv::Mat m);
	cv::Mat contour(cv::Mat m);
	cv::Mat color(cv::Mat m);
	cv::Mat rgb(float r, float g, float b);
	cv::Mat lines(cv::Mat m, cv::Mat dest, cv::Scalar col, int thick);
	cv::Mat threshlines(cv::Mat thresh, cv::Mat dest, cv::Scalar col, int thick);
	cv::Mat trail(cv::Mat m);
	cv::Mat mix(cv::Mat m, int type);
	cv::Mat squash(cv::Mat m);
	cv::Mat resaturate(cv::Mat m, double amt);
	cv::Mat invert(cv::Mat m);

	cv::Mat paint108(cv::Mat m);
	cv::Mat paint107(cv::Mat m);
	cv::Mat paint109(cv::Mat m);
	cv::Mat blackboard3(cv::Mat m);

	cv::Mat process_mov(cv::Mat m);
	cv::Mat process_cap(cv::Mat m);

	CvCapture							*mThisMovie;
	flyCap								*mCap;
	vector < CvCapture *>				mMovieCaps;
	vector <std::string>				mMovies;
	int									mCurrMovie;
	int									mLastFrame;
	gl::Texture							mTexture;
	gl::Texture							mMaskTexture;

	cv::Mat								mMovieMat;
	bool								mHaveMovieMat;
	bool								mPlayingMov;
	cv::Mat								mCapMat;
	cv::Mat								mLastCapMat;
	cv::Mat								mColorCap;
	bool								mHaveCapMat;
	bool								mHaveBoth;
	bool								mCapturing;

	cv::Mat								mAccCap;
	cv::Mat								mAccAdd;
	cv::Mat								mAdd;

	cv::Mat								mLastCalc;
	cv::Mat								mLastLineCalc;
	cv::Mat								mLastOut;

	int									mCnt;
	qtime::MovieWriter					mWriter;
	bool								mWriting;
	std::vector< std::vector<cv::Point> > mCvContours;

	int									mH, mW;
	int									gH, gW;
	int									cH, cW;
	float								mCapSc;

	Vec2f								mV0, mV1, mV2, mV3;
	Vec2f								mOS0, mOS1, mOS2, mOS3;

	SerialComm							*mComm;

	int									mLiveType; // 0 - black ink 1 - white ink 2 - feed ink 3 - overwrite processed grey 4 - overwrite processed spot
	bool								mDrawLines;
};

void ocvCaptureApp::setup()
{
	try {

		mComm = new SerialComm();
		mComm->ProjectorOn(1);
		//mMaskSrf = loadImage("../resources/mask_cross.png");
		//bool has_alpha = mMaskSrf.hasAlpha();

		//mMaskSrf.setPremultiplied(true);
		//bool premultiplied = mMaskSrf.isPremultiplied ();
		mH = 1080*1.0;
		mW = 1920*1.0;
		cH = 1080*0.75;
		cW = 1920*0.75;
		gH = 1080;
		gW = 1920;

		mLiveType = 4;
		mDrawLines = false;

		mV0 = Vec2f(0, 0);
		mV1 = Vec2f(gW, 0);
		mV2 = Vec2f(gW, gH);
		mV3 = Vec2f(0, gH);

		mOS0 = mOS1 = mOS2 = mOS3 = Vec2f(0, 0);
		readOS();

		mCapSc = 0.2;

		mCurrMovie = -1;
		mHaveMovieMat = false;
		mHaveCapMat = false;
		mHaveBoth = false;

		mAccAdd = cv::Mat(mW,mH,CV_8UC1,cvScalar(0));
		mAccCap = cv::Mat(mW*mCapSc,mH*mCapSc,CV_8UC1,cvScalar(0));

		mColorCap = cv::Mat(mW*mCapSc,mH*mCapSc,CV_8UC3,cvScalar(0,0,0));

		mMaskTexture = loadImage("../resources/mask_nocross.png"); //gl::Texture( mMaskSrf );

		mCapturing = false;
		mCap = new flyCap();
		if (mCap) {
			mCap->start();
			mCapturing = mCap->mCapturing;
		}

		mLastCalc = NULL;
		mLastOut = NULL;
		mCnt = 0;

		vector <std::string> mpaths;
		/*mpaths.push_back("C:/black_adults_axial.mov");
		mpaths.push_back("C:/black_adults_axial2.mov");
		mpaths.push_back("C:/black_teens_Section.mov");
		mpaths.push_back("C:/grey_kids_section.mov");
		mpaths.push_back("C:/spot_teens.mov");
		mpaths.push_back("C:/spot_teens2.mov");
		mpaths.push_back("C:/white_adults_axial.mov");
		mpaths.push_back("C:/white_adults_axial_best.mov");
		mpaths.push_back("C:/white_adults_axial2.mov");
		mpaths.push_back("C:/white_kids_section.mov");
		mpaths.push_back("C:/white_kids_section2.mov");
		mpaths.push_back("C:/white_kids_section3.mov");
		mpaths.push_back("C:/white_teens_section.mov");*/
		mpaths.push_back("C:/Apr19_6_4C_B.avi");
		mpaths.push_back("C:/Apr19_6_4C_G.avi");
		mpaths.push_back("C:/Apr19_6_4C_S.avi");
		mpaths.push_back("C:/Apr19_6_4C_W.avi");
		mMovies = mpaths;

		for (int i=0;i<(int)mMovies.size();i++) {
			CvCapture *m = cvCaptureFromFile(mpaths[i].c_str());
			mMovieCaps.push_back(m);
		}


		mThisMovie = mMovieCaps[0];
		mLastFrame = (int)cvGetCaptureProperty(mThisMovie, CV_CAP_PROP_FRAME_COUNT);
		mCurrMovie = 0;
		mPlayingMov = true;

		mWriting = false;
	}
	catch( ... ) {
		console() << "Failed to initialize capture" << std::endl;
	}
}
void ocvCaptureApp::prepareSettings( Settings *settings ){
	settings->setWindowSize( 1920, 1080 );
	//settings->setFullScreen(true);
	settings->setFrameRate( 30.0f );
}

void ocvCaptureApp::readOS() {
	string line;
	std::vector <float> oses;
	ifstream myfile;
	myfile.open("../resources/os.txt", ifstream::in);
	if (myfile.is_open())
	{
		while ( myfile.good() )
		{
			getline (myfile,line);
			float os = atof(line.c_str());
			oses.push_back( os );
		}
		myfile.close();
	} else {
		cout << "unable to open file" <<endl;
	}

	if (oses.size() >= 8) {
		mOS0.x = oses[0];
		mOS0.y = oses[1];
		mOS1.x = oses[2];
		mOS1.y = oses[3];
		mOS2.x = oses[4];
		mOS2.y = oses[5];
		mOS3.x = oses[6];
		mOS3.y = oses[7];
	}

}

void ocvCaptureApp::saveOS() {

	ofstream myfile; 
	myfile.open("../resources/os.txt", ifstream::out);
	if (myfile.is_open())
	{
		char mess[500];
		sprintf(mess,"%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f",mOS0.x,mOS0.y,mOS1.x,mOS1.y,mOS2.x,mOS2.y,mOS3.x,mOS3.y);
		myfile << strdup(mess);
		myfile.close();
	} else {
		cout << "unable to open file" <<endl;
	}

	
}

cv::Mat ocvCaptureApp::process_mov(cv::Mat m) {
	cv::resize(m,m,cvSize(mW,mH));//1920,1080));
	return m;
}

cv::Mat ocvCaptureApp::process_cap(cv::Mat m) {
	cv::Mat last;
	cv::resize(m,m,cvSize(mH*mCapSc,mW*mCapSc));//1920,1080));
	if (mLastCapMat.dims == 2) {
		cv::resize(mLastCapMat,last,cvSize(mH*mCapSc,mW*mCapSc));
		cv::absdiff(last,m,m);
		mAccCap *= 0.7;
		cv::add(mAccCap,m,mAccCap);
		cv::threshold(mAccCap,mAccCap,255,255,CV_THRESH_TRUNC);
		mHaveBoth = true;
	} else {
		mHaveBoth = false;
	}

	return mAccCap;
}

void ocvCaptureApp::update()
{
	if (mCapturing && mCap) {
		mLastCapMat = mCapMat;
		if (mLiveType == 4) {
			mCapMat = mCap->grab();
			mColorCap = mCap->grab(true);
			//cv::cvtColor(mColorCap,mCapMat,CV_BGR2GRAY); doesn't work, not sure why
		} else
			mCapMat = mCap->grab();
		mHaveCapMat = true;
	}

	if (mPlayingMov && mThisMovie && mLiveType < 3) {
		IplImage *img = cvQueryFrame(mThisMovie);
		if (img) {
			mMovieMat = img;
			//mMovieMat = toOcv( mThisMovie.getSurface(),CV_8UC3 );//CV_32FC3 );
			mHaveMovieMat = true;
		}
	}

	int w, h;
	if ( mLiveType < 3) {
		w = mW;
		h = mH;
	} else {
		w = cW;
		h = cH;
	}

	if (mHaveMovieMat || mHaveCapMat) {
		// have something to process and display
		cv::Mat to_display;

		if (mHaveMovieMat && mPlayingMov && mLiveType < 3) {
			to_display = process_mov(mMovieMat);
		} else {
			
			to_display = cv::Mat(h,w,CV_8UC3,cvScalar(150,150,150) );//CV_32FC3,cvScalar(0.5,0.5,0.5));
			
		}

		//cv::threshold(to_display,bright,200,255,cv::THRESH_TOZERO); //THRESH_TOZERO
		//cv::add(to_display,bright,to_display);
		mAdd = cv::Mat(h,w,CV_8UC1,cvScalar(0));

		

		if (mHaveCapMat && mCapturing) {
			cv::Mat capmat = process_cap(mCapMat);
			if (mHaveBoth) {
				//
				cv::Mat capthresh, capthreshbig; // capgrey,
				//cv::cvtColor(capmat, capgrey, CV_BGR2GRAY); // cap is now greyscale
				cv::threshold(capmat,capthresh,100,100,cv::THRESH_BINARY);
				mAdd = threshlines(capthresh,mAdd,cvScalar(255,255,255),1);
				if (mAccAdd.cols != w || mAccAdd.rows != h) {
					cv::resize(mAccAdd,mAccAdd,cv::Size(w,h));
				}
				cv::Mat coladd;
				mAccAdd *= 0.75; // decay on accumulation
				cv::add(mAdd,mAccAdd,mAccAdd);
				cv::threshold(mAccAdd,mAccAdd,255,255,CV_THRESH_TRUNC);
				if (mLiveType < 3) {
					if (mDrawLines == false)
						cv::blur(mAccAdd,mAccAdd,cvSize(7,7));
					cv::cvtColor(mAccAdd,coladd,CV_GRAY2BGR);
				} else {
					coladd = mAccAdd;
				}

				if (mLiveType == 0) {
					cv::subtract(to_display,coladd,to_display);
				} else if (mLiveType == 1) {
					cv::add(to_display,coladd,to_display);
				} else if (mLiveType == 2) {
					cv::subtract(to_display,coladd,to_display);
					cv::Mat invadd, bigcap;
					cv::absdiff(mAccAdd,cv::Scalar(255),invadd);
					cv::resize(mCapMat,bigcap,cv::Size(w,h));
					cv::subtract(bigcap,invadd,bigcap);
					cv::cvtColor(bigcap,bigcap,CV_GRAY2BGR);
					cv::add(to_display,bigcap,to_display);
				} else if (mLiveType == 3) {
					to_display = coladd;
				} else if (mLiveType == 4) {
					cv::Mat inv, bigcolorcap;
					cv::subtract(cv::Scalar(255),coladd,inv);
					cv::cvtColor(inv, inv, CV_GRAY2BGR);
					cv::resize(mColorCap,bigcolorcap,cv::Size(w,h));
					cv::subtract(bigcolorcap,inv,to_display);
				}
				/*
				cv::Mat bigacc;
				cv::resize(mAccCap,bigacc,cvSize(mH,mW));
				cv::cvtColor(bigacc,bigacc,CV_GRAY2BGR);
				cv::add(to_display,bigacc,to_display);*/
			}
		}


		/*cv::threshold(m,dark,4,1,0); //THRESH_BINARY_INV
		cv::multiply(to_display,dark,to_display);*/
		//cv::absdiff(to_display,m,to_display); // should actually mix, not just add
		//}

		//cv::resize(to_display,to_display,cvSize(1920,1080));

		mTexture = gl::Texture( fromOcv(to_display) );
	} 



	if (mPlayingMov) {
		// check if we've finished a movie
		if (mThisMovie) {
			int now = (int)cvGetCaptureProperty(mThisMovie,CV_CAP_PROP_POS_FRAMES);
			if (now >= mLastFrame-5) {
				// time to switch

				++mCurrMovie;
				if (mCurrMovie >= (int)mMovies.size())
					mCurrMovie = 0;

				//int next_m = mCurrMovie + 1;
				//if (next_m >= mMovies.size())
				//	next_m = 0;

				//mNextMovie = mMovieCaps[next_m];

				mThisMovie = mMovieCaps[mCurrMovie];

				cvSetCaptureProperty(mThisMovie,CV_CAP_PROP_POS_FRAMES,0);

				mLastFrame = (int)cvGetCaptureProperty(mThisMovie,CV_CAP_PROP_FRAME_COUNT);
			}
		}

	}

}

void ocvCaptureApp::draw()
{
	if (mWriting) {
		writeImage( "C:/movie/test/" + toString<int>(getElapsedFrames()) + ".jpg", copyWindowSurface() );
	}

	gl::clear();
	//gl::enableAlphaBlending( false );
	//gl::enableAlphaTest	(0.5f,GL_GREATER);
	//gl::enableAdditiveBlending ();


	gl::pushMatrices ();
	//gl::rotate(90);
	//gl::color(0.0/255.0,57.0/255.0,255.0/255.0);


	//if( mTexture )
	//	gl::draw( mTexture, Rectf( 0.0, 0.0, 1920, 1080 ) );

	if (mTexture) {
		// Vertices of quadrilateral
		Vec2f vert0 = mV0 + mOS0; //(0, 0);
		Vec2f vert1 = mV1 + mOS1; //(mW, 0);
		Vec2f vert2 = mV2 + mOS2; //(mW, mH);
		Vec2f vert3 = mV3 + mOS3; //(0, mH);

		// Bind texture to draw with it
		mTexture.enableAndBind();

		// Draw the mask by drawing two triangles, 
		// setting the texture coordinate before each vertex
		glBegin(GL_QUADS);
		{

			glTexCoord2f(0, 0);
			gl::vertex(vert0);
			glTexCoord2f(1, 0);
			gl::vertex(vert1);
			glTexCoord2f(1, 1);
			gl::vertex(vert2);
			glTexCoord2f(0, 1);
			gl::vertex(vert3);
		}
		glEnd();
	}


	//gl::draw(mMaskTexture);
	gl::popMatrices ();

	//if (mMaskTexture)
	//	gl::draw(mMaskTexture,Rectf( 0.0, 0.0, mW, mH ));
	//	mWriter.addFrame( copyWindowSurface() );

}

void ocvCaptureApp::keyDown(KeyEvent event) {
	if (event.getCode() == event.KEY_ESCAPE) {
		mComm->StandBy();
		mCap->stop();
		shutdown();
		exit(0);
	} else if( event.getChar() == '1' ){
		mLiveType = 0;
	} else if( event.getChar() == '2' ){
		mLiveType = 1;
	} else if( event.getChar() == '3' ){
		mLiveType = 2;
	} else if( event.getChar() == '4' ){
		mLiveType = 3;
	} else if( event.getChar() == '5' ){
		mLiveType = 4;
	} else if( event.getChar() == 'e' ){
		mDrawLines = true;
	} else if( event.getChar() == 'r' ){
		mDrawLines = false;
	} else if( event.getChar() == 'q' ){
		mPlayingMov = true;
	} else if ( event.getChar() == 'w' ){
		//mThisMovie.stop();
		mPlayingMov = false;
	} else if ( event.getChar() == 'a' ){
		if (mCap->mCapturing == false) {
			mCap = new flyCap();
			mCap->start();
			mCapturing = mCap->mCapturing;
		}
		mCapturing = mCap->mCapturing;
	} else if ( event.getChar() == 's' ){
		mCapturing = false;
	} else if( event.getChar() == 'c' ){
		mWriting = true;
	} else if( event.getChar() == '=' ){
		saveOS();
	} else if (event.isShiftDown()) {
		if( event.getChar() == '^' ){
			mOS0.x += 1;
		} else if( event.getChar() == 'Y' ){
			mOS0.x -= 1;
		} else if( event.getChar() == 'H' ){
			mOS0.y += 1;
		} else if( event.getChar() == 'N' ){
			mOS0.y -= 1;
		} else if( event.getChar() == '&' ){
			mOS1.x += 1;
		} else if( event.getChar() == 'U' ){
			mOS1.x -= 1;
		} else if( event.getChar() == 'J' ){
			mOS1.y += 1;
		} else if( event.getChar() == 'M' ){
			mOS1.y -= 1;
		} else if( event.getChar() == '*' ){
			mOS2.x += 1;
		} else if( event.getChar() == 'I' ){
			mOS2.x -= 1;
		} else if( event.getChar() == 'K' ){
			mOS2.y += 1;
		} else if( event.getChar() == '<' ){
			mOS2.y -= 1;
		} else if( event.getChar() == '(' ){
			mOS3.x += 1;
		} else if( event.getChar() == 'O' ){
			mOS3.x -= 1;
		} else if( event.getChar() == 'L' ){
			mOS3.y += 1;
		} else if( event.getChar() == '>' ){
			mOS3.y -= 1;
		}
	}
	else if( event.getChar() == '6' ){
		mOS0.x += 10;
	} else if( event.getChar() == 'y' ){
		mOS0.x -= 10;
	} else if( event.getChar() == 'h' ){
		mOS0.y += 10;
	} else if( event.getChar() == 'n' ){
		mOS0.y -= 10;
	} else if( event.getChar() == '7' ){
		mOS1.x += 10;
	} else if( event.getChar() == 'u' ){
		mOS1.x -= 10;
	} else if( event.getChar() == 'j' ){
		mOS1.y += 10;
	} else if( event.getChar() == 'm' ){
		mOS1.y -= 10;
	} else if( event.getChar() == '8' ){
		mOS2.x += 10;
	} else if( event.getChar() == 'i' ){
		mOS2.x -= 10;
	} else if( event.getChar() == 'k' ){
		mOS2.y += 10;
	} else if( event.getChar() == ',' ){
		mOS2.y -= 10;
	} else if( event.getChar() == '9' ){
		mOS3.x += 10;
	} else if( event.getChar() == 'o' ){
		mOS3.x -= 10;
	} else if( event.getChar() == 'l' ){
		mOS3.y += 10;
	} else if( event.getChar() == '.' ){
		mOS3.y -= 10;
	}


	//mComm->keyDown(event);

}

cv::Mat ocvCaptureApp::alpha(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::mul(cv::Mat m1, cv::Mat m2) {
	cv::Mat out;
	cvMul(&m1,&m2,&out);
	return out;
}

cv::Mat ocvCaptureApp::mul(double amt, cv::Mat m) {
	cv::Mat out;
	cvScale(&m,&out,amt);
	return out;
}

cv::Mat ocvCaptureApp::diff(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::blur(cv::Mat m, int amt) {
	cv::blur(m,m,cvSize(amt,amt));
	return m;
}
cv::Mat ocvCaptureApp::thresh(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::contour(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::color(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::rgb(float r, float g, float b) {
	cv::Mat m;
	m = cvCreateMat(mH,mW,CV_8UC3);//1920,1080,CV_8UC3);
	cvSet(&m,cvScalar(r,g,b),NULL);
	return m;
}
cv::Mat ocvCaptureApp::threshlines(cv::Mat thresh, cv::Mat dest, cv::Scalar col, int thick) {
	vector <vector <cv::Point>> contours;
	cv::findContours(thresh,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_TC89_KCOS);

	cv::Mat fill, lines;
	int w, h;
	w = dest.cols;
	h = dest.rows;

	if (mLiveType == 3) {
		fill = cv::Mat(h,w,CV_8UC1,cvScalar(30) );
		lines = cv::Mat(h,w,CV_8UC1,cvScalar(0) );
	} else if (mLiveType == 4) {
		fill = cv::Mat(h,w,CV_8UC1,cvScalar(0) );
		lines = cv::Mat(h,w,CV_8UC1,cvScalar(0) );
	}

	double ysc = dest.rows/(1.0*thresh.rows);
	double xsc = dest.cols/(1.0*thresh.cols);
	for (int i=0; i<(int)contours.size(); i++) {
		vector <cv::Point> pts = contours[i];
		if (pts.size() > 3) {
			for (int j=0; j<(int)pts.size(); j++) {
				pts[j].x *= xsc;
				pts[j].y *= ysc;
			}

			if (mLiveType < 3) {
				if (mDrawLines) {
					cv::polylines(dest,pts,true,cv::Scalar(60),1);
				} else {
					vector <vector <cv::Point>> poly;
					poly.push_back(pts);
					cv::fillPoly(dest,poly,cv::Scalar(60));
				} 
			} else {
				cv::polylines(lines,pts,true,cv::Scalar(255),1);
				vector <vector <cv::Point>> poly;
				poly.push_back(pts);
				cv::fillPoly(fill,poly,cv::Scalar(60));
			}
			//cv::fillConvexPoly(dest,pts,cv::Scalar(120));
		}
	}
	
	if (mLiveType == 3) {
		cv::blur(fill,fill,cv::Size(5,5));
		cv::subtract(fill,lines,fill);
		return fill;
	} else if (mLiveType == 4) {
		cv::blur(fill,fill,cv::Size(5,5));
		cv::add(fill,lines,fill);
		cv::threshold(fill,fill,255,255,CV_THRESH_TRUNC);
		return fill;
	}
	return dest;
}
cv::Mat ocvCaptureApp::lines(cv::Mat m, cv::Mat dest, cv::Scalar col, int thick) {
	vector <vector <cv::Point>> contours;
	cv::Mat thresh;
	cv::Mat grey;
	cv::cvtColor(m, grey, CV_BGR2GRAY);

	cv::threshold(grey,thresh,100,255,cv::THRESH_BINARY); //cv::mean(grey).val[0]
	//cv::adaptiveThreshold(grey,thresh,255,cv::ADAPTIVE_THRESH_MEAN_C,cv::THRESH_BINARY,7,0.0);
	cv::findContours(thresh,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_TC89_KCOS);

	double ysc = dest.rows/m.rows;
	double xsc = dest.cols/m.cols;

	for (int i=0; i<(int)contours.size(); i++) {
		vector <cv::Point> pts = contours[i];
		if (pts.size() > 1) {
			for (int j=0; j<(int)pts.size(); j++) {
				cv::Point st;
				cv::Point en = pts[j];
				if (j==0) {
					st = pts[pts.size()-1];
				} else {
					st = pts[j-1];
				}
				st.x *= xsc;
				st.y *= ysc;
				en.x *= xsc;
				en.y *= ysc;

				// draw line
				cv::line(dest,st,en,col,thick);
			}
		}
	}

	return dest;
}
cv::Mat ocvCaptureApp::trail(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::mix(cv::Mat m, int type) {
	return m;
}
cv::Mat ocvCaptureApp::squash(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::resaturate(cv::Mat m, double amt) {
	cv::Mat hsv, s, out;
	cv::cvtColor(m,hsv,CV_BGR2HSV);
	cvSplit(&hsv,NULL,&s,NULL,NULL);
	cvScale(&s,&s,amt+1.0);
	int from_to[] = { 0,1 };
	cv::mixChannels(&s,1,&hsv,3,from_to,1);
	cv::cvtColor(hsv,out,CV_HSV2BGR);
	return out;
}
cv::Mat ocvCaptureApp::invert(cv::Mat m) {
	cv::Mat out;
	double min = 0.0;
	double max = 0.0;
	cvMinMaxLoc(&m,&min,&max);
	CvScalar s = cvScalarAll(max);
	cvSubRS(&m,s,&out);
	return out;
}

cv::Mat ocvCaptureApp::paint108(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::paint107(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::paint109(cv::Mat m) {
	return m;
}
cv::Mat ocvCaptureApp::blackboard3(cv::Mat m) {
	return m;
}


CINDER_APP_BASIC( ocvCaptureApp, RendererGl )
