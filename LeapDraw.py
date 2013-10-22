# Pygame control happens in runPygame
# Leap Motion control happens in on_frame
# Initialization happens in main

import sys, random, os, string
import Leap, pygame
from Leap import SwipeGesture
from pygame.locals import *

pygame.init()

# Retrieve highest available screen resolution and set window dimensions to that
available_resolutions = pygame.display.list_modes()
BLACK = (0, 0, 0)
WIDTH = available_resolutions[0][0]
HEIGHT = available_resolutions[0][1]
WIDTH, HEIGHT = WIDTH//2, HEIGHT//2
SIZE = WIDTH, HEIGHT

SCREENSHOT_FOLDER = 'screenshots'

class LeapDataListener(Leap.Listener):
	def on_init(self, controller):
		self.storage = {}
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
		self.frame = controller.frame() # The frame of info from the leap
		interactionBox = self.frame.interaction_box # equivalent to a point cloud in 3-space
													# Interaction box provides the 3D rectangle for the finger pos

		finger = self.frame.fingers.frontmost

		distance = finger.touch_distance # Distance forwards (z direction) (for drawing zones)
		stabilizedPosition = finger.stabilized_tip_position # Stabilized tip position for smoother movement at the cost of accuracy/speed
		normalizedPosition = interactionBox.normalize_point(stabilizedPosition) # Normalize the x, y, z pos to the 3D rect

		if len(self.frame.fingers) > 0:
			self.storage['nofingers'] = False
			self.storage['fingers'] = self.frame.fingers
		else:
			self.storage['nofingers'] = True
			self.storage['fingers'] = []

		self.storage['x'] = normalizedPosition.x
		self.storage['y'] = normalizedPosition.y
		self.storage['z'] = normalizedPosition.z

		self.storage['actualX'] = stabilizedPosition.x
		self.storage['actualY'] = stabilizedPosition.y
		self.storage['actualZ'] = stabilizedPosition.z

		# self.rectSurf = pygame.Surface((int(self.cursor.width), int(self.cursor.height)), self.background)
		# 3 Zones:
		# 		1: Drawing
		#		2: Hovering
		#		3: Hovering and danger of being out of range

		if distance > -1 and distance <= 0:
			self.storage['zone'] = 1

		if distance > 0 and distance <= 0.5:
			self.storage['zone'] = 0

		elif distance > .5 and distance <= 1:
			self.storage['zone'] = -1

class ZDistanceLine():
	def __init__(self, width, height, pos):
		self.width = width
		self.height = height
		self.pos = pos
		self.surface = pygame.Surface((self.width, self.height))
		self.dotColor = (255, 255, 255) # White
		self.dotSize = 15


		self.dotXPos = int(self.width // 2) # X value minus half the width
		self.dotYPos = int(self.height // 2)

		print self.dotXPos, self.dotYPos, self.pos


	def updatedSurface(self, zValue):
		# zValue ranges from 0 to 1
		self.dotYPos = int(self.pos[1] + (self.height*zValue))
		blitPos = (self.pos[0]-self.width, self.pos[1])

		self.surface.fill(BLACK)
		pygame.draw.circle(self.surface, (self.dotColor), (self.dotXPos, self.dotYPos), self.dotSize)

		return self.surface, blitPos




def initGame():
	if not os.path.isdir(SCREENSHOT_FOLDER): # If the screenshot folder doesn't exist
		print "Creating folder" # Debug output
		os.mkdir(SCREENSHOT_FOLDER) # Make the folder

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
def round_line(srf, color, start, end, radius=1):

	dx = end[0] - start[0] # x distance
	dy = end[1] - start[1] # y distance
	distance = max(abs(dx), abs(dy)) # find how far the 2 circles are from each other

	for i in range(distance): # iterate to connect circles
		x = int(start[0] + float(i)/distance*dx)
		y = int(start[1] + float(i)/distance*dy)

		pygame.draw.circle(srf,color,(x,y),radius)

def analyzeImage(filepath):
	tempFile = "out" # Name of output from tesseract
	tempFileExt = ".txt" # CONSTANT - DON'T CHANGE (comes from tesseract, not under our control)
	os.system("tesseract " + filepath + " " + tempFile + " -psm 8") # Call tesseract with the options
	tempFile += tempFileExt # Adding the extension on for all future use
	with open(tempFile, 'r') as fin: # Open the output from tesseract
		contents = ""
		for line in fin: # Grab all info from in-file
			contents += line

	os.remove(tempFile) # Delete the output file from tesseract
	return contents

def analyzeSurface(pygameSurface):
	baseName = 'screenshotTest'
	imageExt = '.png'
	imageFilename = os.path.join(SCREENSHOT_FOLDER, baseName + imageExt)
	pygame.image.save(pygameSurface, imageFilename)

	result = analyzeImage(imageFilename)
	cleanedResults = [x for x in result if x in string.ascii_letters]
	result = ''.join(cleanedResults)

	return cleanedResults

def get_eraser_information(screen, drawSurface, cursorSurface, previousEraserPos, cursorSize, fingerList):
	
	# Get finger vectors for fingers 1 and 2
	fingerOnePosition = fingerList[0].stabilized_tip_position
	fingerTwoPosition = fingerList[1].stabilized_tip_position

	# Find the vector of the midpoint
	fingersMidPointCoordinates = (fingerOnePosition + fingerTwoPosition) / 2

	# Get the x, y coordinates for the midpoint
	fingersMidPoint = (int(fingersMidPointCoordinates.x), int(fingersMidPointCoordinates.y))	

	# Make sure that the cursor stays within the dimensions of the screen (as noted in the Leap documentation)
	eraserPos = (fingersMidPoint[0]*HEIGHT, WIDTH-fingersMidPoint[1]*WIDTH)
	eraserColor = (255,255,255) # White

	# Set previous eraser position to the new position
	previousEraserPos = eraserPos

	return previousEraserPos, cursorSurface, drawSurface, eraserColor

def runPygame(leapController, leapListener):

	# Initialize the pygame stuff

	screen = pygame.display.set_mode(SIZE) # Make the pygame window
	background = pygame.Surface(screen.get_size()) # Get the Surface for the background of the window with the size of the window
	drawSurface = pygame.Surface(screen.get_size()) # Get the Surface for drawing
	font = pygame.font.SysFont("Comic Sans MS", 200) # Create a font for displaying text in the window
	pygame.display.set_caption("LEAPS.edu") # Set the title of the pygame window

	drawSurface.set_colorkey(BLACK)

	cursorSize = 10 # Size of cursor circle
	cursorWidth = 50 # Width of surface that holds the cursor
	cursorHeight = 50 # Height of surface that holds the cursor
	cursorOffset = cursorWidth // 2 # Offset to shift the cursor surface to the correct location
	cursorSurface = pygame.Surface((cursorWidth, cursorHeight)) # Surface to hold cursor circle
	cursorSurface.set_colorkey(BLACK)

	previousCursorPos = (0,0)
	previousEraserPos = (0,0)

	drawing = False # Current drawing state
	drawColor = (0, 255, 255) # Color to draw with (Cyan)

	pygame.draw.circle(cursorSurface, (255, 255, 0), (cursorOffset, cursorOffset), cursorSize)
	background.fill(BLACK) # Fill the background with black

	zDist = ZDistanceLine(40, 80, (WIDTH, 0))

	# Running the pygame loop
	while True:
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				leapController.remove_listener(leapListener)
				pygame.quit()
				sys.exit()
			if event.type == pygame.KEYDOWN and event.key == 306: # 306 is left-ctrl
				result = analyzeSurface(drawSurface)
				print result

		if 'x' not in leapListener.storage.keys(): # If the leap has no info to use
			print "\n\n\nPygame tried to run before Leap got a frame\n\n\n"
			continue # Don't do the rest of the loop

		cursorPos = ( int(leapListener.storage['x']*WIDTH), int((1 - leapListener.storage['y'])*HEIGHT) )
		cursorOpacity = 255 * (1 - leapListener.storage['z'])

		cachedEraserPos = (0,0)

		if leapListener.storage['zone'] == 1:
			cursorColor = (0, 255, 255) # Cyan
			if leapListener.storage['nofingers']:
				drawing = False
			else:
				drawing = True

		elif leapListener.storage['zone'] == 0:
			cursorColor = (255, 255, 0) # Yellow
			drawing = False

		elif leapListener.storage['zone'] == -1:
			cursorColor = (255, 0, 0) # Red
			drawing = False

		if drawing:
			if len(leapListener.storage['fingers']) == 2:
				previousEraserPos, cursorSurface, drawSurface, cursorColor = get_eraser_information(screen, drawSurface, cursorSurface, previousEraserPos, cursorSize, leapListener.storage['fingers'])
				
				# need to change the (0,0,0) to the color of the background
				pygame.draw.circle(drawSurface, (0,0,0), cursorPos, cursorSize)
				round_line(drawSurface, (0,0,0), cursorPos, previousCursorPos, cursorSize)

			else:
				pygame.draw.circle(drawSurface, drawColor, cursorPos, cursorSize)
				round_line(drawSurface, drawColor, cursorPos, previousCursorPos, cursorSize)


		pygame.draw.circle(cursorSurface, cursorColor, (cursorOffset, cursorOffset), cursorSize)
		cursorOffsetPos = (cursorPos[0]-cursorOffset, cursorPos[1]-cursorOffset) # Necessary because rects are 
																				 # copied from the upper left corner

		distLineSurface, distLinePos = zDist.updatedSurface(leapListener.storage['z'])

		previousCursorPos = cursorPos

		screen.blit(background, (0,0)) # Put the background onto the screen
		screen.blit(drawSurface, (0,0)) # Put the drawing onto the screen
		screen.blit(distLineSurface, distLinePos) # Put the drawing onto the screen
		screen.blit(cursorSurface, (cursorOffsetPos)) # Put the cursor onto the screen

		pygame.display.flip() # Update full display Surface to the screen


def main():
	# Initialize Leap stuff
	controller = Leap.Controller()
	listener = LeapDataListener()
	controller.add_listener(listener)

	# Make a folder for screen shots if one doesn't exist
	initGame()

	# Run the function for all pygame behavior
	runPygame(controller, listener)

	# Once the game is done, remove the listener
	controller.remove_listener(listener)


if __name__ == '__main__':
	main()