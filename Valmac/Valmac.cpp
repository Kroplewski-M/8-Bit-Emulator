// Valmac8.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Emulation execution is separate, mostly controller by PC.

#include <SFML/Graphics.hpp>
#include <iostream>

#define MEMORY_SIZE	(4096)
#define STACK_SIZE (16)


struct Valmac
{
	bool g_bRunning = true;
	//The Valmac has around 35 opcodes which are all two bytes long. 
	uint16_t opcode;
	
	////4K memory in total
	// NB 1K is 1024 bytes (not 1000)
	uint8_t memory[MEMORY_SIZE];

	inline unsigned char get_mem(unsigned int p_adress)
	{
		_ASSERT(p_adress < MEMORY_SIZE);
		return memory[p_adress];
	}
	inline void set_mem(unsigned int p_address, unsigned char p_value)
	{
		_ASSERT(p_address < MEMORY_SIZE);
		memory[p_address] = p_value;
	}
	inline void clear_memory(void)
	{
		memset(memory, 0, MEMORY_SIZE);
	}


	// CPU registers :  15, 8 - bit general purpose registers named V0, V1 up to VE.
	// The 16th register, Vf, is used  for the 'carry flag'
	uint8_t  V[16];

	//There is an Index register I and a program counter (PC) which can have a value from 0x000 to 0xFFF
	uint16_t I;
	uint16_t PC;
	//The systems memory map :
	// 0x000 - 0x1FF - interpreter (contains font set in emu)
	// 0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
	// 0x200 - 0xFFF - Program ROM and work RAM

	//The graphics system : There is one instruction that draws sprite to the screen. Drawing is done in XOR mode 
	//and if a pixel is turned off as a result of drawing, the VF register is set. 
	//This is used for collision detection.
	//The graphics are black and white and the screen has a total of 2048 pixels(64 x 32).
	//This can easily be implemented using an array that hold the pixel state(1 or 0) :
	uint8_t gfx[64 * 32];

	//no Interrupts or hardware registers.
	//but there are two timer registers that count at 60 Hz.
	//When set above zero they will count down to zero.
	uint8_t delay_timer;
	uint8_t sound_timer;

	/*
	It is important to know that the instruction set has opcodes that allow the program to jump
	to a certain address or call a subroutine. While the specification don’t mention a stack, you will need
	to implement one as part of the interpreter yourself. The stack is used to remember the current location
	before a jump is performed. So anytime you perform a jump or call a subroutine, store the program counter
	in the stack before proceeding. The system has 16 levels of stack and in order to remember which level of
	the stack is used, you need to implement a stack pointer (SP).
	*/
	uint16_t stack[STACK_SIZE];
	uint16_t SP;

	//Finally, the HEX based keypad(0x0 - 0xF), you can use an array to store the 
	//current state of the key.
	uint8_t keypad[16];
	/*
		1	2	3	C
		4	5	6	D
		7	8	9	E
		A	0	B	F
	*/

	sf::RenderWindow window;

	Valmac() : window(sf::VideoMode(800, 600), "Valmac!") {};

	void initialize()
	{
		// Initialize registers and memory once
		PC = 0x200;  // Program counter starts at 0x200
		opcode = 0;      // Reset current opcode	
		I = 0;      // Reset index register
		SP = 0;      // Reset stack pointer
		 
		// Clear display	
		// Clear stack
		memset(stack, 0, sizeof(stack));
		// Clear registers V0-VF
		std::memset(V, 0, sizeof(V));
		// Clear keypad
		memset(keypad, 0, sizeof(keypad));
		// Clear memory
		clear_memory();

		//// Load font set
		//for (int i = ; i < 80; ++i)
		//	memory[i] = Valmac_fontset[i];

		// Reset timers	
	}

	uint16_t MasterMind[10] =
	{
		//0xFF18,
		//0x120A,
		//0xc208, //CXNN[2] is 0-7
		//0xff18,
		//0x00EE,
		//0x6412,
		//0x6313,
		//0x5340,
		//0x2206,
		//0xFF18
		0x6444,
		0x6501,
		0x8454,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		0x0000
	
	};

	bool load_program(uint16_t* pProgram, size_t bufferSize)
	{
		//Read a program from a file
		std::memcpy(memory + PC, pProgram , bufferSize * 2);

		return true;
	}

	uint16_t getProgramOpcode() //big endian
	{
		_ASSERT((PC & 1) == 0);  //PC is not 0DD
		_ASSERT((PC <= MEMORY_SIZE - sizeof(opcode))); //PC is within memory

		uint16_t l_opcode = memory[PC+1] << 8 | memory[PC];
		return l_opcode;
	}

	inline void step_PC()
	{
		PC += 2; //PC cannot be odd
		_ASSERT(PC < MEMORY_SIZE - sizeof(opcode));
	}

	void emulateCycle()		//run c clock tick
	{
		// Fetch Opcode
		opcode = getProgramOpcode();

		std::cout << "0x" << std::hex << opcode << std::endl;

		// Decode Opcode
		switch (opcode & 0xF000)
		{ 
		case 0x0000:
			if (opcode == 0x0000)
			{
				std::cout << "Doing Nothing\n ";
				step_PC();
			}
			else if ((opcode & 0x00FF) == 0x00FE )
			{
				SP -= 1;
				PC = stack[SP];
				//PC = stack[--SP]; 
				//stack[SP] = 0; 
				step_PC();
			}
				break;
		case 0x1000:				//opcode == 0x1A08
			PC = opcode & 0xFFF;	//PC == 0x0A08 
			break;
		case 0x2000:
			stack[SP] = PC;
			SP++;
			PC = (opcode & 0x0FFF);
			break;
		case 0x3000:
			if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x0FF))
			{
				step_PC();
				step_PC();
			}
			else
				step_PC();   
			break;
		case 0x4000:
			if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			{
				step_PC();
				step_PC();
			}
			else
				step_PC();
			break;
		case 0x5000:
			if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			{
				step_PC();
				step_PC(); 
			}
			else
				step_PC();
			break;
		case 0x6000:
			V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			step_PC();
			break;
		case 0x7000:
			V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			step_PC();
			break;
		case 0x8000:
			switch (opcode & 0x000F)
			{
			case 0x0000:
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
				std::cout << "After: " << (int)V[(opcode & 0x0F00) >> 8];
				break;
			case 0x0001:
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
				break;
			case 0x0002:
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
				break;
			case 0x0003:
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
				break;
			case 0x0004:
				std::cout << "Before: " << V[(opcode & 0x0F00) >> 8] << " " << V[0x0F] << std::endl;
				if (V[(opcode & 0x00F0 >> 4)] > (255 - V[(opcode & 0x0F00) >> 8]))
					V[0x0F] = 1;
				else
					V[0x0F] = 0;
				V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
				std::cout << "Result: " << V[(opcode & 0x0F00) >> 8] << " " <<V[0x0F] << std::endl;
				break;
			case 0x0005:
				std::cout << "before: " << (int)V[(opcode & 0x0F00) >> 8] << " " << (int) V[0x0F] << std::endl;
				if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
					V[0x0F] = 1;
				else
					V[0x0F] = 0;
				V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
				std::cout << "After: " << (int)V[(opcode & 0x0F00) >> 8] << " " << (int)V[0x0F] << std::endl;
				break;
			case 0x0006: 
				std::cout << "Before: " << (int)V[(opcode & 0x0F00) >> 8] << " " << (int)V[0x0F] << std::endl;
				V[0x0F] = V[(opcode & 0x0F00) >> 8] & 0x0001;
				V[(opcode & 0x0F00) >> 8] >>= 1;
				std::cout << "After: " << (int)V[(opcode & 0x0F00) >> 8] << " " << (int)V[0x0F] << std::endl;
				break;
			case 0x0007:
				std::cout << "Before: " << (int)V[(opcode & 0x0F00) >> 8] << " " << (int)V[0x0F] << std::endl;
				if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
					V[0x0F] = 0;
				else
					V[0x0F] = 1;
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8] ;
				std::cout << "After: " << (int)V[(opcode & 0x0F00) >> 8] << " " << (int)V[0x0F] << std::endl;
				break;
			case 0x000E:
				V[0x0F] = V[(opcode & 0x0F00) >> 8] >> 7;
				V[(opcode & 0x0F00) >> 8] <<= 1;
				std::cout << "After : " << (int)V[(opcode & 0x0F00) >> 8] << " " << (int)V[0x0F] << std::endl;
				break;
			default:
				break;
			}
			step_PC();
			break;
		case 0x9000:
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			{
				step_PC();
				step_PC();
			}
			else
				step_PC();
			break;
		case 0xA000:
			I = opcode & 0x0FFF;
			break;
		case 0xE000:
			switch (opcode & 0x00FF)
			{
			case 0x009E:  
				break;
			case 0x00A1:
				break;
			default:
				break;
			}
			step_PC();
			break;
		case 0xF000: 
			std::cout << '/7' << std::endl;
			std::cout << "BEEP\n";
			step_PC();
			break;

		default:
			step_PC();
			break;
		}
		// Execute Opcode

		// Update timers
	}
	void SampleInput()
	{
		sf::Event event;
		static int KeyCount = 0;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
		{
			KeyCount++;
			keypad[0] = 1;
		}
		else
			keypad[0] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
		{
			KeyCount++;
			keypad[1] = 1;
		}
		else
			keypad[1] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
		{
			KeyCount++;
			keypad[2] = 1;
		}
		else
			keypad[2] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4))
		{
			KeyCount++;
			keypad[3] = 1;
		}
		else
			keypad[3] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5))
		{
			KeyCount++;
			keypad[4] = 1;
		}
		else
			keypad[4] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6))
		{
			KeyCount++;
			keypad[5] = 1;
		}
		else
			keypad[5] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num7))
		{
			KeyCount++;
			keypad[6] = 1;
		}
		else
			keypad[6] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num8))
		{
			KeyCount++;
			keypad[7] = 1;
		}
		else
			keypad[7] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num9))
		{
			KeyCount++;
			keypad[8] = 1;
		}
		else
			keypad[8] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0))
		{
			KeyCount++;
			keypad[9] = 1;
		}
		else
			keypad[9] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			KeyCount++;
			keypad[10] = 1;
		}
		else
			keypad[10] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
			keypad[11] = 1;
		else
			keypad[11] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
		{
			KeyCount++;
			keypad[12] = 1;
		}
		else
			keypad[12] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			KeyCount++;
			keypad[13] = 1;
		}
		else
			keypad[13] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			KeyCount++;
			keypad[14] = 1;
		}
		else
			keypad[14] = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
		{
			KeyCount++;
			keypad[15] = 1;
		}
		else
			keypad[15] = 0;
	}
};

//#include   // OpenGL graphics and input
//#include "Valmac.h" // Your CPU core implementation

Valmac myValmac;

int main(int argc, char** argv)
{
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
 