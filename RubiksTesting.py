DIRECTION_MAP = {
	'c': {
		0: 2,
		1: 5,
		2: 8,
		3: 1,
		4: 4,
		5: 7,
		6: 0,
		7: 3,
		8: 6,
	},
	'x': {
		0: 6,
		1: 3,
		2: 0,
		3: 7,
		4: 4,
		5: 1,
		6: 8,
		7: 5,
		8: 2,
	}
}

class CubeFace:
	def __init__(self, color):
		self.pieces = [
			color, color, color,
			color, color, 'd',
			color, color, color
		]

	def __str__(self):
		strBuff = ''
		rowNum = 0
		for pieceColor in self.pieces:
			if rowNum >= 3:
				strBuff += '\n'
				rowNum = 0
			strBuff += pieceColor
			rowNum += 1

		return strBuff + '\n'

	def rotate(self, direction):
		if direction == 'c': # Clockwise
			shiftedList = []
			for i in xrange(len(self.pieces)):
				reverseDestination = DIRECTION_MAP['x'][i]
				shiftedList.append(self.pieces[reverseDestination]) # Note: Constructing backwards means 
														  			# using opposite turn to construct turn

		elif direction == 'x': # Counter-clockwise
			shiftedList = []
			for i in xrange(len(self.pieces)):
				reverseDestination = DIRECTION_MAP['c'][i]
				shiftedList.append(self.pieces[reverseDestination]) # Note: Constructing backwards means 
														  			# using opposite turn to construct turn

		self.pieces = shiftedList


	def neighborRotate(self, direction):
		if direction == 'c':
			


class RubiksCube:
	# 0: white
	# 1: red
	# 2: blue
	# 3: orange
	# 4: green
	# 5: yellow
	def __init__(self):
		self.neighborsOf = {
			'w': ['r', 'b', 'o', 'g'],
			'r': ['w', 'b', 'g', 'y'],
			'b': ['w', 'r', 'o', 'y'],
			'o': ['w', 'b', 'g', 'y'],
			'g': ['w', 'r', 'o', 'y'],
			'y': ['r', 'b', 'o', 'g']
		}

		self.faces = {
			'w': CubeFace('w'),
			'r': CubeFace('r'),
			'b': CubeFace('b'),
			'o': CubeFace('o'),
			'g': CubeFace('g'),
			'y': CubeFace('y')
		}

	def __str__(self):
		strBuff = ''
		
		for colorKey, face in self.faces.items():
			strBuff += str(face)

		return strBuff

	def rotate(self, faceColor, direction):
		self.faces[faceColor].rotate(direction)

		placeholder = ['x', 'x', 'x']
		for faceColor in self.neighborsOf[faceColor]:
			placeholder = self.faces[faceColor].neighborRotate(direction, placeholder)


def main():
	cube = RubiksCube()

	print cube
	cube.rotate('w', 'c')
	print cube

main()