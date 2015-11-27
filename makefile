default: all run

all:
	g++ -o acidtrip Visualisering.cpp Object3D.cpp -lGL -lGLU -lglut
clean:
		
run:
	./acidtrip
