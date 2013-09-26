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
        self.image.fill((0, 255, 255))
        # pygame.draw.circle(self.image, (0, 0, 255), (25, 25), 25, 0)
        self.rect = self.image.get_rect()

        
    def update(self, position):
        self.rect.center = position

    def scale_cursor(self, multiple):
    	height = int(multiple * 150)
    	width = int(multiple * 150)
    	
    	newResolution = (width, height)
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

		frame = controller.frame()
		interactionBox = frame.interaction_box

		finger = frame.fingers.frontmost
		zone = finger.touch_zone
		distance = finger.touch_distance
		
		stabilizedPosition = finger.stabilized_tip_position

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
			self.allSprites.draw(self.screen)

		if self.draw_on: # if holding down mouse click
			pygame.draw.circle(self.screen, self.color, finger_pos, self.radius) #draw a circle at new mouse position
			self.round_line(self.screen, self.color, finger_pos, self.last_position, self.radius) # connects the 2 circles together to form a line

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

def initailizePictureFolder():
	if not os.path.isdir(SSHOT_FOLDER):
		print "Creating folder"
		os.mkdir(SSHOT_FOLDER)

def analyzeImage(filepath):
	os.system("tesseract " + filepath + " out -psm 8")
	with open('out.txt', 'r') as fin:
		contents = ""
		for line in fin:
			contents += line

	os.remove('out.txt')
	return contents 

def runPygame(leapController, leapListener):

	screen = pygame.display.set_mode(SIZE)
	background = pygame.Surface(screen.get_size())
	background.fill((0,0,0))
	screen.blit(background, (0,0))
	pygame.display.set_caption("LEAPS.edu")
	font = pygame.font.SysFont("Comic Sans MS", 200)

	cursor = Cursor()
	allSprites = pygame.sprite.Group(cursor)

	# Adding pygame info to listener
	leapListener.background = background
	leapListener.cursor = cursor
	leapListener.allSprites = allSprites
	leapListener.screen = screen

	imageCounter = 1
	# Running the pygame loop
	while True:
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				leapController.remove_listener(leapListener)
				pygame.quit()
				sys.exit()
			if event.type == pygame.KEYDOWN:
				imageFilename = os.path.join(SSHOT_FOLDER, 'test' + str(imageCounter) + '.png')
				print "Image saved as", imageFilename
				pygame.image.save(leapListener.screen, imageFilename)
				imageCounter += 1

				result = analyzeImage(imageFilename)
				cleanedResults = [x for x in result if x in string.ascii_letters]
				result = ''.join(cleanedResults)
				print "Results: ", result
				label = font.render(result, 1, (255, 255, 0)) # Makes yellow label
				screen.fill((0,0,0))
				screen.blit(label, (100, 100))



def main():
	controller = Leap.Controller()
	listener = EduListener()
	controller.add_listener(listener)

	initailizePictureFolder()

	runPygame(controller, listener)

	controller.remove_listener(listener)


if __name__ == '__main__':
	main()