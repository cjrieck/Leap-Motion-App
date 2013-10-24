#include <iostream>
#include "Leap.h"

using namespace std;
using namespace Leap;


class RubiksDataStore : public Listener {

public:
	bool twoHands;
	Vector rotationDirection;
	
	/*
	Store rotation vectors (to translate to cube later)
	*/

	virtual void onInit(const Controller&);
	virtual void onConnect(const Controller&);
	virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);

    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
};

void RubiksDataStore::onInit(const Controller& controller) {
	cout << "Initialized" << endl;
}

void RubiksDataStore::onConnect(const Controller& controller) {
	cout << "Connected" << endl;
	/*
	Insert gestures here
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	*/
	controller.enableGesture(Gesture::TYPE_SWIPE);
}

void RubiksDataStore::onDisconnect(const Controller& controller) {
	cout << "Disonnected" << endl;
}

void RubiksDataStore::onFrame(const Controller& controller){

	const Frame frame = controller.frame();
	const GestureList gestures = frame.gestures();

	HandList hands = frame.hands();

	Hand leftHand = hands.leftmost(); // always track one hand

	for (int i = 0; i < gestures.count(); ++i)
	{
		Gesture activeGesture = gestures[i];

		switch (activeGesture.type()){
			case Gesture::TYPE_SWIPE:
			{
				SwipeGesture swipe = activeGesture;

				this->rotationDirection = swipe.direction();
				// Vector rotationSpeed = swipe.speed(); // if we want to implement fast rotations later on
				
				/*
				Do an entire cube rotation in here
				*/

				break;
			}
			case Gesture::TYPE_KEY_TAP:
			{
				break;
			}
			case Gesture::TYPE_SCREEN_TAP:
			{
				break;
			}
			case Gesture::TYPE_CIRCLE:
			{
				break;
			}

			default:
				break;
		}
	}

	if (hands.count() == 2){
		this->twoHands = true; // there are 2 hands in the view of the Leap Motion
		Hand rightHand = hands.rightmost(); // now set the rightmost hand
	}



	/*
	What we need to achieve:
		check for a swipe gesture
			rotate the cube in the direction of the swipe
			-> see if we can get an upward and downward swipe tracking going

		check for one "stationary" hand
		then track the other rotating hand while rotating the cube accordingly
			(we'll use hand pitch, roll and yaw to achieve a good rotation tracking)


	*/
}

void RubiksDataStore::onFocusGained(const Controller& controller){
	cout << "Focus gained" << endl;
}

void RubiksDataStore::onFocusLost(const Controller& controller){
	cout << "Focus Lost" << endl;
}

int main(){

	  // Create a sample listener and controller
	  RubiksDataStore listener;
	  Controller controller;

	  // Have the sample listener receive events from the controller
	  controller.addListener(listener);

	  // Keep this process running until Enter is pressed
	  std::cout << "Press Enter to quit..." << std::endl;
	  std::cin.get();

	  // Remove the sample listener when done
	  controller.removeListener(listener);

	return 0;
}