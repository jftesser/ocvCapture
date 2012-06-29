#include "ProjectorControl.hpp"

SerialComm::SerialComm() {
	setup();
}

SerialComm::~SerialComm() {

}

void SerialComm::setup()
{
	Error=false;
	Stand=false;
	PJOn=false;

	//search for available devices and aquire the projector as a Serial::Device
	const vector<Serial::Device> &devices( Serial::getDevices() );
	for( vector<Serial::Device>::const_iterator deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt ) {
		console() << "Device: " << deviceIt->getName() << endl;
	}
	try {
		Serial::Device dev = Serial::findDeviceByNameContains("COM1");
		serial = Serial( dev, 115200);
	}
	catch( ... ) {
		console() << "There was an error initializing the serial device!" << std::endl;
		//exit( -1 );
	}
}

bool SerialComm::StandBy()
{
	//Assumes that the projector has already been established as a serial device named 'serial'

	uint8_t byte;

	//send startbyte
	byte=STARTBYTE;
	serial.writeByte(byte);
	
	//send device number (assumed 00)
	byte=0x00;
	serial.writeByte(byte);

	//send command byte one
	byte=0x58;
	serial.writeByte(byte);

	//send data byte
	byte=0x00;
	serial.writeByte(byte);

	//send checksum
	byte=0x58;
	serial.writeByte(byte);

	//send endbyte
	byte=ENDBYTE;
	serial.writeByte(byte);

	return CheckAck(); //will call something similar to the code in update() to check for ACK
}

bool SerialComm::ProjectorOn(int lamps)
{
	//Assumes that the projector has already been established as a serial device named 'serial'

	uint8_t byte;

	//send startbyte
	byte=STARTBYTE;
	serial.writeByte(byte);
	
	//send device number (assumed 00)
	byte=0x00;
	serial.writeByte(byte);

	//send command byte one
	byte=0x58;
	serial.writeByte(byte);

	//send data byte
	byte=0x03; //this is the auto setting
	if (lamps == 1) byte=0x01;
	if (lamps == 2) byte=0x02;
	serial.writeByte(byte);

	//send checksum
	byte=0x5b;
	if (lamps == 1) byte=0x59;
	if (lamps == 2) byte=0x5a;
	serial.writeByte(byte);

	//send endbyte
	byte=ENDBYTE;
	serial.writeByte(byte);

	return CheckAck(); //will call something similar to the code in update() to check for ACK
}

bool SerialComm::CheckAck(){
	
	uint8_t len = 0,totlen = 0;
	uint8_t byte;
	uint8_t rcvd[128];
		
	ctr=0;
	while(totlen<6){
		console() << len << endl;	
		byte=0x00;
		totlen+=len;
		for (;ctr<(totlen);ctr++) {
			byte=serial.readByte();
			rcvd[ctr]=byte;
		}
		len=serial.getNumBytesAvailable();

		if (len == 0) break; // otherwise there is an infinite loop when no projector is attached
	}
	/* used for debugging

	for (ctr=0;ctr<totlen;ctr++) {
		console() << int(rcvd[ctr]) << " " ;
		if (ctr==len-1) {console() << endl;}
	}
	*/
	//here check to see if it is an ack/nack

	if (rcvd[2]==0x00) {
		// it's an ack/nack
		if (rcvd[3]==0x06) return true; //command acknowledged
		if (rcvd[3]==0x015) return false;
	}
	return false;
}

void SerialComm::mouseDown( MouseEvent event )
{
}

void SerialComm::keyDown(KeyEvent event) {
	if( event.getChar() == 's' ){
		if (!StandBy()) { Error=true;}
		else {Stand=true;PJOn=false;}
	}
	if(event.getChar() == 'o' ) {
		if (!ProjectorOn(1)) {Error=true;}
		else {PJOn=true;Stand=false;}
	}
	//if (event.getChar() == 'q') {
	//	quit();
	//}
}

void SerialComm::update()
{
}

/*void SerialComm::draw()
{
	// clear out the window with black

	if (Stand) { gl::clear(Color(0,0,1));}
	else if (PJOn) {gl::clear(Color(0,1,0));}
	else {gl::clear( Color( 0,0,0) );}
	if (Error) {gl::clear(Color(1,0,0));}
}*/
