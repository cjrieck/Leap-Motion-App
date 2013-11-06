#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "Resources.h"

#include "../../leap/Leap.h"
#include <iostream>
#include <cmath>

using namespace ci;
using namespace ci::app;
using namespace Leap;
using namespace std;


// --------- CONSTANTS
static const short sizeCube = 3;
static const short cubeletWidth = 100;
static const short cubeWidth = cubeletWidth*3;
static const Vec3f cubeSize(cubeWidth, cubeWidth, cubeWidth);
static const Vec3f cubeletSize(cubeletWidth, cubeletWidth, cubeletWidth);
static const Vec3f xOffset(cubeletWidth, 0, 0);
static const Vec3f yOffset(0, cubeletWidth, 0);
static const Vec3f zOffset(0, 0, cubeletWidth);

class Cubelet {
	Vec3f center, size;
public:
	Cubelet() {};
	Cubelet(const Vec3f& cntr, const Vec3f& sze) : center(cntr), size(sze) {};
    
	Vec3f getCenter() { return this->center; };
    
	void operator()(const Vec3f&, const Vec3f&);
    
	void draw();
	void rotate(const Vec3f&, const float&);
};

void Cubelet::operator()(const Vec3f& cntr, const Vec3f& diagonalSize) {
	this->center = cntr;
	this->size = diagonalSize;
}

void Cubelet::draw() {
	gl::drawStrokedCube(this->center, this->size);
}

void Cubelet::rotate(const Vec3f& axis, const float& angle) {
	//this->center.rotate(axis, angle);
	this->center = this->center * Quatf(axis, angle);
}

class Cube {
	short nSides, nCubelets;
	Vec3f center, size;
	Cubelet main;
	Cubelet* cubeletArray;
public:
	Cube() {};
    
	void operator()(const Vec3f&, const Vec3f&, const short&);
    
	void draw() const;
	void rotateSlices(const short&, const Vec3f&, const float&);
};

void Cube::operator()(const Vec3f& cntr, const Vec3f& totalSize, const short& sides) {
	this->center = cntr;
	this->size = totalSize;
	this->nSides = sides;
    
	//cubeletArray = new Cubelet**[nSides];
	this->nCubelets = pow(this->nSides, 3);
	cubeletArray = new Cubelet[this->nCubelets];
	short negativeOffset = -(nSides/2);
    
	short numCubelets = 0;
	for (short i = 0; i < nSides; i++) { // -1, 0, 1 for xOffset scale
		//cubeletArray[i] = new Cubelet*[nSides];
		short iOff = i + negativeOffset;
        
		for (short j = 0; j < nSides; j++) { // -1, 0, 1 for yOffset scale
			//cubeletArray[i][j] = new Cubelet[nSides];
			short jOff = j + negativeOffset;
            
			for (short k = 0; k < nSides; k++) { // -1, 0, 1 for zOffset scale
				short kOff = k + negativeOffset;
				Vec3f cubeletPos = Vec3f::zero();
				cubeletPos += (iOff * xOffset); // Offsets the cubelet by the xOffset constant vector, in a direction determined by i
				cubeletPos += (jOff * yOffset);
				cubeletPos += (kOff * zOffset);
                
				cubeletArray[numCubelets] = Cubelet(cubeletPos, cubeletSize);
				numCubelets++;
			}
		}
	}
}

void Cube::draw() const {
	//for (short i = 0; i < this->nSides; i++) { // -1, 0, 1 for xOffset scale
	//	for (short j = 0; j < this->nSides; j++) { // -1, 0, 1 for yOffset scale
	//		for (short k = 0; k < this->nSides; k++) { // -1, 0, 1 for zOffset scale
	//			this->cubeletArray[i][j][k].draw();
	//		}
	//	}
	//}
	for (short i = 0; i < this->nCubelets; i++) {
		this->cubeletArray[i].draw();
	}
}

void Cube::rotateSlices(const short& nSlicesTurning, const Vec3f& rotationAxis, const float& rotationDirection) {
	short nCubeletsTurning = pow(this->nSides, 2) * nSlicesTurning;
	short* indiciesToTurn = new short[nCubeletsTurning];
    
	short index = 0;
	if (nSlicesTurning == 1) {
		if (rotationAxis == Vec3f::xAxis()) {
			for (short i = 0; i < this->nCubelets; i++) {
				if (this->cubeletArray[i].getCenter().x == xOffset.x ) {
					indiciesToTurn[index++] = i;
				}
			}
		}
		else if (rotationAxis == Vec3f::yAxis()) {
			for (short i = 0; i < this->nCubelets; i++) {
				if (this->cubeletArray[i].getCenter().y == yOffset.y ) {
					indiciesToTurn[index++] = i;
				}
			}
		}
		else if (rotationAxis == Vec3f::zAxis()) {
			for (short i = 0; i < this->nCubelets; i++) {
				if (this->cubeletArray[i].getCenter().z == zOffset.z ) {
					indiciesToTurn[index++] = i;
				}
			}
		}
	}
    
	for (short i = 0; i < nCubeletsTurning; i++) {
		console() << this->cubeletArray[indiciesToTurn[i]].getCenter() << endl;
		this->cubeletArray[indiciesToTurn[i]].rotate(rotationAxis, rotationDirection);
        //	Cubelet& cubeletTurning = this->cubeletArray[indiciesToTurn[i]];
        //	cubeletTurning.rotate(rotationAxis, rotationDirection);
	}
	console() << endl << endl << endl;
	//this->cubeletArray[26].rotate(rotationAxis, rotationDirection);
}

class LeapMotionListener : public Listener {
public:
	float* pointsList;
    
	virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
};

void LeapMotionListener::onInit(const Controller& controller) {
	short lenData = 3;
	//delete[] this->pointsList;
	this->pointsList = new float[lenData];
	for (short i = 0; i < lenData; i++) {
		this->pointsList[i] = 0;
	}
	console() << "Initialized" << endl;
}

void LeapMotionListener::onConnect(const Controller& controller) {
	console() << "Connected" << endl;
}

void LeapMotionListener::onDisconnect(const Controller& controller) {
	console() << "Disconnected" << endl;
}

void LeapMotionListener::onExit(const Controller& controller) {
	console() << "Exited" << endl;
}

void LeapMotionListener::onFrame(const Controller& controller) {
	Frame frame = controller.frame();
	InteractionBox intBox = frame.interactionBox();
    
	Finger finger = frame.fingers().frontmost();
	Vector stabilizedPoint = finger.stabilizedTipPosition();
    
	Vector normalizedPoint = intBox.normalizePoint(stabilizedPoint) * 2;
    
	this->pointsList[0] = normalizedPoint.x - 1;
	this->pointsList[1] = normalizedPoint.y - 1;
	this->pointsList[2] = normalizedPoint.z - 1;
}

void LeapMotionListener::onFocusGained(const Controller& controller) {
	console() << "Focus gained" << endl;
}

void LeapMotionListener::onFocusLost(const Controller& controller) {
	console() << "Focus lost" << endl;
}

class LeapMotionApp : public AppNative {
	// Cube
	Cube rCube;
	gl::Texture blueFace;
    
	// Leap
	LeapMotionListener leapListener;
	Controller leapController;
public:
	// Cinder
	void setup();
	void prepareSettings(Settings*);
	void mouseDown( MouseEvent event );
	void keyDown( KeyEvent event );
	void update();
	void draw();
};

void LeapMotionApp::setup() {
	blueFace = gl::Texture( loadImage( loadResource("BlueFace.PNG") ) );
	this->leapController.addListener(leapListener);
	this->rCube(Vec3f::zero(), cubeSize, sizeCube);
}

void LeapMotionApp::prepareSettings(Settings* settings) {
	//settings->setWindowSize(800, 600);
	settings->setFullScreen();
	settings->setFrameRate(60.0f);
}

void LeapMotionApp::mouseDown( MouseEvent event ) {
}

void LeapMotionApp::keyDown( KeyEvent event ) {
	if (event.getCode() == KeyEvent::KEY_ESCAPE) {
		quit();
	}
}

void LeapMotionApp::update() {
	gl::translate( Vec3f(getWindowWidth()/2, getWindowHeight()/2, 0) ); // Shift over for coordinate axis rotations
	this->rCube.rotateSlices(1, Vec3f::zAxis(), .05);
	//this->rCube.rotateSlice(nSlices, axis, direction);
	gl::rotate( Vec3f::yAxis() );
	//gl::rotate( Vec3f::yAxis() * this->leapListener.pointsList[0] );
	//gl::rotate( Vec3f::xAxis() * this->leapListener.pointsList[1] );
	//gl::rotate( Vec3f::zAxis() * this->leapListener.pointsList[2] );
	gl::translate( Vec3f(-getWindowWidth()/2, -getWindowHeight()/2, 0) );
}

void LeapMotionApp::draw() {
	gl::translate( Vec3f(getWindowWidth()/2, getWindowHeight()/2, 0) );
    
	gl::clear( Color( 0, 0, 0 ), true );
	this->rCube.draw();
    
	gl::translate( Vec3f(-getWindowWidth()/2, -getWindowHeight()/2, 0) );
    
    
	// float gray = sin( getElapsedSeconds()*2.5 ) * 0.5f + 0.5f;
    
	// float x = cos( getElapsedSeconds() ) * 100.0f;
	// float y = sin( getElapsedSeconds() ) * 100.0f;
	// gl::drawSolidRect( Rectf(1.0f, 1.0f, 400.0f, 400.0f));
	// gl::rotate( Vec3f(0, 1, 0) );
    
	//gl::drawCube( Vec3f::zero(), cubeletSize );
	//gl::drawCube( Vec3f::zero() + yOffset, cubeletSize );
	// gl::drawSolidCircle( Vec2f( x, y ) + getWindowSize() / 2, abs(x) );
	// gl::draw( blueFace );
}

CINDER_APP_NATIVE( LeapMotionApp, RendererGl )
