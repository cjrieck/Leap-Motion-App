import Leap
import sys

class MyListener(Leap.Listener):
	# NOTE: No __init__ function. Listener object instantiated on inherited Leap.Listener object

	def on_init(self, controller):
		print "Initialized"

	def on_connect(self, controller):
		print "Connected"

		# can enable gestures here using the enable_gesture function
		# Example:
		# controller.enable_gesture(Leap.Gesture.TYPE_SCREEN_TAP) # Enables a screen tap gesture

	def on_disconnect(self, controller):
		# Called when tracking is stopped
		print "Disconnected"

	def on_exit(self, controller):
		# Called when program stopped
		print "Exited"

	def on_frame(self, controller):

		# Instantiate a Frame object
		# This has information on if any hands are present within the controller's vision
		frame = controller.frame()

		if not frame.hands.is_empty: # If there is something within the controller's vision

			# Get the first hand
			# NOTE: The Frame object stores the amount of hand objects (extends Interface) in a list
			hand = frame.hands[0]

			# Get fingers on hand
			# NOTE: Each hand object has its own list of finger objects (extends Pointable)
			fingers = hand.fingers

			if not hand.fingers.is_empty: # Check if any fingers present
				
				# Check the length of the list of fingers
				if len(fingers) == 1:
					print "1 - Hello World"
				elif len(fingers) == 2:
					print "2 - Hello Worlds"
				elif len(fingers) == 3:
					print "3 - Hello Worlds"
				elif len(fingers) == 4:
					print "4 - Hello Worlds"
				else:
					print "5 - Hello Worlds"
				
		# if gestures were enabled, should also check if any gestures are present in the frame as well
		# can check gestures with frame.gestures().empty -> returns list of gestures present in Frame object
		if frame.hands.is_empty:
			print "No fingers"

def main():

	controller = Leap.Controller()

	listener = MyListener()
	
	controller.add_listener(listener)

	print "Press enter to quit..."
	sys.stdin.readline() # Continue to run until you hit a key

	controller.remove_listener(listener) # remove listener when program not running (good practice)

if __name__ == '__main__':
	main()