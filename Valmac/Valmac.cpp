// Valmac8.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Emulation execution is separate, mostly controller by PC.

#include <SFML/Graphics.hpp>
#include <iostream>
#include "Valmac.h"



Valmac::Valmac()
	:window(sf::VideoMode(800, 600), "Valmac!"){}

unsigned char Valmac::get_mem(unsigned int p_adress)
	{
		_ASSERT(p_adress < MEMORY_SIZE);
		return memory[p_adress];
	}

	void Valmac::set_mem(unsigned int p_address, unsigned char p_value)
	{
		_ASSERT(p_address < MEMORY_SIZE);
		memory[p_address] = p_value;
	}

	void Valmac::clear_memory(void)
	{
		memset(memory, 0, MEMORY_SIZE);
	}

	void Valmac::initialize()
	{
		// Initialize registers and memory once
		PC = 0x0200;  // Program counter starts at 0x200
		opcode = 0;      // Reset current opcode	
		I = 0;      // Reset index register
		SP = 0;      // Reset stack pointer

		// Clear display	
		window.clear(sf::Color(0,0,0,255));
		system("CLS");
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

	bool Valmac::load_program(uint16_t* pProgram, size_t bufferSize)
	{
	for(int i = 0; i < (sizeof(pProgram) / sizeof(uint16_t)); i++)
	{
		memory[0x0200 + 2 * i] = (unsigned char)pProgram[i];
		memory[0x0200 + 2 * i + 1] = unsigned char(pProgram[i] >>8);
	}

		return true;
	}

	uint16_t Valmac::getProgramOpcode()
	{
		_ASSERT((PC & 1) == 0);  //PC is not 0DD
		_ASSERT(PC >= 0x0200); //PC is within memory

		uint16_t l_opcode = memory[PC + 1] << 8 | memory[PC];
		return l_opcode;
	}

	void Valmac::step_PC()
	{
		PC += 2; //PC cannot be odd
		_ASSERT(PC < MEMORY_SIZE - sizeof(opcode));
	}

	void Valmac::emulateCycle()
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
				break;
			}
			else if ((opcode & 0x00FF) == 0x00EE)
			{
				SP -= 1;
				PC = stack[SP];
				//PC = stack[--SP]; 
				//stack[SP] = 0; 
				step_PC();
			}
			else if ((opcode & 0x00FF) == 0x00FF)
			{
				g_bRunning = false;
				break;
			}
			else if ((opcode & 0x00F0) == 0x00E0)
			{
				window.clear(sf::Color(0, 0, 0, 255));
				system("CLS");
				std::cout << "Cleared screen\n";
				break;
			}
			break;
		case 0x1000:				//opcode == 0x1A08
			PC = opcode & 0xFFF;	//PC == 0x0A08 
			break;
		case 0x2000:
			std::cout << "Called Subroutine at NNN\n";
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
				std::cout << "stepped PC \n";
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
			std::cout << "Setting V\n";
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
				std::cout << "Result: " << V[(opcode & 0x0F00) >> 8] << " " << V[0x0F] << std::endl;
				break;
			case 0x0005:
				std::cout << "before: " << (int)V[(opcode & 0x0F00) >> 8] << " " << (int)V[0x0F] << std::endl;
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
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
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
			{
				std::cout << "Step PC \n";
				step_PC();
			}
			break;
		case 0xA000:
			I = opcode & 0x0FFF;
			std::cout << "setting I\n";
			break;
		case 0xB000:
			std::cout << "Setting PC\n";
			PC = V[0] + (opcode & 0x0FFF);
			break;
		case 0xC000:
			V[(opcode & 0x0F00) >> 8] = (rand() % (255 + 1)) & (opcode & 0x00FF);
			std::cout << "Random set \n";
		case 0xE000:
			switch (opcode & 0x00FF)
			{
			case 0x009E:
				if (keypad[V[(opcode & 0x0F00) >> 8]])
				{
					step_PC();
					std::cout << "Step PC \n";
				}
				break;
			case 0x00A1:
				if (!keypad[V[(opcode & 0x0F00) >> 8]])
				{
					step_PC();
				}
				break;
			default:
				break;
			}
			step_PC();
			break;
		case 0xF000:
			std::cout << '/7' << std::endl;
			std::cout << "BEEP\n";
			switch (opcode & 0x00FF)
			{
			case 0x0007:
				std::cout << "Set V to delay time\n";
				V[(opcode & 0x0F00) >> 8] = delay_timer;
				break;
			case 0x000A:
				while (!(keypad[0] || keypad[1] || keypad[2] || keypad[3] || keypad[4] || keypad[5] || keypad[6] || keypad[7] || keypad[8]
					|| keypad[9] || keypad[10] || keypad[11] || keypad[12] || keypad[13] || keypad[14] || keypad[15]))
				{
					SampleInput();
				}
				for (size_t i = 0; i < 16; i++)
				{
					if (keypad[i])
					{
						V[(opcode && 0x0F00) >> 8] = i;
						break;
					}
				}
				break;
			case 0x0015:
				std::cout << "Setting delay timer\n";
				delay_timer = V[(opcode & 0x0F00) >> 8];
				break;
			case 0x0018:
				std::cout << "Setting sound timer\n";
				sound_timer = V[(opcode & 0x0F00) >> 8];
				break;
			case 0x001E:
				std::cout << "Setting index\n";
				I += (V[(opcode & 0x0F00) >> 8]);
				break;
			case 0x00029:

				break;
			case 0x0033:

				break;
			case 0x0055:
				std::cout << "reg_dump\n";
				for (int i = 0; i <= (opcode & 0x0F00) >> 8;i++)
				{
					memory[I + i] = V[i];
				}
				break;
			case 0x0065:
				std::cout << "reg_load\n";
				for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
				{
					V[i] = I + i;
				}
				break;
			default:
				break;
			}
			step_PC();
			break;
			

		default:
			step_PC();
			break;
		}
		// Execute Opcode

		// Update timers
	}

	void Valmac::SampleInput()
	{
		sf::Event event;
		static int KeyCount = 0;
		//while (window.isOpen())
		{
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window.close();
				}
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
		{
			KeyCount++;
			keypad[11] = 1;
		}
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

	uint16_t MasterMind[3] =
	{
		//TEST01
		0xFF18,
		0xC208,
		0xC308
	};

	