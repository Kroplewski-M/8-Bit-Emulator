#pragma once
#include "Valmac.h"
#include "iostream"



int main(int argc, char** argv)
{
	Valmac myValmac;
	myValmac.initialize();
	//test program 1
	myValmac.load_program(myValmac.MasterMind, sizeof(myValmac.MasterMind));

	int i = 0;
	while (myValmac.g_bRunning)
	{
		myValmac.emulateCycle();
		myValmac.SampleInput();
		i++;
		myValmac.g_bRunning = i < 10;
	}


	std::cout << "V[0] exit code was: " << myValmac.V[0] << std::endl;
	std::cout << "End of World\n";

	system("pause");
}

