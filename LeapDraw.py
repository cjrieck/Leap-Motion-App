import sys
import Leap
from Leap import SwipeGesture
import pygame
import random

pygame.init()

available_resolutions = pygame.display.list_modes()

WIDTH = available_resolutions[0][0]
HEIGHT = available_resolutions[0][1]
SIZE = WIDTH, HEIGHT

class MyListener(Leap.Listener):

	def on_init(self, controller):
		self.draw_on = False
		self.last_position = (0,0)
		self.color = 255,128,0
		self.radius = 10
		self.screen = pygame.display.set_mode(SIZE)
		print "Initialized"

	def on_connect(self, controller):
		print "Connected"

		# put in gestures here
		controller.enable_gesture(Leap.Gesture.TYPE_SWIPE)

	def on_disconnect(self, controller):
		print "Disconnected"

	def on_exit(self, controller):
		print "Exited"

	def on_frame(self, controller):
		
		"""
		NOTE: Maybe on SWIPE gesture, erase screen?
		"""

		frame = controller.frame()
		interactionBox = frame.interaction_box

		finger = frame.fingers.frontmost
		zone = finger.touch_zone
		distance = finger.touch_distance
		
		stabilizedPosition = finger.stabilized_tip_position

		normalizedPosition = interactionBox.normalize_point(stabilizedPosition)
		
		x = normalizedPosition.x * WIDTH
		y = HEIGHT - normalizedPosition.y * HEIGHT

		finger_pos = (int(x),int(y))

		if distance <= -0.1:
			
			pygame.draw.circle(self.screen, self.color, finger_pos, self.radius) # defines the brush and size of the brush
			self.draw_on = True


		if distance <= 0.3 and distance > -0.1: #zone == 0: #e.type == pygame.MOUSEBUTTONUP: # has data types of 'pos' and 'button'
			self.draw_on = False
			self.color = (random.randrange(256), random.randrange(256), random.randrange(256)) # generates random color

		# if e.type == pygame.MOUSEMOTION: # while clicking and holding. Has data type of 'pos', 'rel' and 'button'
		if self.draw_on: # if holding down mouse click
			pygame.draw.circle(self.screen, self.color, finger_pos, self.radius) #draw a circle at new mouse position
			self.round_line(self.screen, self.color, finger_pos, self.last_position, self.radius) # connects the 2 circles together to form a line
		self.last_position = finger_pos # update the last position to the position you ended the line on
	
		if (not frame.gestures().is_empty) and distance > 0.3:
			self.screen.fill((0,0,0))
			
		pygame.display.flip() # Update full display Surface to the screen

	"""
	Function:
	Allows for the connection of 2 circles to create a line.

	Arguments:
	srf = the pygame Surface
	color = color in (int, int, int) form
	start = starting position of mouse
	end = ending position of mouse
	radius = default is 1 if nothing passed in. Determines size of brush.
	"""
	def round_line(self, srf, color, start, end, radius=1):

		dx = end[0] - start[0] # x distance
		dy = end[1] - start[1] # y distance
		distance = max(abs(dx), abs(dy)) # find how far the 2 circles are from each other

		for i in range(distance): # iterate to connect circles
			x = int(start[0] + float(i)/distance*dx)
			y = int(start[1] + float(i)/distance*dy)

			pygame.draw.circle(srf,color,(x,y),radius)

def main():

	controller = Leap.Controller()
	listener = MyListener()

	controller.add_listener(listener)

	sys.stdin.readline()

	controller.remove_listener(listener)


if __name__ == '__main__':

	main()

