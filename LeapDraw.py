# Pygame control happens in runPygame
# Leap Motion control happens in on_frame
# Initialization happens in main


import sys, random, os, string
import Leap, pygame
from Leap import SwipeGesture
from pygame.locals import *

pygame.init()

available_resolutions = pygame.display.list_modes()

WIDTH = available_resolutions[0][0]
HEIGHT = available_resolutions[0][1]
SIZE = WIDTH, HEIGHT

SSHOT_FOLDER = 'screenshots'

class Cursor(pygame.sprite.Sprite):
	def __init__(self):
		pygame.sprite.Sprite.__init__(self)
		self.image = pygame.Surface((50, 50))
		self.image.fill((0, 255, 255)) # Make the cursor cyan
		self.rect = self.image.get_rect()
		self.height = 0
		self.width = 0
		
	def update(self, position):
		self.rect.center = position

	def scale_cursor(self, multiple):
		self.height = int(multiple * 150)
		self.width = int(multiple * 150)
		
		newResolution = (self.width, self.height)
		previousCursor = self.image
		
		self.image = pygame.transform.smoothscale(previousCursor, newResolution)
		self.image.fill((0,255,255))
		return self.image

	def change_fill(self):
		self.image.fill((0,0,0))
		return self.image


class EduListener(Leap.Listener):
	def __init__(self):
		Leap.Listener.__init__(self)
		self.draw_on = False
		self.last_position = (0,0)
		self.color = 255,128,0
		self.radius = 10

		# Initialize all to 0, they will be set later
		self.screen = 0
		self.background = 0
		self.cursor = 0
		self.allSprites = 0

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
		9/16/13

		Figure out how to save what was drawn on the screen when you
		go back to hovering. Screenshot it then blit the background
		again as that screenshot?


		Get circle scaling right in scale_cursor function
		"""

		if self.background == 0:
			return

		frame = controller.frame() # The frame of info from the leap
		interactionBox = frame.interaction_box # The 3D box where we "draw"

		finger = frame.fingers.frontmost
		zone = finger.touch_zone
		distance = finger.touch_distance
		
		stabilizedPosition = finger.stabilized_tip_position # Stabilized tip position for smoother movement at the cost of accuracy/speed

		normalizedPosition = interactionBox.normalize_point(stabilizedPosition)
		
		x = normalizedPosition.x * WIDTH
		y = HEIGHT * (1 - normalizedPosition.y)

		finger_pos = (int(x),int(y))

		if distance <= -0.001:
			
			pygame.draw.circle(self.screen, self.color, finger_pos, self.radius) # defines the brush and size of the brush
			self.draw_on = True
			# self.cursor.image = self.cursor.change_fill()

		if distance <= 0.5 and distance > 0:

			self.draw_on = False
			# self.color = (random.randrange(256), random.randrange(256), random.randrange(256)) # generates random color
			self.color = (255, 255, 0) # Sets color to yellow

			self.cursor.image = self.cursor.scale_cursor(distance)

			self.allSprites.clear(self.screen, self.background)
			self.allSprites.update(finger_pos)



		if self.draw_on: # if holding down mouse click
			pygame.draw.circle(self.background, self.color, finger_pos, self.radius) #draw a circle at new mouse position
			self.round_line(self.background, self.color, finger_pos, self.last_position, self.radius) # connects the 2 circles together to form a line

		self.last_position = finger_pos # update the last position to the position you ended the line on
	
		if (not frame.gestures().is_empty) and (distance > 0 and (not self.draw_on)):
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

def initGame():
	if not os.path.isdir(SSHOT_FOLDER): # If the screenshot folder doesn't exist
		print "Creating folder" # Debug output
		os.mkdir(SSHOT_FOLDER) # Make the folder

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

def runPygame(leapController, leapListener):

	# Initialize the pygame stuff
	screen = pygame.display.set_mode(SIZE) # Make the pygame window
	background = pygame.Surface(screen.get_size()) # Get the Surface for the background of the window with the size of the window
	background.fill((0,0,0)) # Fill the background with black
	screen.blit(background, (0,0)) # Put the background onto the screen
	pygame.display.set_caption("LEAPS.edu") # Set the title of the pygame window
	font = pygame.font.SysFont("Comic Sans MS", 200) # Create a font for displaying text in the window

	cursor = Cursor() # Create a cursor sprite
	allSprites = pygame.sprite.Group(cursor) # Create a sprite group out of the cursor

	# Adding pygame info to the leap listener
	leapListener.background = background 
	leapListener.cursor = cursor
	leapListener.allSprites = allSprites
	leapListener.screen = screen

	# Counter for screen shots, just so we're not overwriting the old one with the new
	imageCounter = 1
	# Running the pygame loop
	while True:
		for event in pygame.event.get(): # Get all events and loop over
			if event.type == pygame.QUIT: # If the window has been X'ed out
				leapController.remove_listener(leapListener) # Remove the listener
				pygame.quit() # Quit pygame
				sys.exit() # Quit python
			if event.type == pygame.KEYDOWN: # Keypressed
				imageFilename = os.path.join(SSHOT_FOLDER, 'test' + str(imageCounter) + '.png') # Create screenshot filename
				print "Image saved as", imageFilename # Debug output
				pygame.image.save(leapListener.screen, imageFilename) # Save screenshot
				imageCounter += 1 # Increment the screenshot number

				result = analyzeImage(imageFilename) # Call analyzer function, store result
				cleanedResults = [x for x in result if x in string.ascii_letters] # Make a list, remove all non-ascii characters
				result = ''.join(cleanedResults) # Store the cleaned stuff back into result
				print "Results: ", result # Debug output
				label = font.render(result, 1, (255, 255, 0)) # Makes yellow label
				screen.fill((0,0,0)) # Fills screen with black
				screen.blit(label, (100, 100)) # Copies the label text onto the screen Surface



def main():
	# Initialize Leap stuff
	controller = Leap.Controller()
	listener = EduListener()
	controller.add_listener(listener)

	# Make a folder for screen shots if one doesn't exist
	initGame()

	# Run the function for all pygame behavior
	runPygame(controller, listener)

	# Once the game is done, remove the listener
	controller.remove_listener(listener)


if __name__ == '__main__':
	main()