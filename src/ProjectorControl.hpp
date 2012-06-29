#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Serial.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "dos.h"

#include <sstream>

using namespace ci;
using namespace ci::app;
using namespace std;

#define STARTBYTE 0xfe
#define ENDBYTE 0xff

class SerialComm {
  public:
	SerialComm();
	~SerialComm();

	void setup();
	void mouseDown( MouseEvent event );	
	void keyDown(KeyEvent event);
	void update();
	//void draw();
	bool StandBy();
	bool ProjectorOn(int lamps);
	bool CheckAck();

	bool Error;
	bool Stand;
	bool PJOn;
	
	Serial serial;
	uint8_t			ctr;
	std::string lastString;
};