#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "Resources.h"

#include "Leap.h"
#include <iostream>

using namespace ci;
using namespace ci::app;
using namespace Leap;
using namespace std;


// --------- CONSTANTS
static const short sizeCube = 3;
static const Vec3f cubeSize(300, 300, 300);
static const Vec3f cubeletSize(50, 50, 50);
static const Vec3f xOffset(100, 0, 0);
static const Vec3f yOffset(0, 100, 0);
static const Vec3f zOffset(0, 0, 100);

class Cubelet {
	Vec3f center, size;
public:
	Cubelet() {};
	Cubelet(const Vec3f& cntr, const Vec3f& sze) : center(cntr), size(sze) {};

	void operator()(const Vec3f&, const Vec3f&);

	void draw();
};

void Cubelet::operator()(const Vec3f& cntr, const Vec3f& diagonalSize) {
	this->center = cntr;
	this->size = diagonalSize;
}

void Cubelet::draw() {
	gl::drawStrokedCube(this->center, this->size);
}

class Cube {
	short nSides;
	Vec3f center, size;
	Cubelet main;
	Cubelet*** cubeletArray;
public:
	Cube() {};

	void operator()(const Vec3f&, const Vec3f&, const short&);

	void draw();
};

void Cube::operator()(const Vec3f& cntr, const Vec3f& totalSize, const short& sides) {
	this->center = cntr;
	this->size = totalSize;
	this->nSides = sides;

	cubeletArray = new Cubelet**[nSides];

	for (short i = -1; i < nSides-1; i++) { // -1, 0, 1 for xOffset scale
		cubeletArray[i+1] = new Cubelet*[nSides];

		for (short j = -1; j < nSides-1; j++) { // -1, 0, 1 for yOffset scale
			cubeletArray[i+1][j+1] = new Cubelet[nSides];

			for (short k = -1; k < nSides-1; k++) { // -1, 0, 1 for zOffset scale
				Vec3f cubeletPos = Vec3f::zero();
				cubeletPos += (i * xOffset);
				cubeletPos += (j * yOffset);
				cubeletPos += (k * zOffset);

				cubeletArray[i+1][j+1][k+1] = Cubelet(cubeletPos, cubeletSize);
			}
		}
	}

	this->main(this->center, this->size);
}

void Cube::draw() {
	for (short i = 0; i < this->nSides; i++) { // -1, 0, 1 for xOffset scale
		for (short j = 0; j < this->nSides; j++) { // -1, 0, 1 for yOffset scale
			for (short k = 0; k < this->nSides; k++) { // -1, 0, 1 for zOffset scale
				this->cubeletArray[i][j][k].draw();
			}
		}
	}
}

class LeapMotionListener : public Listener {
public:
	virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
};

void LeapMotionListener::onInit(const Controller& controller) {
	cout << "Initialized" << endl;
}

void LeapMotionListener::onConnect(const Controller& controller) {
	cout << "Connected" << endl;
}

void LeapMotionListener::onDisconnect(const Controller& controller) {
	cout << "Disconnected" << endl;
}

void LeapMotionListener::onExit(const Controller& controller) {
	cout << "Exited" << endl;
}

void LeapMotionListener::onFrame(const Controller& controller) {
}

void LeapMotionListener::onFocusGained(const Controller& controller) {
	cout << "Focus gained" << endl;
}

void LeapMotionListener::onFocusLost(const Controller& controller) {
	cout << "Focus lost" << endl;
}

class LeapMotionTestingApp : public AppNative {
	LeapMotionListener leapListener;
	Controller controller;
public:
	gl::Texture myImage;
	Cube rCube;

	void setup();
	void prepareSettings(Settings*);
	void mouseDown( MouseEvent event );	
	void keyDown( KeyEvent event );
	void update();
	void draw();
};

void LeapMotionTestingApp::setup() {
	myImage = gl::Texture( loadImage( loadResource(BLUE_FACE) ) );
	this->rCube(Vec3f::zero(), cubeSize, sizeCube);
}

void LeapMotionTestingApp::prepareSettings(Settings* settings) {
	// settings->setWindowSize(800, 600);
	settings->setFullScreen();
	settings->setFrameRate(60.0f);
}

void LeapMotionTestingApp::mouseDown( MouseEvent event ) {
}

void LeapMotionTestingApp::keyDown( KeyEvent event ) {
	if (event.getCode() == KeyEvent::KEY_ESCAPE) {
		quit();
	}
}

void LeapMotionTestingApp::update() {
	gl::translate( Vec3f(getWindowWidth()/2, getWindowHeight()/2, 0) );
	gl::rotate( Vec3f::yAxis() );
	gl::translate( Vec3f(-getWindowWidth()/2, -getWindowHeight()/2, 0) );
}

void LeapMotionTestingApp::draw() {
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
	// gl::draw( myImage );
}

CINDER_APP_NATIVE( LeapMotionTestingApp, RendererGl )
