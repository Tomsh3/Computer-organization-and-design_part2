
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <xc.h>
#include <sys/attribs.h>
#include "config.h"
#include "lcd.h"
#include "led.h"
#include "btn.h"
#include "swt.h"
#include "ssd.h"


#pragma config JTAGEN = OFF     
#pragma config FWDTEN = OFF      


/* ------------------------------------------------------------ */
/*						Configuration Bits		                */
/* ------------------------------------------------------------ */


// Device Config Bits in  DEVCFG1:	
#pragma config FNOSC =	PRIPLL
#pragma config FSOSCEN =	OFF
#pragma config POSCMOD =	XT
#pragma config OSCIOFNC =	ON
#pragma config FPBDIV =     DIV_1

// Device Config Bits in  DEVCFG2:	
#pragma config FPLLIDIV =	DIV_2
#pragma config FPLLMUL =	MUL_20
#pragma config FPLLODIV =	DIV_1

/* ------------------------------------------------------------ */
/*						     Globals        	                */
/* ------------------------------------------------------------ */

// Definitions:
#define MAX_LINE 8 // maximum line length in memin.txt.
#define MAX_LABEL 50 // the maximum lable size is 50 and .
#define MAX_MEMO_LINES 512 // maximum number of lines in the memory.
#define NUM_OF_REG 16 // number of registers.
#define NUM_OF_IOREG 14 // number of IO registers.

// Structures:
typedef struct Format
{
    int opcode; // 8 bits
    int rd; // 4 bits
    int rs; // 4 bits
    int rt; // 4 bits
    int imm; // 12 bits.
}Format;

// Variables:
int PC = 0; // program counter
unsigned int rs_unsigned = 0; // used for srl in execute function
int BTNL = 0; // indicates BTNL was pressed when value is 1, else 0
int BTNU = 0; // indicates BTNU was pressed when value is 1, else 0
int BTNR = 0; // indicates BTNR was pressed when value is 1, else 0
int BTND = 0; // indicates BTND was pressed when value is 1, else 0
int BTNC = 0; // indicates BTNC was pressed when value is 1, else 0
int halt_ind = 0; // indicates if HALT command has been activated
int irq_ongoing_ind = 0; // 0 if not performing irq, 1 if currently performing an irq
int pause_ind = 0; // 1 if PAUSE is activated, 0 else
int step_ind = 0; // 1 if SINGLE STEP activated, 0 else
int second_ind = 0; // 1 if a seconds has passed in the current iteration, 0 else
int execute_allow = 1; // 1 when the iteration will include performing a assembly command, 0 else
int current_time = 0; // buffer for current time in each iteration
int current_LED = 7; // indicator of the current led that has been lit, incremented when "reti" command is executed
int LCD_mem_index = 0; // used for "lcd_update" function to keep the current memory address which is shown in the lcd
int LCD_reg_index = 0; //  used for "lcd_update" function to keep the current register number which is shown in the lcd
int register_arr[NUM_OF_REG] = { 0 }; //  16 registers 
int IO_register_arr[NUM_OF_IOREG] = { 0 }; // 14 IO registers
int memory[MAX_MEMO_LINES] = { 0 }; // memory initiation
char memin_arr[MAX_MEMO_LINES][MAX_LINE]; // memin initiation

Format inst = { 0 }; // instruction 
Format* ptr_inst = &inst; // instruction's address

// Timing definitions
#define CLKS_IN_SECOND (8000000*10/2)
#define CLKS_IN_MILLISECOND (CLKS_IN_SECOND / 1000)
#define SECONDS_IN_DAY (24*3600)
#define COMMAND_REQUIRED_TIME (40000000 / 1130)

// Last 32 bit clock counter sample
unsigned int clk32 = 0;
char clk_buffer[80];


// We maintain this counter in software by polling the 32 bit hardware
// counter frequently, detecting wrap-around, and maintaining the high
// 32 bits in software.
unsigned int clk32_prev = 0;
unsigned int clk64_msb = 0;
unsigned long long clk64;



/* ------------------------------------------------------------ */
/*                       Functions Declaration                  */
/* ------------------------------------------------------------ */

void build_instruction(int PC, Format* ptr_inst, char memin_arr[MAX_MEMO_LINES][MAX_LINE]);
int execute(Format* ptr_inst, int memory[MAX_MEMO_LINES]);
void pause_check();
void initiation();
void lcd_update(int BTNL, int seconds, int memory[MAX_MEMO_LINES]);
void led_update();
static void clk_poll(void);
void btn_check();
void cycles();
void irq_update();
void ssd_update();
void memory_cpy(int source[], int destination[], int size);
void memin_cpy(char source[MAX_MEMO_LINES][MAX_LINE], char destination[MAX_MEMO_LINES][MAX_LINE], int rows, int columns);

/* ------------------------------------------------------------ */
/*                              Main           	                */
/* ------------------------------------------------------------ */

int main(int argc, char** argv)
{
    
	initiation();
    //check which program to run, decided by SW7 status:
    if (SWT_GetValue(7) == 0) // if SW7 is off -> load "fib" memin
    {
        int memory_fib[MAX_MEMO_LINES] = { 0x00D01001, 0x04DD1007, 0x0E301020, 0x0D100006, 0x0F201021, 0x13000000, 0x00DD1FFD, 0x0F9D1002, 0x0FFD1001, 0x0F3D1000, 0x00501001, 0x0A13500E, 0x00230000, 0x07100018, 0x01331001, 0x0D100006, 0x00921000, 0x0E3D1000, 0x01331002, 0x0D100006, 0x00229000, 0x0E3D1000, 0x0EFD1001, 0x0E9D1002, 0x00DD1003, 0x07F00000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000007 }; // memory size
        char memin_arr_fib[MAX_MEMO_LINES][MAX_LINE] = { "00D01001", "04DD1007", "0E301020", "0D100006", "0F201021", "13000000", "00DD1FFD", "0F9D1002", "0FFD1001", "0F3D1000", "00501001", "0A13500E", "00230000", "07100018", "01331001", "0D100006", "00921000", "0E3D1000", "01331002", "0D100006", "00229000", "0E3D1000", "0EFD1001", "0E9D1002", "00DD1003", "07F00000", "00000000", "00000000", "00000000", "00000000", "00000000", "00000000", "00000007" };
        memory_cpy(memory_fib, memory, MAX_MEMO_LINES); // copy fib memory to the global array
        memin_cpy(memin_arr_fib, memin_arr, MAX_MEMO_LINES, MAX_LINE); // copy fib memin to the global array
    }

    else // if SW7 is on -> load "stopper" memin
	{
        int memory_stopper[MAX_MEMO_LINES] = {0x007013FF, 0x1270100D, 0x00701001, 0x1270100B, 0x00901000, 0x00A00000, 0x00B00000, 0x00300000, 0x00400000, 0x11501003, 0x0710500E, 0x0050101A, 0x12501006, 0x12100001, 0x11501004, 0x07105013, 0x00501043, 0x12501006, 0x12101001, 0x11501005, 0x07105009, 0x00501049, 0x12501006, 0x00501001, 0x12501002, 0x07100009, 0x00701009, 0x07179029, 0x00991001, 0x0710001E, 0x00403000, 0x04441004, 0x0044B000, 0x04441004, 0x0044A000, 0x04441004, 0x00449000, 0x1240100A, 0x12001003, 0x12000000, 0x10000000, 0x00701005, 0x0717A02E, 0x00AA1001, 0x00901000, 0x0710001E, 0x00701009, 0x0717B034, 0x00BB1001, 0x00901000, 0x00A01000, 0x0710001E, 0x00701005, 0x0717303B, 0x00331001, 0x00901000, 0x00A01000, 0x00B01000, 0x0710001E, 0x1200100A, 0x00901000, 0x00A00000, 0x00B00000, 0x00300000, 0x00400000, 0x12001003, 0x10000000, 0x12001004, 0x11501004, 0x07105044,    0x12001004, 0x12001001, 0x10000000, 0x12001005, 0x1200100A, 0x00901000, 0x00A00000, 0x00B00000, 0x00300000, 0x00400000, 0x12001005, 0x12001002, 0x10000000, 0x00000000};
        char memin_arr_stopper[MAX_MEMO_LINES][MAX_LINE] = {"007013FF", "1270100D", "00701001", "1270100B", "00901000", "00A00000", "00B00000", "00300000", "00400000", "11501003", "0710500E", "0050101A", "12501006", "12100001", "11501004", "07105013", "00501043", "12501006", "12101001", "11501005", "07105009", "00501049", "12501006", "00501001", "12501002", "07100009", "00701009", "07179029", "00991001", "0710001E", "00403000", "04441004", "0044B000", "04441004", "0044A000", "04441004", "00449000", "1240100A", "12001003", "12000000", "10000000", "00701005", "0717A02E", "00AA1001", "00901000", "0710001E", "00701009", "0717B034", "00BB1001", "00901000", "00A01000", "0710001E", "00701005", "0717303B", "00331001", "00901000", "00A01000", "00B01000", "0710001E", "1200100A", "00901000", "00A00000", "00B00000", "00300000", "00400000", "12001003", "10000000", "12001004", "11501004", "07105044",    "12001004", "12001001", "10000000", "12001005", "1200100A", "00901000", "00A00000", "00B00000", "00300000", "00400000", "12001005", "12001002", "10000000", "00000000"};   
        memory_cpy(memory_stopper, memory, MAX_MEMO_LINES); // copy stopper memory to the global array
        memin_cpy(memin_arr_stopper, memin_arr, MAX_MEMO_LINES, MAX_LINE); // copy stopper memin to the global array
    }

    int test_execution = 0; //Jump Indicator
    unsigned long long seconds = 0, last_seconds = 0, resetval = 0, last_command_time = 0; // used for counting seconds  
    while (1) {  // infinite loop
        clk_poll(); // update clk64 according to the microchip's clock
        seconds = clk64 / CLKS_IN_SECOND;
        int beginning_time = clk64; // used later to determine how long each assembly command is being processed

        if (seconds != last_seconds) // if a second has passed
        {
            second_ind = 1;
            execute_allow = 1;
            last_seconds = seconds;
            LCD_DisplayClear(); // clear the lcd before updating it
            DelayAprox10Us(1000); // delay required between lcd clear to lcd wright
            lcd_update(BTNL, seconds, memory);
        }
        
        if (IO_register_arr[8]%2 == 1) // rate of ssd update 
        {
            ssd_update();
        }
        
        irq_update();
        btn_check();
        pause_check();
        
        if (((IO_register_arr[8] % 1024) == 0) && (IO_register_arr[8] != 0)) // check if 1024 commands has been executed
        {
            if (second_ind != 1) // if they were executed in less than 1 second - don't proceed to next command
            {
                execute_allow = 0;
            }
            else // else - proceed to next command and check each iteration if a second has passed, by using "second_ind"
            {
                second_ind = 0;
            }
        }    
        
        // if PAUSE is activated or HALT was executed or it's not the time to execute a command (execute_allow) -> then don't execute)
        if (((pause_ind == 1) && (step_ind == 0)) || (halt_ind == 1) || (execute_allow == 0)) 
            continue;
        
        step_ind = 0; // reset indicator, SINGLE STEP was performed
        
        // execute command and update cycles:
        build_instruction(PC, ptr_inst, memin_arr);
        test_execution = execute(ptr_inst, memory);
        cycles();
        
        // control minimum time of assembly command execution
        do
        {
            clk_poll();
            current_time = clk64; 
        }
        while((current_time - beginning_time) < COMMAND_REQUIRED_TIME);
            
        if (test_execution)
            continue; // the execution did a jump to another place so we don't need to promote PC
        
        PC++; // if there wasn't a jump - increment PC
    }
	return (0);
}

/* ------------------------------------------------------------ */
/*                            Functions       	                */
/* ------------------------------------------------------------ */

// This function copies the chosen memory by SW7 to the global array
void memory_cpy(int source[], int destination[], int size)
{
    for (int i = 0; i < size; i++) 
    {
        destination[i] = source[i];
    }
}

// This function copies the chosen memin by SW7 to the global array
void memin_cpy(char source[MAX_MEMO_LINES][MAX_LINE], char destination[MAX_MEMO_LINES][MAX_LINE], int rows, int columns)
{
    for (int i = 0; i < rows; i++) 
    {
        for (int j = 0; j < columns; j++) 
        {
            strcpy(destination[i], source[i]);
        }
    }
}

// This function updates the SSD (second display) according to display7seg register
void ssd_update()
{
    SSD_WriteDigitsGrouped(IO_register_arr[10], 0);
}

// This function updates irq registers according to irqstatus and irqenable
void irq_update()
{
    if (BTNC == 1)
    {
        IO_register_arr[4] = 1; // irq1status = 1
        BTNC = 0; // reset indicator to detect next activation
    }
    
    if (BTND == 1)
    {
        IO_register_arr[5] = 1; // irq2status = 1
        BTND = 0; // reset indicator to detect next activation
    }
    
    // check if one of the irq's enable and status are "1" at the same time, if so and there is no ongoing irq - jump to irq
    int irq_ind = ((IO_register_arr[0] && IO_register_arr[3]) || (IO_register_arr[1] && IO_register_arr[4]) || (IO_register_arr[2] && IO_register_arr[5]));
    if ((irq_ind == 1) && (irq_ongoing_ind == 0))
    {
        IO_register_arr[7] = PC; // load PC to irqreturn register
        PC = IO_register_arr[6]; // load irqhandler register's value to PC
        irq_ongoing_ind = 1; // indicates that there is a ongoing irq
    }
    else
        return;
}

// This function updates IORegisters: "clks" and "timercurrent" according to "timermax"
void cycles()
{
    IO_register_arr[8]++; // increment clock counter
    if (IO_register_arr[11] == 1)
    {
        IO_register_arr[12]++; // increment timercurrent
        if (IO_register_arr[12] == IO_register_arr[13]) // if timercurrent reached timermax then restart the timer
        {
            IO_register_arr[12] = 0; // reset timercurrent
            IO_register_arr[3] = 1; // request irq0 has been called to - a second has passed
        }
    }
}

// This function checks if a button has been pressed
void btn_check()
{
    if (BTN_GetValue('L') == 1) // check if BTNL was pressed
    {
        DelayAprox10Us(7000);
        if (BTN_GetValue('L') == 1) // check again that BTNL is pressed after a delay to make sure it's not noise
            BTNL = 1;
    }
    if (BTN_GetValue('R') == 1) // check if BTNR was pressed
    {
        DelayAprox10Us(7000);
        if (BTN_GetValue('R') == 1) // check again that BTNR is pressed after a delay to make sure it's not noise
            BTNR = 1;
    }
    if (BTN_GetValue('U') == 1) // check if BTNU was pressed
    {
        DelayAprox10Us(7000);
        if (BTN_GetValue('U') == 1) // check again that BTNU is pressed after a delay to make sure it's not noise
            BTNU = 1;
    }
    if (BTN_GetValue('D') == 1) // check if BTND was pressed
    {
        DelayAprox10Us(7000);
        if (BTN_GetValue('D') == 1) // check again that BTND is pressed after a delay to make sure it's not noise
            BTND = 1;
    }
    if (BTN_GetValue('C') == 1) // check if BTNC was pressed
    {
        DelayAprox10Us(7000);
        if (BTN_GetValue('C') == 1) // check again that BTNC is pressed after a delay to make sure it's not noise
            BTNC = 1;
    }
}

// This function executes "PAUSE" - return 1 if PAUSE is activated, return 2 if SINGLE STEP is activated, return 0 if PAUSE is deactivated
void pause_check()
{
    if (((BTNL == 1) && (pause_ind == 0)) || (((BTNL == 0) && (pause_ind == 1)) && (BTNR == 0))) // user clicked BTNL to activate PAUSE
    {
        pause_ind = 1;
        BTNL = 0;
    }
    if ((BTNL == 0) && (pause_ind == 1) && (BTNR == 1)) // user clicked BTNR while PAUSE to activate SINGLE STEP
    {
        BTNR = 0;
        step_ind = 1;
	}
    if (((BTNL == 1) && (pause_ind == 1)) || ((BTNL == 0) && (pause_ind == 0))) // user clicked BTNL to deactivate PAUSE or didn't click BTNL at all
    {
        BTNL = 0;
        pause_ind = 0;
    }
}

// This function builds the components of the commands (op,rt,rd,...) 
void build_instruction(int PC, Format* ptr_inst, char memin_arr[MAX_MEMO_LINES][MAX_LINE])
{
	int num;
	char op_code[3] = { memin_arr[PC][0],memin_arr[PC][1],'\0' }; // op_code will get 2 hexa digits
	ptr_inst->opcode = strtoul(op_code, NULL, 16); // convert the opcode to decimal number

	char rd[2] = { memin_arr[PC][2],'\0' };// rd will get 2 hexa digits
	ptr_inst->rd = strtoul(rd, NULL, 16); // convert the rd to decimal number

	char rs[2] = { memin_arr[PC][3],'\0' };// rs will get 2 hexa digits
	ptr_inst->rs = strtoul(rs, NULL, 16); // convert the rs to decimal number

	char rt[2] = { memin_arr[PC][4],'\0' };// rt will get 2 hexa digits
	ptr_inst->rt = strtoul(rt, NULL, 16); // convert the rt to decimal number

	char imm[4] = { memin_arr[PC][5],memin_arr[PC][6],memin_arr[PC][7],'\0' };// imm will get 3 hexa digits
	ptr_inst->imm = strtoul(imm, NULL, 16); // convert the imm to decimal number

	// check if rd / rs / rt is imm, if yes - load the imm to the Reg Array
	if (ptr_inst->rs == 1) { // rs is immidiate
		num = strtoul(imm, NULL, 16); // convert the imm to decimal and sign extend it
		// check if imm is a negative number:
		if ((imm[0] == 'F') || (imm[0] == 'E') || (imm[0] == 'D') || (imm[0] == 'C')\
			|| (imm[0] == 'B') || (imm[0] == 'A') || (imm[0] == '9') || (imm[0] == '8'))
		{
			num |= 0xfffff000;
		}
		ptr_inst->imm = num;
		register_arr[ptr_inst->rs] = ptr_inst->imm;
	}

	else if (ptr_inst->rt == 1) { // rt is immidiate
		num = strtoul(imm, NULL, 16); // convert the imm to decimal and sign extend it
		// check if imm is a negative number:
		if ((imm[0] == 'F') || (imm[0] == 'E') || (imm[0] == 'D') || (imm[0] == 'C')\
		|| (imm[0] == 'B') || (imm[0] == 'A') || (imm[0] == '9') || (imm[0] == '8')) 
		{ 
			num |= 0xfffff000;
		}
		ptr_inst->imm = num;
		register_arr[ptr_inst->rt] = ptr_inst->imm;
	}

	else if (ptr_inst->rd == 1) { // rd is immidiate
		num = strtoul(imm, NULL, 16); // convert the imm to decimal and sign extend it
		// check if imm is a negative number:
		if ((imm[0] == 'F') || (imm[0] == 'E') || (imm[0] == 'D') || (imm[0] == 'C')\
		|| (imm[0] == 'B') || (imm[0] == 'A') || (imm[0] == '9') || (imm[0] == '8'))
		{
			num |= 0xfffff000;
		}
		ptr_inst->imm = num;
		register_arr[ptr_inst->rd] = ptr_inst->imm;
	}
    
	else {//the instruction did not use imm, reset imm to 0
		register_arr[1] = 0;
	}
}

// This function identifies the opcode and executes the instruction (registers, memory, PC...) 
int execute(Format* ptr_inst, int memory[MAX_MEMO_LINES])
{
	unsigned int rs_unsigned = register_arr[ptr_inst->rs] &= 0x00000fff; // used later for srl
    
    switch (ptr_inst->opcode)
	{
	case 0x0: // add
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt];
		return 0;

	case 0x1: //sub
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] - register_arr[ptr_inst->rt];
		return 0;

	case 0x2: //and
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] & register_arr[ptr_inst->rt];
		return 0;

	case 0x3: //or
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] | register_arr[ptr_inst->rt];
		return 0;

	case 0x4: //sll
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] << register_arr[ptr_inst->rt];
		return 0;

	case 0x5: //sra
		register_arr[ptr_inst->rd] = register_arr[ptr_inst->rs] >> register_arr[ptr_inst->rt];
		return 0;

	case 0x6: //srl
		register_arr[ptr_inst->rd] = rs_unsigned >> register_arr[ptr_inst->rt];
		return 0;

	case 0x7: //beq
		if (register_arr[ptr_inst->rs] == register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0x8: //bne 
		if (register_arr[ptr_inst->rs] != register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0x9: //blt
		if (register_arr[ptr_inst->rs] < register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0xA: //bgt
		if (register_arr[ptr_inst->rs] > register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0xB: //ble
		if (register_arr[ptr_inst->rs] <= register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0xC: //bge
		if (register_arr[ptr_inst->rs] >= register_arr[ptr_inst->rt]) {
			PC = register_arr[ptr_inst->rd];
			return 1;
		}return 0;

	case 0xD: //jal
		register_arr[15] = PC + 1;
		PC = register_arr[ptr_inst->rd];
		return 1;

	case 0xE: //lw
		register_arr[ptr_inst->rd] = memory[(register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt]) % MAX_MEMO_LINES]; //considering the size limitation
		return 0;

	case 0xF: //sw;
		memory[(register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt]) % MAX_MEMO_LINES] = register_arr[ptr_inst->rd];
		return 0;

	case 0x13: //halt
		halt_ind = 1;;//will stop the program after this iteration while updating the LCD, SSD and clock
		return 1;

	case 0x10: //reti
		irq_ongoing_ind = 0; // request has ended
        PC = IO_register_arr[7];// Jump to the value stored in IOregister[7] (PC of interrupt return address)
        led_update(); // update leds due to reti command
		return 1;

	case 0x11: //in
		register_arr[ptr_inst->rd] = IO_register_arr[register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt]]; // Copy the value in IOregister[ R[rs] + R[rt] ] into R[rd]
		return 0;

	case 0x12: //out
		IO_register_arr[register_arr[ptr_inst->rs] + register_arr[ptr_inst->rt]] = register_arr[ptr_inst->rd]; // Copy the value in R[rd] into IOregister[ R[rs] + R[rt] ] 
		return 0;
	}
}

// This function initiates the microchip's IO modules
void initiation()
{
	// libraries initialization for the IO modules
    LCD_Init(); 
    LED_Init(); 
    SWT_Init();
    BTN_Init();
    SSD_Init();
}

// This function updates the data that the LCD displays according to SW0 and SW1
void lcd_update(int BTNL, int seconds, int memory[MAX_MEMO_LINES])
{
    // update second line:

    char PC_buffer[3];
    char RSP_buffer[3];
    char TIME_buffer[8];
    
    sprintf(PC_buffer, "%03x", PC); // convert pc to hexa and store it in a buffer
    LCD_WriteStringAtPos(PC_buffer, 1, 0);
    
    sprintf(RSP_buffer, "%03x", register_arr[13]); // convert $sp content to hexa and store it in a buffer
    LCD_WriteStringAtPos(RSP_buffer, 1, 4);
    
    sprintf(TIME_buffer, "%08d", seconds); // convert seconds content to hexa and store it in a buffer 
    LCD_WriteStringAtPos(TIME_buffer, 1, 8);
    
    // update first line:
    
    // SW0=OFF and SW1=OFF:
    if (SWT_GetValue(0) == 0 && SWT_GetValue (1) == 0) 
    {
        char MEMORY_buffer[8];
        sprintf(MEMORY_buffer, "%08x", memory[PC]); // convert memory[PC] to hexa and store it in a buffer
        LCD_WriteStringAtPos(MEMORY_buffer, 0, 0); // display next instruction to be executed in the lcd
    }
    
    // SW0=OFF and SW1=OFF
    if (SWT_GetValue(0) == 1 && SWT_GetValue (1) == 0)
    {
        char hex_i[2];
        char hex_reg[8];
        sprintf(hex_i, "%02x", LCD_reg_index); // convert register number to hexa
        sprintf(hex_reg, "%08x", register_arr[LCD_reg_index]); // convert register value to hexa
        
        // display the register and its value 
        LCD_WriteStringAtPos("R", 0, 0);
        LCD_WriteStringAtPos(hex_i, 0, 1);
        LCD_WriteStringAtPos("=", 0, 4);
        LCD_WriteStringAtPos(hex_reg, 0, 6);
        if (BTNU == 1) // if BTNU was pressed show next register
        {
            LCD_reg_index++;
            LCD_reg_index = LCD_reg_index%16; // if currently watching R15 and clicked "BTNU" the next register shown will be R0
            BTNU = 0; // restart indicator to detect next click on BTNU
        }
    }
    
    // SW0=OFF and SW1=OFF
    if (SWT_GetValue(0) == 0 && SWT_GetValue (1) == 1) 
    {
        char hex_i[3];
        char hex_mem[8];
        sprintf(hex_i, "%03x", LCD_mem_index);
        sprintf(hex_mem, "%08x", memory[LCD_mem_index]);
        
        // display the memory address and its data 
        LCD_WriteStringAtPos("M", 0, 0);
        LCD_WriteStringAtPos(hex_i, 0, 1);
        LCD_WriteStringAtPos("=", 0, 5);
        LCD_WriteStringAtPos(hex_mem, 0, 7);
        
        if (BTNU == 1) // if BTNU was pressed show next address
        {
            LCD_mem_index++;
            LCD_mem_index = LCD_mem_index%512; // if currently watching M[511] and clicked "BTNU" the next memory address shown will be 0
            BTNU = 0; // restart indicator to detect next click on BTNU
        } 
    }
    
    // SW0=OFF and SW1=OFF
    if (SWT_GetValue(0) == 1 && SWT_GetValue (1) == 1) 
    {
        char CYCLES_buffer[16];
        sprintf(CYCLES_buffer, "%08x", IO_register_arr[8]); // convert clk [number of cycles] to hexa and store it in a buffer
        LCD_WriteStringAtPos(CYCLES_buffer, 0, 0); // display current number of clock cycles
    }
    
}

// This function makes sure that the right led is on when a "reti" command is encountered
void led_update()
{
        LED_SetValue(current_LED, 0); // Turn off old LD 
        current_LED++;
        current_LED = (current_LED) % 8;
        LED_SetValue(current_LED, 1); // Turn on current LD         
}

// This function updates clk62 to the current number of clock cycles of the microchip that has been passed since last "RESET"
static void clk_poll(void)
{
    clk32 = _CP0_GET_COUNT();

	if (clk32 < clk32_prev) {
		clk64_msb++;
	}
	clk32_prev = clk32;
	clk64 = (((unsigned long long) clk64_msb) << 32) | clk32;
}

