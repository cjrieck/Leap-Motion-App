import sys, random, os, string
import Leap, pygame
from Leap import SwipeGesture
from pygame.locals import *

pygame.init()

# Retrieve highest available screen resolution and set window dimensions to that
available_resolutions = pygame.display.list_modes()
WIDTH = available_resolutions[0][0]
HEIGHT = available_resolutions[0][1]
SIZE = WIDTH, HEIGHT

SSHOT_FOLDER = 'screenshots'

class Cursor(pygame.sprite.Sprite):
	def __init__(self):
		pygame.sprite.Sprite.__init__(self)
		self.image = pygame.Surface((50, 50)) # 50x50 pixel pygame surface
		self.image.fill((0, 255, 255)) # Make the cursor cyan
		# pygame.draw.circle(self.image, (0, 0, 255), (25, 25), 25, 0)
		self.rect = self.image.get_rect() # returns rectangle that bounds this image
		self.height = 0 
		self.width = 0
	
	"""
	Function: update(self, position)
	Arguments: position -> position of the finger relative to the screen
	Moves the cursor along with the finger
	"""
	def update(self, position):
		self.rect.center = position

	"""
	Function: scale_cursor(self, multiple)
	Arguments: multiple -> number less than 1 (finger distance from the 'touch' zone)
	Will scale the square cursor relative to how far the tip of the User's
	finger is from the 'touch' or drawing zone
	"""
	def scale_cursor(self, multiple):
		# scale the cursor dimensions
		self.height = int(multiple * 150)
		self.width = int(multiple * 150)
		
		newResolution = (self.width, self.height)
		previousCursor = self.image
		
		self.image = pygame.transform.smoothscale(previousCursor, newResolution) # set the new cursor to a new image with new dimensions
		self.image.fill((0,255,255)) # keep color the same
		return self.image

	"""
	Change the fill of the image from cyan to black
	"""
	def change_fill(self):
		self.image.fill((0,0,0))
		return self.image


class EduListener(Leap.Listener):

	def __init__(self):
		Leap.Listener.__init__(self)
		self.draw_on = False
		self.color = 255,128,0
		self.radius = 10

		# Initialize all to 0, they will be set later
		self.screen = 0
		self.background = 0
		self.cursor = 0
		self.allSprites = 0

		self.counter = 0
		self.last_position = (0,0)
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

		self.counter += 1
		print "In on_frame", self.counter
		if self.background == 0:
			return

		self.frame = controller.frame() # The frame of info from the leap
		interactionBox = self.frame.interaction_box # equivalent to a point cloud in 3-space

		self.gestures = self.frame.gestures() # get activated gestures if present

		finger = self.frame.fingers.frontmost
		distance = finger.touch_distance
		stabilizedPosition = finger.stabilized_tip_position # Stabilized tip position for smoother movement at the cost of accuracy/speed
		normalizedPosition = interactionBox.normalize_point(stabilizedPosition)

		x = normalizedPosition.x * WIDTH
		y = HEIGHT * (1 - normalizedPosition.y)

		finger_pos = (int(x),int(y))

		if distance <= 0.5 and distance > 0:
			self.cursor.image = self.cursor.scale_cursor(distance) #change cursor dimensions based on distance from 'touch zone'
			
			# self.allSprites.draw(self.screen)
			self.allSprites.clear(self.screen, self.background) # clear sprites from last draw() call on the group
			self.allSprites.update(finger_pos) # updates sprites on the screen		

		self.last_position = finger_pos
		if (not self.frame.gestures().is_empty) and (distance > 0 and (not self.draw_on)): # if swipe while not drawing
			self.screen.fill((0,0,0)) # make the screen black

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


def initailizePictureFolder():
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

	last_position = (0,0)
	# Running the pygame loop
	while True:

		frame = leapController.frame() # The frame of info from the leap
		interactionBox = frame.interaction_box # equivalent to a point cloud in 3-space
		gestures = frame.gestures()
		finger = frame.fingers.frontmost
		distance = finger.touch_distance
		stabilizedPosition = finger.stabilized_tip_position # Stabilized tip position for smoother movement at the cost of accuracy/speed
		normalizedPosition = interactionBox.normalize_point(stabilizedPosition)
		
		# Keeps the finger point within the dimensions of the screen
		x = normalizedPosition.x * WIDTH
		y = HEIGHT * (1 - normalizedPosition.y)

		finger_pos = (int(x),int(y))

		if distance <= -0.001: # if in the 'touch zone'
			
			pygame.draw.circle(screen, leapListener.color, finger_pos, leapListener.radius) # defines the brush and size of the brush
			leapListener.draw_on = True

		if distance <= 0.5 and distance > 0: # if in the 'hover zone'

			leapListener.draw_on = False
			leapListener.color = (255, 255, 0) # Sets color to yellow

		if leapListener.draw_on: # if holding down mouse click
			pygame.draw.circle(background, leapListener.color, finger_pos, leapListener.radius) #draw a circle at new mouse position
			leapListener.round_line(screen, leapListener.color, finger_pos, last_position, leapListener.radius) # connects the 2 circles together to form a line
		
		last_position = finger_pos # update the last position to the position you ended the line on
		
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

		pygame.display.flip() # Update full display Surface to the screen


def main():
	# Initialize Leap stuff
	controller = Leap.Controller()
	listener = EduListener()
	controller.add_listener(listener)

	# Make a folder for screen shots if one doesn't exist
	initailizePictureFolder()

	# Run the function for all pygame behavior
	runPygame(controller, listener)

	# Once the game is done, remove the listener
	controller.remove_listener(listener)


if __name__ == '__main__':
	main()