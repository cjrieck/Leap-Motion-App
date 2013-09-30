################################################################################
# Copyright (C) 2012-2013 Leap Motion, Inc. All rights reserved.               #
# Leap Motion proprietary and confidential. Not for distribution.              #
# Use subject to the terms of the Leap Motion SDK Agreement available at       #
# https://developer.leapmotion.com/sdk_agreement, or another agreement         #
# between Leap Motion and you, your company or other organization.             #
################################################################################

import Leap, sys, pygame
from Leap import CircleGesture, KeyTapGesture, ScreenTapGesture, SwipeGesture

pygame.init()

available_resolutions = pygame.display.list_modes()
# WIDTH = available_resolutions[0][0]
# HEIGHT = available_resolutions[0][1]
WIDTH = 800
HEIGHT = 400
SIZE = WIDTH, HEIGHT

class SampleListener(Leap.Listener):
	def on_init(self, controller):
		image_width = 50
		image_height = 50

		ball_image = pygame.transform.scale(pygame.image.load("bouncyball.png"), (image_width, image_height))
		self.ball = Ball(ball_image, (image_width, image_height), (WIDTH//2, 0), SIZE)

		self.screen = pygame.display.set_mode(SIZE)
		
		print "Initialized"

	def on_connect(self, controller):
		print "Connected"

		# Enable gestures
		# controller.enable_gesture(Leap.Gesture.TYPE_CIRCLE);
		# controller.enable_gesture(Leap.Gesture.TYPE_KEY_TAP);
		# controller.enable_gesture(Leap.Gesture.TYPE_SCREEN_TAP);
		# controller.enable_gesture(Leap.Gesture.TYPE_SWIPE);

	def on_disconnect(self, controller):
		# Note: not dispatched when running in a debugger.
		print "Disconnected"

	def on_exit(self, controller):
		print "Exited"

	def on_frame(self, controller):
		self.screen.fill((0, 0, 0))

		frame = controller.frame()
		interactionBox = frame.interaction_box

		# finger = frame.fingers.frontmost
		for finger in frame.fingers:

			normalizedPosition = interactionBox.normalize_point(finger.stabilized_tip_position)

			x, y = normalizedPosition.x, normalizedPosition.y

			scaledX, scaledY = (int(x*WIDTH), int(HEIGHT-(y*HEIGHT)))

			pointScaler = normalizedPosition.z
			pointSize = int(10*pointScaler)

			pygame.draw.circle(self.screen, (128, 128, 128), (scaledX, scaledY), pointSize)

			if self.ball.surrounds((scaledX, scaledY)):

				self.ball.vY = 0
				self.ball.vX = 0

				self.ball.x = scaledX-(self.ball.width//2)
				self.ball.y = scaledY-(self.ball.height//2)

		self.screen.blit(self.ball.image, self.ball.move())

		pygame.display.update()


	def state_string(self, state):
		if state == Leap.Gesture.STATE_START:
			return "STATE_START"

		if state == Leap.Gesture.STATE_UPDATE:
			return "STATE_UPDATE"

		if state == Leap.Gesture.STATE_STOP:
			return "STATE_STOP"

		if state == Leap.Gesture.STATE_INVALID:
			return "STATE_INVALID"


class Ball():
	def __init__(self, image, size=(50, 50), pos=(0, 0), bounds=(400, 400)):
		self.image = image

		self.x, self.y = pos
		self.width, self.height = size
		self.xBound, self.yBound = bounds

		self.vX = 0
		self.vY = 0

	def move(self):
		self.x += self.vX
		self.y += self.vY

		self.update()

		return (self.x, self.y)

	def update(self):
		if self.y > self.yBound:
			self.y = self.yBound
			self.vY = -self.vY

		if self.x > self.xBound or self.x < 0:
			self.x = self.xBound

		self.vY += .05 # Gravity
		self.vX *= .9 # Friction

	def surrounds(self, pointer_pos):
		x, y = pointer_pos
		if x < (self.x + self.width) and x > (self.x) and y < (self.y + self.height) and y > (self.y):
			return True

def main():
	# Create a sample listener and controller
	listener = SampleListener()
	controller = Leap.Controller()

	# Have the sample listener receive events from the controller
	controller.add_listener(listener)

	while True:
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				controller.remove_listener(listener)
				pygame.quit()
				break
			if event.type == pygame.KEYDOWN:
				# print "Key pressed"
				pygame.image.save(listener.screen, 'test.png')

	# sys.stdin.readline()

	# Remove the sample listener when done

if __name__ == "__main__":
	main()