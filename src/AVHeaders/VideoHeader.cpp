/*
 * AudioFrame.h
 *
 *  Created on: 09-oct-2008
 *      Author: arturo castro
 */

#include "VideoHeader.h"

namespace ofxPm{
VideoHeader::VideoHeader(VideoBuffer & buffer){
setup(buffer);
}

//------------------------------------------------------
VideoHeader::VideoHeader(){
    fps         = 25;
    position    = 0;
    oneFrame    = (TimeDiff)round(1000000.0/(double)fps);
    speed       = 1;
    in          = 1;
    out         = 0;
    delay       = 0;
	opacity		= 255;
    playing     = false;
	loopStart	= false;
	loopMode	= OF_LOOP_NORMAL;
	driveMode	= 0;
	currentPos  = 0;
	buffer 		= NULL;
}


//------------------------------------------------------
void VideoHeader::setup(VideoBuffer & buffer){
    //newFrameEvent.init("Playmodes.VideoHeader.newFrame");
    this->buffer= &buffer;
    fps         = buffer.getFps();
    position    = buffer.size();
    oneFrame    = (TimeDiff)round(1000000.0/(double)fps);
    speed       = 1;
    in          = 1;
    out         = 0;
    playing   	= false;
    delay       = 0;
	opacity		= 255;
	loopStart	= false;
	loopMode	= OF_LOOP_NORMAL;
	driveMode	= 0;
	currentPos  = 0;
}

//------------------------------------------------------
VideoHeader::~VideoHeader(){
}

//------------------------------------------------------
void VideoHeader::draw(){

	
    double oneLength=(double)(ofGetWidth()-PMDRAWSPACING*2)/(double)(buffer->getMaxSize());
    double currentLength=double(currentPos+1)/((double)this->buffer->getMaxSize())*(double)(ofGetWidth()-PMDRAWSPACING*2);
	
	// draw header line
    ofSetLineWidth(1);
	if(playing) ofSetColor(0,200,0);
	else ofSetColor(255,128,0);

	ofRect(currentLength+PMDRAWSPACING-oneLength,PMDRAWELEMENTSY+30,oneLength,10);
	ofDrawBitmapString(ofToString(buffer->size()-1-currentPos),ofPoint(currentLength+PMDRAWSPACING-(oneLength/2),PMDRAWELEMENTSY+25));

	ofSetColor(255,128,0);
	
	int	inFrame  = int(double(buffer->size()-1)*(in));
	int outFrame = int(double(buffer->size()-1)*(out));
	int inPos = PMDRAWSPACING + ((buffer->size()-1-inFrame) * oneLength) + oneLength/2;
	int outPos = PMDRAWSPACING + ((buffer->size()-1-outFrame) * oneLength) + oneLength/2;
		
	// draw in & out lines
	ofSetLineWidth(1.0);
	ofLine(inPos,PMDRAWELEMENTSY+10,inPos,PMDRAWELEMENTSY+60);
    ofLine(outPos,PMDRAWELEMENTSY+10,outPos,PMDRAWELEMENTSY+60);
	ofLine(inPos,PMDRAWELEMENTSY+60,outPos,PMDRAWELEMENTSY+60);
	// draw inPos triangle
	ofBeginShape();
		ofVertex(inPos,PMDRAWELEMENTSY+10);
		ofVertex(inPos+5,PMDRAWELEMENTSY+5);
		ofVertex(inPos,PMDRAWELEMENTSY);
	ofEndShape();
	// draw outPos triangle
	ofBeginShape();
		ofVertex(outPos,PMDRAWELEMENTSY+10);
		ofVertex(outPos-5,PMDRAWELEMENTSY+5);
		ofVertex(outPos,PMDRAWELEMENTSY);
	ofEndShape();
	
	ofDrawBitmapString("[ " + ofToString(inFrame),ofPoint(inPos+0,PMDRAWELEMENTSY+75));
	ofDrawBitmapString(ofToString(outFrame) + " ]" ,ofPoint(outPos-30,PMDRAWELEMENTSY+75));
	
	ofSetColor(255,255,255);
}

//------------------------------------------------------
float VideoHeader::getFps(){
    return fps;
}
//------------------------------------------------------
void VideoHeader::setFps(float fps){
    this->fps=fps;
}

//------------------------------------------------------

VideoFrame VideoHeader::getVideoFrame(int index)
{
	//buffer->lock();
		int indexFrame = CLAMP(index,0,buffer->size()-1);
		VideoFrame frame = buffer->getVideoFrame(indexFrame);
	//buffer->unlock();
	return frame;
}
		
//------------------------------------------------------
VideoFrame VideoHeader::getNextVideoFrame(){

	//buffer->lock();
		currentPos=getNextPosition();
		VideoFrame frame = buffer->getVideoFrame(currentPos);
	//buffer->unlock();
	return frame;
}

//------------------------------------------------------
int VideoHeader::getNextPosition(){
	// returns the real position in the buffer . calculate the next position in frames
    // from the beginning of the recording based on speed    
	// position expresses number of frames since start
	
	// calculate how much microseconds is a frame
	// if we're playing, speed has sense, if not not ?
	
	unsigned int buffer_size;
	unsigned int totalNumFr;
	unsigned int lastAbsFrame;
	int	inFrame;
	int outFrame;
	int	inAbsFrame;
	int	outAbsFrame;
	int backpos;
	int nextPos;
	
	switch (driveMode) 
	{
		case 0 :
			// normal mode, based on time
			
			if(playing) oneFrame=(TimeDiff)(1000000.0/fps/speed);
			else oneFrame=(TimeDiff)(1000000.0/fps/1.0);
			
			buffer_size=buffer->size();
			totalNumFr = buffer->getTotalFrames();
			lastAbsFrame = totalNumFr - buffer_size; 
			inFrame  = int(double(buffer_size-1)*(in));
			outFrame = int(double(buffer_size-1)*(out));
			inAbsFrame  = totalNumFr -  inFrame;
			outAbsFrame = totalNumFr - outFrame;
			
			//	printf("VIDEOVIDEOVIDEOVIDEO______________________________\n\n");
			//	printf("-------------------------------\nTOTAL : %d\nSIZE : %d \nLAST : %d\n IN : %d / %d \n OUT : %d / %d \nPOSITION : %f %d\n",
			//		   totalNumFr,
			//		   buffer_size,
			//		   lastAbsFrame,
			//		   inAbsFrame,totalNumFr-inAbsFrame,
			//		   outAbsFrame,totalNumFr-outAbsFrame,
			//		   position, int(position));
			//
			//	printf("VIDEOVIDEOVIDEOVIDEO______________________________\n\n");
			
			// if time spend since last positionTS.update() + portion to next frame is >= oneFrame
			// means that we need to update the position !!
			if((double)positionTS.elapsed()+(position-floor(position))*(double)abs(oneFrame)>=abs(oneFrame))
			{
				if(oneFrame!=0)
				{
					position=position + double(positionTS.elapsed())/(double)oneFrame;
				}
				// updates the time-stamp with the current time
				positionTS.update();		
			}
			
			// if header is playing and loopStart is requested, set position to inPoint or outPoint depending on speed's sign !
			if(playing && loopStart)
			{
				if(speed>0.0) position=double(inAbsFrame);
				else position=double(outAbsFrame);
				loopStart=false;
			}
			
			// if we're playing in loop and we're reaching the outpoint
			if(playing && (int(position) > (outAbsFrame)))
			{
				if(loopMode==OF_LOOP_NORMAL) position = double(inAbsFrame);
				else if (loopMode==OF_LOOP_NONE)
				{
					setPlaying(false);
				}
				else if (loopMode==OF_LOOP_PALINDROME) 
				{
					speed=-speed;
				}
			}
			// if we're in playing in loop and we're reaching the inpoint (while speed is negative probably)
			else if(playing && (int(position) < (inAbsFrame)))
			{
				if(loopMode==OF_LOOP_NORMAL) position = double(outAbsFrame);
				else if (loopMode==OF_LOOP_NONE) setPlaying(false);
				else if (loopMode==OF_LOOP_PALINDROME) 
				{
					speed=-speed;
				}
			}
			
			// clamp position to it's limits ...
			if(playing) position=CLAMP(position,double(inAbsFrame),double(outAbsFrame));
			else position=CLAMP(position,double(lastAbsFrame),double(totalNumFr));
			
			
			// backpos
			backpos=0;	
			if (!playing) backpos=0;
			else {
				backpos=int(buffer->getTotalFrames()-int(position));
				backpos=CLAMP(backpos,0,buffer_size-1);
			}
			
			nextPos;
			if (playing) nextPos= (buffer_size-1) - backpos;
			else		 nextPos= (buffer_size-1) - (delay/oneFrame);
			
			nextPos = CLAMP(nextPos,0,buffer_size-1);
			return nextPos;
		
			break;
			
		case 1 :
			// position driven by audio trough calls to delay !!
			oneFrame=(TimeDiff)(1000000.0/fps/1.0);
			buffer_size=buffer->size();
			nextPos= int(buffer_size-1) - int(double(delay)/double(oneFrame));
			nextPos = CLAMP(nextPos,0,buffer_size-1);
			return nextPos;
			
			break;
		default:
			return 0;
			break;
	}
}

//------------------------------------------------------
VideoBuffer *VideoHeader::getBuffer()
{
    return buffer;
}

//------------------------------------------------------
void VideoHeader::setSpeed(float speed)
{
	this->speed = speed;
}
//------------------------------------------------------
float VideoHeader::getSpeed()
{
    return speed;
}
//------------------------------------------------------
void VideoHeader::setOpacity(int opacity) 
{
	this->opacity = CLAMP (opacity,0,255);
}
//------------------------------------------------------
int VideoHeader::getOpacity() 
{
	return opacity;
}


	
	
	
//------------------------------------------------------
// get & set delay 
//------------------------------------------------------
int VideoHeader::getDelayMs() 
{
	return delay/1000;
}
//------------------------------------------------------
int VideoHeader::getDelayFrames() 
{
	return this->getDelayMs()/(TimeDiff)(1000.0/fps/1.0);
}
//------------------------------------------------------
float VideoHeader::getDelayPct()
{
	double res = this->getDelayFrames()/(buffer->size()-1);
	return res;
}
//------------------------------------------------------
void VideoHeader::setDelayMs(float delayMs)
{
	double oneFrame=(TimeDiff)(1000000.0/fps/1.0);
	int delayToSet = int(double(delayMs*1000.0));

	// control not out of bounds !! needs more precise control related to bufferMarkers !! (TO DO)
	if(delayToSet<0) delayToSet = 0;
	else if (delayToSet>int(double(buffer->getMaxSize()-1)*double(oneFrame))) delayToSet = int(double(buffer->getMaxSize()-1)*double(oneFrame));

	this->delay = delayToSet;
}
//------------------------------------------------------
void VideoHeader::setDelayFrames(int delayFrames)
{
	this->setDelayMs(int((double(delayFrames) / double(this->getFps()))*1000.0));
}
//------------------------------------------------------
void VideoHeader::setDelayPct(float pct)
{
	this->setDelayFrames(int(double(pct) * double(buffer->size())));
}

	
//------------------------------------------------------
// get & set in & out
//------------------------------------------------------
float VideoHeader::getIn()
{
	return in;
}
//------------------------------------------------------
void VideoHeader::setInMs(float in)
{
	// needs more precise control related to bufferMarkers !! (TO DO)

	double oneFrameMs=(TimeDiff)(1000000.0/fps/1.0);
	double fAux = double(in*1000.0f) / (oneFrameMs*double(buffer->size()));
	this->setInPct(CLAMP(fAux,0.0,1.0));    
	
	printf("vH :: in %d = pct %f\n",in,double(in*1000.0f) / (oneFrameMs*double(buffer->size())));
	//printf("ms to set %f translated to %f fps %f\n",in,double(in) / (oneFrameMs*double(buffer->size())),this->fps);
	//printf("VIDEO inMs %d :: %f \n",in,double(in) / (oneFrameMs*double(buffer->size())));
}
//------------------------------------------------------
void VideoHeader::setInPct(float in)
{
	//this->in=CLAMP(in,this->out,1.0);
	this->in=in;
}
//------------------------------------------------------
void VideoHeader::setInFrames(int in)
{
	double pct = double(in)/double(buffer->size());
	this->setInPct(pct);
}
	
//------------------------------------------------------
float VideoHeader::getOut()
{
	return out;
}
//------------------------------------------------------
void VideoHeader::setOutMs(float out)
{
	double oneFrameMs=(TimeDiff)(1000000.0/fps/1.0);
	double fAux = double(out*1000.0f) / (oneFrameMs*double(buffer->size()));
	this->setOutPct(CLAMP(fAux,0.0,1.0));    

}
//------------------------------------------------------
void VideoHeader::setOutPct(float out)
{
	this->out=CLAMP(out,0.0f,this->in);
	this->out=out;

}
//------------------------------------------------------
void VideoHeader::setOutFrames(int out)
{
	double pct = double(out)/double(buffer->size());
	this->setInPct(pct);
}

//------------------------------------------------------
// get & set loop & playing
//------------------------------------------------------
int VideoHeader::getLoopMode()
{
	return loopMode;
}
//------------------------------------------------------
void VideoHeader::setLoopMode(int loop)
{
    if(!((loop==OF_LOOP_NORMAL) || (loop==OF_LOOP_NONE) || (loop==OF_LOOP_PALINDROME)))
	{
		printf("ofxPlaymodes::VideoHeader:: Incorrect loop mode!\n");
	}
	else loopMode = loop;
}
	
//------------------------------------------------------
void VideoHeader::setLoopToStart()
{
	loopStart=true;
}
//------------------------------------------------------
bool VideoHeader::isPlaying() 
{
	return playing;
}
//------------------------------------------------------
void VideoHeader::setPlaying(bool isPlaying, float speed)
{
	this->speed = speed;
	if(isPlaying)
	{
		// if we're entering loop mode move position to in point
		// this behaviour is to sync entering loop mode with starting at inPoint or outPoint depending on speed
		this->playing = isPlaying;
		int	loopFrame;
		if(speed>0.0f) loopFrame = int(double(buffer->size()-1)*(in));
		else loopFrame = int(double(buffer->size()-1)*(out));
		int	inAbsFrame  = buffer->getTotalFrames() -  loopFrame;
		position = inAbsFrame; 
	}
	else
	{
		// if we're gettint out of loop mode move delay to actual position
		// this behaviour is to let the header (set by delay on no loop) where the loop was when deactivated
		// other beahaviour could be to let the header on delay / inPoint / outPoint position when loop is turned off
		this->playing = isPlaying;
		double	actualFrame  = double(buffer->getTotalFrames()-1) - (position);
		TimeDiff oneFrame=(TimeDiff)(1000000.0/fps/1.0);		
		delay = (actualFrame-1)*oneFrame;
	}
}		
//------------------------------------------------------
void VideoHeader::setPlaying(bool isPlaying)
{
	this->setPlaying(isPlaying,1.0);	
}
	

//------------------------------------------------------
void VideoHeader::receivedLoopEvent(int &i)
{
	setLoopToStart();
}	

//------------------------------------------------------
void VideoHeader::setDriveMode(int mode)
{
	driveMode = mode;
}

// Added functions.
//------------------------------------------------------
void VideoHeader::reset() {
    speed       = 1;
    in          = 1;
    out         = 0;
    playing   	= false;
    delay       = 0;
    opacity		= 255;
    loopStart	= false;
    loopMode	= OF_LOOP_NORMAL;
    driveMode	= 0;
    currentPos  = 0;
}


}

