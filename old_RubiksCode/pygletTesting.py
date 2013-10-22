import pyglet, os
from pyglet.gl import *
from pyglet.window import key
from pyglet.window import mouse

win = pyglet.window.Window()

# label = pyglet.text.Label('Hello, World',
# 						font_name='Times New Roman',
# 						font_size=36,
# 						x=window.width//2, y=window.height//2,
# 						anchor_x='center', anchor_y='center')

# path = r"C:\\Users\\bryan\\Documents\\GitHub\\Leap-Motion-App\\faces\\BlueFace.PNG"
image = pyglet.resource.image('BlueFace.PNG')

@win.event
def on_draw():
	glClear(GL_COLOR_BUFFER_BIT)

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)


	# Draw some stuff
	glBegin(GL_TRIANGLE_FAN)
	glVertex2i(200, 300)
	glVertex2i(250, 250)
	glVertex2i(300, 200)
	glVertex2i(250, 150)
	glEnd()
	# window.clear()
	# image.blit(10, 10)

@win.event
def on_key_press(symbol, modifiers):
	if symbol == key.LEFT:
		print "The left key was pressed"

@win.event
def on_mouse_press(x, y, button, modifiers):
	if button == mouse.LEFT:
		print "The LMB was pressed"


pyglet.app.run()