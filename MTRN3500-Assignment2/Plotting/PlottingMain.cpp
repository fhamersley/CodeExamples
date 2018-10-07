#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <zmq.hpp>

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <map>
#include <vector>
#include <math.h>
#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>

#include <Windows.h>
#include <tchar.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <turbojpeg.h>


void display();
void idle();

GLuint tex;
zmq::context_t context(1);
zmq::socket_t subscriber(context, ZMQ_SUB);

SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
int val = PMObj.SMAccess();
ProcessManagement * PM = (ProcessManagement*)PMObj.pData;		//Pointer 'PM' for accessing the data

//-------------------------------------------------------//
// THIS IS NOT PLOTTING. THIS IS ACTUALLY THE CAMERA CODE
//-------------------------------------------------------//

int main(int argc, char ** argv) {

	PM->ShutDown.Flags.Plot = 0;

	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - GL CAMERA");

	glutDisplayFunc(display);
	glutIdleFunc(idle);

	std::cout << "Camera Module started" << std::endl;

	glGenTextures(1, &tex);

	// Connect to server 

	//  Socket to talk to server
	subscriber.connect("tcp://192.168.1.200:26000");

	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	glutMainLoop();

	return 0;
} 

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	glBegin(GL_QUADS);
		//glColor3f(1, 0, 0);

		glTexCoord2f(0, 1);  glVertex2f(-1, -1);
		glTexCoord2f(1, 1);  glVertex2f(1, -1);
		glTexCoord2f(1, 0);  glVertex2f(1, 1);
		glTexCoord2f(0, 0);  glVertex2f(-1, 1);
	glEnd();

	glutSwapBuffers();
}

void idle()
{
	
	// Set Plotting (camera) heartbeat
	PM->HeartBeat.Flags.Plot = 1;

	/*//--------------------------------------------------------------------------------------------//
	PM->ShutDown.Flags.Plot = 0;
	while (!PM->ShutDown.Flags.Plot)
	{
		PM->HeartBeat.Flags.Plot = 1;
		if (_kbhit()) {
			std::cout << "Breaking" << std::endl;
			Sleep(1000);
			break;
		}
	}
	//return 0;

	//--------------------------------------------------------------------------------------------//*/

	// Receive from ZMQ
	zmq::message_t update;

	if (subscriber.recv(&update, ZMQ_NOBLOCK)) {

		std::cout << "Received data" << update.size() << std::endl;
		
		// JPEG Decompression
		long unsigned int _jpegSize = update.size(); //!< _jpegSize from above
		unsigned char* _compressedImage = static_cast<unsigned char*>(update.data()); //!< _compressedImage from above

		int jpegSubsamp, width, height;
		tjhandle _jpegDecompressor = tjInitDecompress();
		tjDecompressHeader2(_jpegDecompressor, _compressedImage, _jpegSize, &width, &height, &jpegSubsamp);
		unsigned char * buffer = new unsigned char[width*height*3]; //!< will contain the decompressed image

		tjDecompress2(_jpegDecompressor, _compressedImage, _jpegSize, buffer, width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);
		tjDestroy(_jpegDecompressor);
		
		// Load textures
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE , buffer);
		Sleep(100);
		delete[] buffer;


	}
	else {
		std::cout << "Received no data" << std::endl;
	}

	
	if (PM->ShutDown.Flags.Plot)
	{
		exit(0);
	}

	PM->HeartBeat.Flags.Plot = 1;

	// Close module if PM is dead
	if (PM->HeartBeat.Flags.PM == 0)
	{
		double TempTS = PM->PMTimeStamp;
		Sleep(200);
		if (TempTS == PM->PMTimeStamp)
		{
			std::cout << "PM not detected. Shutting down Plotting (camera)" << std::endl;
			PM->ShutDown.Flags.Plot = 1;
		}
	}


	display();
}