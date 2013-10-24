Leap-Motion-App
===============

Leap Into the Future of Education

 Teammates: Clayton, Robert, Aimee, Bryan

*** Make sure to include appropriate files for your system in the app's directory ***

 GitHub Tutorial:

 http://net.tutsplus.com/articles/general/team-collaboration-with-github/

To compile RubiksControl.cpp:
	In terminal, run ->
		cd /Leap-Motion-App
		g++ -std=c++98 RubiksControl.cpp -I ./include -o ./build/RubiksController ./lib/libLeap.dylib
		cp ./lib/libLeap.dylib ./build
		./build/RubiksController 