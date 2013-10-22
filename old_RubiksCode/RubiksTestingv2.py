SIZE_CUBE = 3
GAP_SIDE = 'X'

class CubePiece:
	def __init__(self, sides):
		self.sides = sides

	def __str__(self):
		strBuff = ''

		for side in self.sides:
			if side != GAP_SIDE:
				strBuff += side

		return strBuff + '\n'


class RubiksCube:
	def __init__(self):
		white = 'w'; red = 'r'
		blue = 'b'; orange = 'o'
		green = 'g'; yellow = 'y'

		none = GAP_SIDE

		self.pieceArray = []
		for x in xrange(SIZE_CUBE):
			for y in xrange(SIZE_CUBE):
				for z in xrange(SIZE_CUBE):
					cubeSides = [white, red, blue, orange, green, yellow]
					if x > 0:
						cubeSides[2] = none
					if x < SIZE_CUBE-1:
						cubeSides[4] = none
					if y > 0:
						cubeSides[0] = none
					if y < SIZE_CUBE-1:
						cubeSides[5] = none
					if z > 0:
						cubeSides[1] = none
					if z < SIZE_CUBE-1:
						cubeSides[3] = none

					# print "Piece at %i %i %i has sides %s" % (x, y, z, ' '.join(cubeSides))

					self.pieceArray.append(CubePiece(cubeSides))

	def __str__(self):
		strBuff = ''
		for piece in self.pieceArray:
			strBuff += str(piece)

		return strBuff



		

def main():
	cube = RubiksCube()

	print cube

main()