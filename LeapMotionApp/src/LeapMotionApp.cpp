#include "cinder/app/AppNative.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Vector.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/gl/Vbo.h"

#include "Resources.h"
#include "Leap.h"

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

static const float roughPi = atan(1) * 4;
static const short nFramesPerRotation = 20;
static const float intendedRotation = roughPi / 2;
static const float rotationSpeed = intendedRotation / (float)nFramesPerRotation;

static const short nFramesPerSwipeRotation = 20;
static const float swipeRotationSpeed = intendedRotation / (float)nFramesPerSwipeRotation;

static const Vec3f cubeSize(cubeWidth, cubeWidth, cubeWidth);
static const Vec3f cubeletSize(cubeletWidth, cubeletWidth, cubeletWidth);
static const Vec3f xOffset(cubeletWidth, 0.0f, 0.0f);
static const Vec3f yOffset(0.0f, cubeletWidth, 0.0f);
static const Vec3f zOffset(0.0f, 0.0f, cubeletWidth);

float myRoundF(const float& num) {
	float result = 0;
	float floored = floorf(num);
	float remainder = num - floored;

	if (num == 0) {
		result = 0.0f;
	}
	else if (num > 0) {
		if (remainder >= 0.5f) {
			result = floored + 1.0f;
		}
		else {
			result = floored;
		}
	}
	else if (num < 0) {
		if (remainder >= 0.5f) {
			result = floored + 1.0f;
		}
		else {
			result = floored;
		}
	}
	return result;
}

Vec3f findClosestAxis(Vec3f direction) {
	Vec3f axis = direction;
	
	if (abs(axis.x) > abs(axis.y)) axis.y = 0;
	else axis.x = 0;
	
	if (abs(axis.x) > abs(axis.z)) axis.z = 0;
	else axis.x = 0;
	
	if (abs(axis.y) > abs(axis.z)) axis.z = 0;
	else axis.y = 0;
	
	axis.normalize();
	
	return axis;
}

class CubeletFace {
private:
	gl::VboMesh mesh;
	gl::TextureRef texture;

	static const short nCorners = 4;
	Vec3f oldPositions[nCorners];
public:
	CubeletFace() {};
	CubeletFace(const Vec3f*, gl::TextureRef);

	void draw();
	void rotate(const Vec3f&, const float&);
	void snapCorners();
};

CubeletFace::CubeletFace(const Vec3f* faceCorners, gl::TextureRef faceTexture) {
	short nVerts = 4;
	short nQuads = 1;

	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setDynamicPositions();
	layout.setStaticTexCoords2d();

	this->texture = faceTexture; // Store the texture
	this->mesh = gl::VboMesh(nVerts, nQuads * 4, layout, GL_QUADS);

	vector<uint32_t> indices;
	vector<Vec2f> texCoords;

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);
	texCoords.push_back( Vec2f(0.0f, 0.0f) );
	texCoords.push_back( Vec2f(0.0f, 1.0f) );
	texCoords.push_back( Vec2f(1.0f, 1.0f) );
	texCoords.push_back( Vec2f(1.0f, 0.0f) );

	this->mesh.bufferIndices(indices);
	this->mesh.bufferTexCoords2d( 0, texCoords );

	gl::VboMesh::VertexIter iter = this->mesh.mapVertexBuffer(); // Update the VBO with the vertex iter

	for (short i = 0; i < 4; i++) {
		iter.setPosition( faceCorners[i] ); // Initialize position
		this->oldPositions[i] = faceCorners[i];
		++iter;
	}
}

void CubeletFace::draw() {
	this->texture->enableAndBind();
	gl::draw(this->mesh);
}

void CubeletFace::rotate(const Vec3f& axis, const float& angle) {
	gl::VboMesh::VertexIter iter = this->mesh.mapVertexBuffer(); // Update the VBO with the vertex iter

	for (short i = 0; i < this->nCorners; i++) {
		Vec3f oldPos = this->oldPositions[i];
		Vec3f newPos = oldPos * Quatf(axis, angle); // Rotate using Quaternion multiplication

		this->oldPositions[i] = newPos;
		iter.setPosition( newPos ); // Update corner positions
		++iter;
	}
}

void CubeletFace::snapCorners() {
	gl::VboMesh::VertexIter iter = this->mesh.mapVertexBuffer();

	for (short i = 0; i < this->nCorners; i++) {
		this->oldPositions[i].x = myRoundF(this->oldPositions[i].x);
		this->oldPositions[i].y = myRoundF(this->oldPositions[i].y);
		this->oldPositions[i].z = myRoundF(this->oldPositions[i].z);

		iter.setPosition(this->oldPositions[i]);
		++iter;
	}
}

class Cubelet {
private:
	gl::TextureRef whiteFace, blueFace, redFace, greenFace, orangeFace, yellowFace;
	Vec3f center, size;
	CubeletFace* faceArray;
	gl::TextureRef* faceTexArray;
	static const short nFaces = 6;
public:
	Cubelet() {};
	Cubelet(const Vec3f&, const Vec3f&);

	Vec3f getCenter() { return this->center; };

	void operator()(const Vec3f&, const Vec3f&);

	void draw();
	void rotate(const Vec3f&, const float&);
	void snapPosition();
};

Cubelet::Cubelet(const Vec3f& cntr, const Vec3f& diagonalSize) {
	this->whiteFace = gl::Texture::create( loadImage( loadResource(WHITE_FACE) ) );
	this->orangeFace = gl::Texture::create( loadImage( loadResource(ORANGE_FACE) ) );
	this->greenFace = gl::Texture::create( loadImage( loadResource(GREEN_FACE) ) );
	this->redFace = gl::Texture::create( loadImage( loadResource(RED_FACE) ) );
	this->blueFace = gl::Texture::create( loadImage( loadResource(BLUE_FACE) ) );
	this->yellowFace = gl::Texture::create( loadImage( loadResource(YELLOW_FACE) ) );

	this->faceTexArray = new gl::TextureRef[6];
	// Ordered so the final showing is correct (according to Zevi)
	faceTexArray[0] = this->greenFace; // Left
	faceTexArray[1] = this->whiteFace; // Bottom
	faceTexArray[2] = this->redFace; // Back
	faceTexArray[3] = this->orangeFace; // Front
	faceTexArray[4] = this->yellowFace; // Top
	faceTexArray[5] = this->blueFace; // Right


	this->center = cntr;
	this->size = diagonalSize;
	Vec3f halfSize = this->size / 2;

	Vec3f cubeVertices[8]; // Creating vertices of cubelet (relative to center)
	short vertIndex = 0;
	for (short i = -1; i <= 1; i += 2) { // Note: Jumping by 2 -> -1 and 1
		for (short j = -1; j <= 1; j += 2) {
			for (short k = -1; k <= 1; k += 2) {
				cubeVertices[vertIndex++] = Vec3f(i*halfSize.x, j*halfSize.y, k*halfSize.z);
			}
		}
	}

	// Face 1:
	Vec3f tempVecArray[6][4] = { // Cubelet faces (constructed from relative positions and center)
		{
			this->center + cubeVertices[0], // Face 1
			this->center + cubeVertices[1],
			this->center + cubeVertices[3],
			this->center + cubeVertices[2]
		},
		{
			this->center + cubeVertices[0], // Face 2
			this->center + cubeVertices[1],
			this->center + cubeVertices[5],
			this->center + cubeVertices[4]
		},
		{
			this->center + cubeVertices[0], // Face 3
			this->center + cubeVertices[2],
			this->center + cubeVertices[6],
			this->center + cubeVertices[4]
		},
		{
			this->center + cubeVertices[1], // Face 4
			this->center + cubeVertices[3],
			this->center + cubeVertices[7],
			this->center + cubeVertices[5]
		},
		{
			this->center + cubeVertices[3], // Face 5
			this->center + cubeVertices[2],
			this->center + cubeVertices[6],
			this->center + cubeVertices[7]
		},
		{
			this->center + cubeVertices[4], // Face 6
			this->center + cubeVertices[5],
			this->center + cubeVertices[7],
			this->center + cubeVertices[6]
		}
	};

	this->faceArray = new CubeletFace[this->nFaces];

	for (short i = 0; i < this->nFaces; i++) {
		this->faceArray[i] = CubeletFace(tempVecArray[i], this->faceTexArray[i]);
	}
}

void Cubelet::draw() {
	//gl::drawStrokedCube(this->center, this->size);
	//gl::drawVector(Vec3f::zero(), this->center);
	for (short i = 0; i < this->nFaces; i++) {
		this->faceArray[i].draw();
	}
}

void Cubelet::rotate(const Vec3f& axis, const float& angle) {
	this->center = this->center * Quatf(axis, angle); // Rotate using Quaternion multiplication
	for (short i = 0; i < this->nFaces; i++) {
		this->faceArray[i].rotate(axis, angle);
	}
}

void Cubelet::snapPosition() {
	this->center.x = myRoundF(this->center.x);
	this->center.y = myRoundF(this->center.y);
	this->center.z = myRoundF(this->center.z);

	for (short i = 0; i < this->nFaces; i++) {
		this->faceArray[i].snapCorners();
	}
}

class Cube {
private:
	// Cube movement status variables
	bool midMovement;
	Vec3f motionAxis;
	short motionDirection;
	short nSlicesMoving;
	short nMovementsLeft;
	short nCubeletsMoving;
	short* turningIndices;
	short cooldown;

	// Cube information variables
	short nSides, nCubelets;
	Vec3f center, size;
	Cubelet* cubeletArray;
public:
	Cube() : midMovement(false), nMovementsLeft(nFramesPerRotation), cooldown(nFramesPerRotation) {};

	void operator()(const Vec3f&, const Vec3f&, const short&);

	void update();
	void draw() const;
	void rotateSlices(const short&, const Vec3f&, const short&);
	void endMovement();
};

void Cube::operator()(const Vec3f& cntr, const Vec3f& totalSize, const short& sides) {
	this->center = cntr;
	this->size = totalSize;
	this->nSides = sides;

	short maxNumCubelets = (short)pow(this->nSides, 3);
	this->turningIndices = new short[maxNumCubelets];

	cubeletArray = new Cubelet[maxNumCubelets];
	short offset = (nSides/2); // Offset so the range is half negative, half positive (i.e. [-1, 1] or [-8, 8])

	short numCubelets = 0;
	for (short i = 0; i < this->nSides; i++) {
		short iOff = i - offset;

		for (short j = 0; j < this->nSides; j++) {
			short jOff = j - offset;

			for (short k = 0; k < this->nSides; k++) {
				short kOff = k - offset;

				if (i == 0 || i == this->nSides-1 || j == 0 || j == this->nSides-1 || k == 0 || k == this->nSides-1) {
					Vec3f cubeletPos = Vec3f::zero();
					cubeletPos += (iOff * xOffset); // Offsets the cubelet by the xOffset constant vector, in a direction determined by i
					cubeletPos += (jOff * yOffset);
					cubeletPos += (kOff * zOffset);

					cubeletArray[numCubelets++] = Cubelet(cubeletPos, cubeletSize);
				}
			}
		}
	}

	this->nCubelets = numCubelets;
}

void Cube::draw() const {
	for (short i = 0; i < this->nCubelets; i++) {
		this->cubeletArray[i].draw();
	}
}

void Cube::update() {
	if (this->cooldown > 0) this->cooldown--;

	if (this->midMovement) {
		if (this->nMovementsLeft <= 0) {
			this->endMovement();
			this->cooldown = nFramesPerRotation;
		}
		else {
			for (short i = 0; i < this->nCubeletsMoving; i++) {
				this->cubeletArray[this->turningIndices[i]].rotate(this->motionAxis, this->motionDirection*rotationSpeed);
			}
			this->nMovementsLeft--;
		}
	}
}

void Cube::rotateSlices(const short& nSlicesTurning, const Vec3f& rotationAxis, const short& rotationDirection) {
	if (this->midMovement or this->cooldown) return;
	else this->midMovement = true;

	this->nSlicesMoving = nSlicesTurning;
	this->motionAxis = rotationAxis;
	this->motionDirection = rotationDirection;

	short nCubeletsTurning = (short)pow(this->nSides, 2) * this->nSlicesMoving;
	short* indiciesToTurn = new short[nCubeletsTurning];

	short index = 0;
	if (this->motionAxis == Vec3f::xAxis()) { // Turning around xAxis in positive
		for (short i = 0; i < this->nCubelets; i++) {
			if (this->cubeletArray[i].getCenter().x == cubeletWidth ) {
				indiciesToTurn[index++] = i;
			}
		}
	}
	else if (this->motionAxis == Vec3f::yAxis()) { // Turning around yAxis in positive
		for (short i = 0; i < this->nCubelets; i++) {
			if (this->cubeletArray[i].getCenter().y == cubeletWidth ) {
				indiciesToTurn[index++] = i;
			}
		}
	}
	else if (this->motionAxis == Vec3f::zAxis()) { // Turning around zAxis in positive
		for (short i = 0; i < this->nCubelets; i++) {
			if (this->cubeletArray[i].getCenter().z == cubeletWidth ) {
				indiciesToTurn[index++] = i;
			}
		}
	}
	else if (this->motionAxis == Vec3f::xAxis()*-1) { // Turning around xAxis in negative
		for (short i = 0; i < this->nCubelets; i++) {
			if (this->cubeletArray[i].getCenter().x*-1 == cubeletWidth ) {
				indiciesToTurn[index++] = i;
			}
		}
	}
	else if (this->motionAxis == Vec3f::yAxis()*-1) { // Turning around yAxis in negative
		for (short i = 0; i < this->nCubelets; i++) {
			if (this->cubeletArray[i].getCenter().y*-1 == cubeletWidth ) {
				indiciesToTurn[index++] = i;
			}
		}
	}
	else if (this->motionAxis == Vec3f::zAxis()*-1) { // Turning around zAxis in negative
		for (short i = 0; i < this->nCubelets; i++) {
			if (this->cubeletArray[i].getCenter().z*-1 == cubeletWidth ) {
				indiciesToTurn[index++] = i;
			}
		}
	}

	this->nCubeletsMoving = index;
	this->turningIndices = indiciesToTurn;
}

void Cube::endMovement() {
	this->midMovement = false;
	this->nMovementsLeft = nFramesPerRotation;

	for (short i = 0; i < this->nCubelets; i++) {
		this->cubeletArray[i].snapPosition();
	}
}

class LeapMotionListener : public Listener {
public:
    Vector* fingers;
	bool swipeGestureActive;
	Vector swipeDirection;
	bool circleGestureActive;
	Vector circleNormal;

	virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
};

void LeapMotionListener::onInit(const Controller& controller) {
	short numFingers = 4;
    this->fingers = new Vector[numFingers];
	
	this->circleGestureActive = false;
	this->swipeGestureActive = false;
	
	controller.enableGesture(Gesture::TYPE_SWIPE);
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	
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
	GestureList gestures = frame.gestures();
	
	for (int i = 0; i < gestures.count(); i++) {
		if (gestures[i].type() == Gesture::TYPE_CIRCLE) {
			this->circleGestureActive = true;
			CircleGesture cGesture = gestures[i];
			this->circleNormal = cGesture.normal();
		}
		else if (gestures[i].type() == Gesture::TYPE_SWIPE and !(this->circleGestureActive)) {
			this->swipeGestureActive = true;
			SwipeGesture sGesture = gestures[i];
			this->swipeDirection = sGesture.direction();
		}
		else {
			this->swipeGestureActive = false;
			this->circleGestureActive = false;
		}
	}
	
	if (gestures.count() == 0) {
		this->swipeGestureActive = false;
		this->circleGestureActive = false;
	}
}

void LeapMotionListener::onFocusGained(const Controller& controller) {
	console() << "Focus gained" << endl;
}

void LeapMotionListener::onFocusLost(const Controller& controller) {
	console() << "Focus lost" << endl;
}

class LeapCamera {
private:
public:
	bool currentlyAnimating;
	short nAnimationsLeft;
	CameraPersp camera;
	Vec3f animationRotationAxis;
	
	LeapCamera() {};
	
	void animateSwipeRotation(Vector);
	void update();
	
	void operator()(Vec3f, Vec3f, float, float, float, float);
};

void LeapCamera::operator()(Vec3f eyePoint, Vec3f interestPoint, float fieldOfView, float aspectRatio, float nearPlane, float farPlane) {
	this->camera.setEyePoint(eyePoint);
	this->camera.setCenterOfInterestPoint(interestPoint);
	this->camera.setPerspective(fieldOfView, aspectRatio, nearPlane, farPlane);
	this->currentlyAnimating = false;
}

void LeapCamera::animateSwipeRotation(Vector swipeDirection) {
	if (this->currentlyAnimating) {
//		console() << "Trying to animate during an animation, failed." << endl;
		return;
	}
	else {
//		console() << "Starting rotation with direction: " << swipeDirection << endl;
		this->currentlyAnimating = true;
		this->nAnimationsLeft = nFramesPerSwipeRotation;
	}
	
	Vec3f vec3fSwipeAxis = findClosestAxis(Vec3f(swipeDirection.x, swipeDirection.y, swipeDirection.z));
	
	this->animationRotationAxis = vec3fSwipeAxis.cross(Vec3f::zAxis());
	
//	console() << "Rotating towards " << vec3fSwipeAxis << " around " << this->animationRotationAxis	<< endl;
}

void LeapCamera::update() {
	if (!this->currentlyAnimating) {
		return;
	}
	
	Vec3f currentLocation = camera.getEyePoint();
	Vec3f currentFocus = camera.getCenterOfInterestPoint();
	
	Vec3f newLocation = currentLocation * Quatf(this->animationRotationAxis, swipeRotationSpeed);
	
	this->camera.setEyePoint(newLocation);
	this->camera.setCenterOfInterestPoint(currentFocus);
	
	this->nAnimationsLeft--;
	
	if (this->nAnimationsLeft <= 0) {
		this->currentlyAnimating = false;
	}
}

class LeapMotionApp : public AppNative {
private:
public:
	// Cube
	Cube rCube;
	gl::TextureFontRef mText;
	Font mFont;
	bool shiftPressed;

	// Leap
	LeapMotionListener leapListener;
	Controller leapController;

	// Camera
	LeapCamera staticCamera; // For leap's static displaying
	LeapCamera lCamera;
	
	// Cinder
	void setup();
	void prepareSettings(Settings*);
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
	void update();
	void draw();
	void resize();
};

void LeapMotionApp::setup() {
	// Initial settings
	short cameraSphereRadius = 800;
	Vec3f cubeStartPos = Vec3f::zero(); // Cube starts at 0
	
	Vec3f leapViewPoint(0.0f, 0.0f, 1.0f); // Leap's viewpoint is always from back in the z-direction
	leapViewPoint.normalize();
	leapViewPoint *= cameraSphereRadius;
	Vec3f viewStartPos(1.0f, 1.0f, 1.0f); // View starts at some value
	viewStartPos.normalize(); // Normalize to unit vector
	viewStartPos *= cameraSphereRadius; // 800 is the radius of sphere along which the camera travels
    float fieldOfView = 60.0f;
    float depthOfView = 3000.0f;

	// Setup calls
	this->leapController.addListener(leapListener);
	this->rCube(cubeStartPos, cubeSize, sizeCube);
	this->shiftPressed = false;
	
	mFont = Font("Arial", 22);
	mText = gl::TextureFont::create( mFont );
	
	this->staticCamera(viewStartPos, cubeStartPos, fieldOfView, getWindowAspectRatio(), 1.0f, depthOfView);
	this->lCamera(viewStartPos, cubeStartPos, fieldOfView, getWindowAspectRatio(), 1.0f, depthOfView);
}

void LeapMotionApp::prepareSettings(Settings* settings) {
	settings->setFullScreen();
	settings->setFrameRate(60.0f);
}

void LeapMotionApp::mouseDown( MouseEvent event ) {
}

void LeapMotionApp::mouseDrag( MouseEvent event ) {
}

void LeapMotionApp::keyDown( KeyEvent event ) {
	if (event.getCode() == KeyEvent::KEY_ESCAPE) {
		this->leapController.removeListener(this->leapListener);
		quit();
	}
	
	if (event.getCode() == KeyEvent::KEY_LSHIFT) {
		this->shiftPressed = true;
	}

	if (event.getCode() == KeyEvent::KEY_u && event.isShiftDown()) { // Up face turn
		this->rCube.rotateSlices(1, Vec3f::yAxis(), 1);
	}
	else if (event.getCode() == KeyEvent::KEY_u) {
		this->rCube.rotateSlices(1, Vec3f::yAxis(), -1);
	}
	else if (event.getCode() == KeyEvent::KEY_r && event.isShiftDown()) { // Right face turn
		this->rCube.rotateSlices(1, Vec3f::xAxis(), 1);
	}
	else if (event.getCode() == KeyEvent::KEY_r) {
		this->rCube.rotateSlices(1, Vec3f::xAxis(), -1);
	}
	else if (event.getCode() == KeyEvent::KEY_f && event.isShiftDown()) { // Front face turn
		this->rCube.rotateSlices(1, Vec3f::zAxis(), 1);
	}
	else if (event.getCode() == KeyEvent::KEY_f) {
		this->rCube.rotateSlices(1, Vec3f::zAxis(), -1);
	}
	else if (event.getCode() == KeyEvent::KEY_b && event.isShiftDown()) { // Back face turn
		this->rCube.rotateSlices(1, Vec3f::zAxis()*-1, 1);
	}
	else if (event.getCode() == KeyEvent::KEY_b) {
		this->rCube.rotateSlices(1, Vec3f::zAxis()*-1, -1);
	}
	else if (event.getCode() == KeyEvent::KEY_l && event.isShiftDown()) { // Left face turn
		this->rCube.rotateSlices(1, Vec3f::xAxis()*-1, 1);
	}
	else if (event.getCode() == KeyEvent::KEY_l) {
		this->rCube.rotateSlices(1, Vec3f::xAxis()*-1, -1);
	}
	else if (event.getCode() == KeyEvent::KEY_d && event.isShiftDown()) { // Down face turn
		this->rCube.rotateSlices(1, Vec3f::yAxis()*-1, 1);
	}
	else if (event.getCode() == KeyEvent::KEY_d) {
		this->rCube.rotateSlices(1, Vec3f::yAxis()*-1, -1);
	}
}

void LeapMotionApp::keyUp( KeyEvent event ) {
	if (event.getCode() == KeyEvent::KEY_LSHIFT) {
		this->shiftPressed = false;
	}
}

void LeapMotionApp::update() {
	if (this->leapListener.swipeGestureActive) {
		this->lCamera.animateSwipeRotation(this->leapListener.swipeDirection);
	}
	if (this->leapListener.circleGestureActive) {
		Vector& normal = this->leapListener.circleNormal;
		Vec3f rotationAxis = findClosestAxis(Vec3f(normal.x, normal.y, normal.z));
		short direction = 1;
		
		if (rotationAxis.x == -1 or rotationAxis.y == -1 or rotationAxis.z == -1) { // If the axis is negative, make it positive but also invert rotation
			rotationAxis *= -1;
			direction *= -1;
		}
		if (this->shiftPressed) { // If shift if pressed, invert axis
			rotationAxis *= -1;
		}
		
		this->rCube.rotateSlices(1, rotationAxis, direction);
	}
	
	this->rCube.update();
	this->lCamera.update();
}

void LeapMotionApp::draw() {
	// Clear to black
	gl::clear( Color( 255, 255, 255 ), true );

	// enable the depth buffer (after all, we are doing 3D)
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	// Update the view from Leap's camera
//	gl::setMatrices( this->staticCamera.camera );
	
	// Update the view from camera
	gl::setMatrices( this->lCamera.camera );
	// Render the cube
	this->rCube.draw();
}

void LeapMotionApp::resize() {
	lCamera.camera.setAspectRatio( getWindowAspectRatio() );
}

CINDER_APP_NATIVE( LeapMotionApp, RendererGl )
