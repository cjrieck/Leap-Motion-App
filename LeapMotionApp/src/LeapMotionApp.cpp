#include "cinder/app/AppNative.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Vector.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
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

static const float rotationSpeed = .05f;

static const Vec3f cubeSize(cubeWidth, cubeWidth, cubeWidth);
static const Vec3f cubeletSize(cubeletWidth, cubeletWidth, cubeletWidth);
static const Vec3f xOffset(cubeletWidth, 0.0f, 0.0f);
static const Vec3f yOffset(0.0f, cubeletWidth, 0.0f);
static const Vec3f zOffset(0.0f, 0.0f, cubeletWidth);
static const Vec3f rotatedCloseEnough(3.0f, 3.0f, 3.0f);

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
	//this->texture->disable();
}

void CubeletFace::rotate(const Vec3f& axis, const float& angle) {
	gl::VboMesh::VertexIter iter = this->mesh.mapVertexBuffer(); // Update the VBO with the vertex iter

	for (short i = 0; i < 4; i++) {
		Vec3f oldPos = this->oldPositions[i];
		Vec3f newPos = oldPos * Quatf(axis, angle); // Rotate using Quaternion multiplication

		this->oldPositions[i] = newPos;
		iter.setPosition( newPos ); // Update corner positions
		++iter;
	}
}

class Cubelet {
private:
	gl::TextureRef whiteFace, blueFace, redFace, greenFace, orangeFace, yellowFace;
	Vec3f center, size;
	CubeletFace* faceArray;
	static const short nFaces = 6;
public:
	Cubelet() {};
	Cubelet(const Vec3f&, const Vec3f&);

	Vec3f getCenter() { return this->center; };

	void operator()(const Vec3f&, const Vec3f&);

	void draw();
	void rotate(const Vec3f&, const float&);
};

Cubelet::Cubelet(const Vec3f& cntr, const Vec3f& diagonalSize) {
	this->whiteFace = gl::Texture::create( loadImage( loadResource(BLUE_FACE) ) );
	this->blueFace = gl::Texture::create( loadImage( loadResource(BLUE_FACE) ) );
	this->redFace = gl::Texture::create( loadImage( loadResource(BLUE_FACE) ) );
	this->greenFace = gl::Texture::create( loadImage( loadResource(BLUE_FACE) ) );
	this->orangeFace = gl::Texture::create( loadImage( loadResource(BLUE_FACE) ) );
	this->yellowFace = gl::Texture::create( loadImage( loadResource(BLUE_FACE) ) );


	this->center = cntr;
	this->size = diagonalSize;
	Vec3f halfSize = this->size / 2;

	Vec3f cubeVertices[8];
	short vertIndex = 0;
	for (short i = -1; i <= 1; i += 2) { // Note: Jumping by 2 -> -1 and 1
		for (short j = -1; j <= 1; j += 2) {
			for (short k = -1; k <= 1; k += 2) {
				cubeVertices[vertIndex++] = Vec3f(i*halfSize.x, j*halfSize.y, k*halfSize.z);
			}
		}
	}

	// Face 1:
	Vec3f tempVecArray[this->nFaces][4] = {
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
		this->faceArray[i] = CubeletFace(tempVecArray[i], this->blueFace);
	}
}

void Cubelet::draw() {
	//gl::drawStrokedCube(this->center, this->size);
	gl::drawVector(Vec3f::zero(), this->center);
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

class Cube {
private:
	bool midMovement;
	Vec3f motionAxis;
	short motionDirection;
	short nSlicesMoving;
	short nMovementsMade;

	short nSides, nCubelets;
	Vec3f center, size;
	Cubelet* cubeletArray;
public:
	Cube() : midMovement(false), nMovementsMade(0) {};

	void operator()(const Vec3f&, const Vec3f&, const short&);

	void update();
	void draw() const;
	void rotateSlices(const short&, const Vec3f&, const short&);
};

void Cube::operator()(const Vec3f& cntr, const Vec3f& totalSize, const short& sides) {
	this->center = cntr;
	this->size = totalSize;
	this->nSides = sides;

	this->nCubelets = (short)pow(this->nSides, 3);
	cubeletArray = new Cubelet[this->nCubelets];
	short offset = (nSides/2); // Offset so the range is half negative, half positive (i.e. [-1, 1] or [-8, 8])

	short numCubelets = 0;
	for (short i = 0; i < nSides; i++) {
		short iOff = i - offset;

		for (short j = 0; j < nSides; j++) {
			short jOff = j - offset;

			for (short k = 0; k < nSides; k++) {
				short kOff = k - offset;
				Vec3f cubeletPos = Vec3f::zero();
				cubeletPos += (iOff * xOffset); // Offsets the cubelet by the xOffset constant vector, in a direction determined by i
				cubeletPos += (jOff * yOffset);
				cubeletPos += (kOff * zOffset);

				cubeletArray[numCubelets++] = Cubelet(cubeletPos, cubeletSize);
			}
		}
	}
}

void Cube::draw() const {
	for (short i = 0; i < this->nCubelets; i++) {
		this->cubeletArray[i].draw();
	}
}

void Cube::update() {
	if (!this->midMovement) return;

	if (this->nMovementsMade > 100) {
		this->nMovementsMade = 0;
		this->midMovement = false;
		return; 
	}
	else {
		this->nMovementsMade++;
	}

	short nCubeletsTurning = (short)pow(this->nSides, 2) * this->nSlicesMoving;
	short* indiciesToTurn = new short[nCubeletsTurning];

	short index = 0;
	if (this->motionAxis == Vec3f::xAxis()) {
		for (short i = 0; i < this->nCubelets; i++) {
			if (this->cubeletArray[i].getCenter().x == cubeletWidth ) {
				indiciesToTurn[index++] = i;
			}
		}
	}
	else if (this->motionAxis == Vec3f::yAxis()) {
		for (short i = 0; i < this->nCubelets; i++) {
			if (this->cubeletArray[i].getCenter().y == cubeletWidth ) {
				indiciesToTurn[index++] = i;
			}
		}
	}
	else if (this->motionAxis == Vec3f::zAxis()) {
		for (short i = 0; i < this->nCubelets; i++) {
			if (this->cubeletArray[i].getCenter().z == cubeletWidth ) {
				indiciesToTurn[index++] = i;
			}
		}
	}

	for (short i = 0; i < nCubeletsTurning; i++) {
		this->cubeletArray[indiciesToTurn[i]].rotate(this->motionAxis, this->motionDirection*rotationSpeed);
	}
}

void Cube::rotateSlices(const short& nSlicesTurning, const Vec3f& rotationAxis, const short& rotationDirection) {
	if (this->midMovement) return;
	else this->midMovement = true;

	this->nSlicesMoving = nSlicesTurning;
	this->motionAxis = rotationAxis;
	this->motionDirection = rotationDirection;
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
private:
public:
	// Cube
	Cube rCube;
	gl::VboMeshRef mesh;

	static const int VERTICES_X = 20, VERTICES_Z = 20;

	gl::VboMeshRef	mVboMesh, mVboMesh2;
	gl::TextureRef	mTexture;
	CameraPersp		mCamera;

	// Leap
	LeapMotionListener leapListener;
	Controller leapController;

	// Cinder
	Vec2i mMousePos;
	MayaCamUI mMayaCam; // Have to include "MayaCamUI.h"
	CameraPersp mCameraPersp; // Have to include "CameraPersp.h"?

	void setup();
	void prepareSettings(Settings*);
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
	void update();
	void draw();
	void resize();
};

void LeapMotionApp::setup() {
	// Initial settings
	Vec3f viewStartPos(800.0f, 0.0f, 0.0f); // View starts back at 800
	Vec3f cubeStartPos = Vec3f::zero(); // Cube starts at 0

	// Setup calls

	this->leapController.addListener(leapListener);

	this->rCube(cubeStartPos, cubeSize, sizeCube);

	CameraPersp camera;
	camera.setEyePoint( viewStartPos );
	camera.setCenterOfInterestPoint( cubeStartPos );
	camera.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 2000.0f );
	mMayaCam.setCurrentCam( camera );
	//mVboMesh->bufferTexCoords2d( 0, texCoords );

	//// setup the parameters of the Vbo
	//int totalVertices = VERTICES_X * VERTICES_Z;
	//int totalQuads = ( VERTICES_X - 1 ) * ( VERTICES_Z - 1 );
	//gl::VboMesh::Layout layout;
	//layout.setStaticIndices();
	//layout.setDynamicPositions();
	//layout.setStaticTexCoords2d();
	//mVboMesh = gl::VboMesh::create( totalVertices, totalQuads * 4, layout, GL_QUADS );
	//
	//// buffer our static data - the texcoords and the indices
	//vector<uint32_t> indices;
	//vector<Vec2f> texCoords;
	//for( int x = 0; x < VERTICES_X; ++x ) {
	//	for( int z = 0; z < VERTICES_Z; ++z ) {
	//		// create a quad for each vertex, except for along the bottom and right edges
	//		if( ( x + 1 < VERTICES_X ) && ( z + 1 < VERTICES_Z ) ) {
	//			indices.push_back( (x+0) * VERTICES_Z + (z+0) );
	//			indices.push_back( (x+1) * VERTICES_Z + (z+0) );
	//			indices.push_back( (x+1) * VERTICES_Z + (z+1) );
	//			indices.push_back( (x+0) * VERTICES_Z + (z+1) );
	//		}
	//		// the texture coordinates are mapped to [0,1.0)
	//		texCoords.push_back( Vec2f( x / (float)VERTICES_X, z / (float)VERTICES_Z ) );
	//	}
	//}
	//
	//mVboMesh->bufferIndices( indices );
	//mVboMesh->bufferTexCoords2d( 0, texCoords );
	//
	//// make a second Vbo that uses the statics from the first
	//mVboMesh2 = gl::VboMesh::create( totalVertices, totalQuads * 4, mVboMesh->getLayout(), GL_QUADS, &mVboMesh->getIndexVbo(), &mVboMesh->getStaticVbo(), NULL );
	//mVboMesh2->setTexCoordOffset( 0, mVboMesh->getTexCoordOffset( 0 ) );
}

void LeapMotionApp::prepareSettings(Settings* settings) {
	//settings->setWindowSize(800, 600);
	settings->setFullScreen();
	settings->setFrameRate(60.0f);
}

void LeapMotionApp::mouseDown( MouseEvent event ) {
	mMayaCam.mouseDown( event.getPos() );
}

void LeapMotionApp::mouseDrag( MouseEvent event ) {
	// Store position of mouse
	mMousePos = event.getPos();

	// Let MayaCam handle the mouse dragging camera changes
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void LeapMotionApp::keyDown( KeyEvent event ) {
	if (event.getCode() == KeyEvent::KEY_ESCAPE) {
		quit();
	}
	if (event.getCode() == KeyEvent::KEY_a) {
		this->rCube.rotateSlices(1, Vec3f::yAxis(), 1);
	}
	if (event.getCode() == KeyEvent::KEY_s) {
		this->rCube.rotateSlices(1, Vec3f::yAxis(), -1);
	}
	if (event.getCode() == KeyEvent::KEY_d) {
		this->rCube.rotateSlices(1, Vec3f::xAxis(), 1);
	}
}

void LeapMotionApp::update() {
	// set up the camera 
	gl::setMatrices( mMayaCam.getCamera() );

	this->rCube.update();
}

void LeapMotionApp::draw() {
	// Clear to black
	gl::clear( Color( 0, 0, 0 ), true );

	// enable the depth buffer (after all, we are doing 3D)
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Render the cube
	this->rCube.draw();
}

void LeapMotionApp::resize() {
	CameraPersp camera = mMayaCam.getCamera();
	camera.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( camera );
}

CINDER_APP_NATIVE( LeapMotionApp, RendererGl )
