#include <iostream>
#include "Leap.h"

using namespace Leap;
using namespace std;

class RubiksListener : public Listener{
public:
	virtual void onInit(const Controller&);
	virtual void onConnect(const Controller&);
	virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
};

void RubiksListener::onInit(const Controller& controller){
	cout << "Initialized" << endl;
}

void RubiksListener::onConnect(const Controller& controller){
	cout << "Connected" << endl;
	/*
	Insert gestures here
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	*/
}

void RubiksListener::onDisconnect(const Controller& controller){
	cout << "Disonnected" << endl;
}