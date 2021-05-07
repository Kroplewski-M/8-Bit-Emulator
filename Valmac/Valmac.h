#pragma once
#include <SFML\Graphics.hpp>
#define MEMORY_SIZE	(4096)
#define STACK_SIZE (16)

class Valmac
{
public:
	Valmac();

	inline unsigned char get_mem(unsigned int p_adress);
	inline void set_mem(unsigned int p_address, unsigned char p_value);
	inline void clear_memory(void);
	void initialize();
	bool load_program(uint16_t* pProgram, size_t bufferSize);
	uint16_t getProgramOpcode();
	inline void step_PC();
	void emulateCycle();		//run c clock tick
	void SampleInput();

	bool g_bRunning = true;
	//The Valmac has around 35 opcodes which are all two bytes long. 
	uint16_t opcode;

	////4K memory in total
	// NB 1K is 1024 bytes (not 1000)
	uint8_t memory[MEMORY_SIZE];


	uint8_t  V[16];
	uint16_t I;
	uint16_t PC;

	uint8_t gfx[64 * 32];

	uint8_t delay_timer;
	uint8_t sound_timer;


	uint16_t stack[STACK_SIZE];
	uint16_t SP;
	uint16_t MasterMind[3]
	{
		//TEST01
		0xFF18,
		0xC208,
		0xC308

		//Test02
		//0xF015,
		//0xF018,
		//0xF01E

		//Test03
		//0xF055,
		//0xF065,
		//0xF007

		//Test04
		//0xE59E,
		//0x9000,
		//0xB000

		//Test05
		//0x6000,
		//0x3AAA,
		//0x0000

	};


	uint8_t keypad[16];
	/*
		1	2	3	C
		4	5	6	D
		7	8	9	E
		A	0	B	F
	*/

	sf::RenderWindow window;

};
