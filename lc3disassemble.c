#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

enum OPCODES {
	OP_BR,//
	OP_ADD,//
	OP_LD,//
	OP_ST,//
	OP_JSR,//
	OP_AND,//
	OP_LDR,//
	OP_STR,//
	OP_RTI,//
	OP_NOT,//
	OP_LDI,//
	OP_STI,//
	OP_RET,//
	OP_ILLEGAL,
	OP_LEA,//
	OP_TRAP
};

enum TRAP_VECTORS {
	GETC = 0X20,
	OUT,
	PUTS,
	IN,
	PUTSP,
	HALT
};

char *registers[] = {"R0", "R1", "R2", "R3", \
					 "R4", "R5", "R6", "R7"};
					 
void disassemble(uint16_t);
void printbin(uint16_t);
int read_negative(int, int);
void print_fill(uint16_t);

FILE *f;
uint16_t pc = 0x0;
int origin = 0x0;
int counter = 0x0;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: lc3disassemble <filename>\n");
		return -1;
	}
	if ((f = fopen(*++argv, "rba")) == NULL) {
		printf("File not found\n");
		return 1;
	}
	
	size_t fsize;
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	unsigned char *buffer = (unsigned char *) malloc(fsize);
	fread(buffer, fsize, 1, f);
	fclose(f);

	uint16_t left, right, full;
	left = buffer[pc++];
	right = buffer[pc++];
	origin += (left << 8) | right;
	printf("\n.ORIG x%.4x\n\n", origin);
	
	while (pc < fsize) {
		//printf("mem address: %x", origin);
		++counter;
		left = buffer[pc++];
		right = buffer[pc++];
		full = (left << 8) | right;
		printbin(full);
		disassemble(full);
		//printf("\t%.4x\n", full);
	}
	free(buffer);
	return 0;
}

void disassemble(uint16_t n) {
	int instr = n;
	int op = (instr >> 12) & 0xf;
	switch (op) {
		case OP_BR: {
			int nzp = (instr >> 9) & 0x7;
			switch (nzp) {
				case 1: 	// positive only
					printf("\tBNp\n");
					break;
				case 2: 	// zero flag only
					printf("\tBNz\n");
					break;
				case 3: 	// zero or positive 
					printf("\tBNzp\n");
					break;
				case 4: 	// negative only
					printf("\tBNn\n");
					break;
				case 5:		// neg or pos
					printf("\tBNnp\n");
					break;
				case 6: 	// neg or zero
					printf("\tBNnz");
					break;
				case 7:		// unconditional, or nzp 
					printf("\tBN\n");
					break;
				case 0:
				/* if no bits are set, then it's not a BR, but
				 * instead is data */
					print_fill(instr);
					//print_fill(instr);
					break;
			} // end nzp switch
			break;
		}
		case OP_ADD: {
			uint16_t dr = (instr >> 9) & 0x7;
			uint16_t sr1 = (instr >> 6) & 0x7;
			uint16_t imm_flag = (instr >> 5) & 0x1;
			if (imm_flag) { // if bit 5 is set to one
				int imm5 = instr & 0x1f;
				// if the 5th bit of imm5 is set, then it's a negative
				if ((imm5 >> 4) & 0x1) {
					imm5 = read_negative(imm5, 5);
				}
				printf("\tADD %s, %s, #%d\n", registers[dr], registers[sr1], imm5);
			} else {	// if bit 5 set to zero 
				uint16_t sr2 = instr & 0x7;
				printf("\tADD %s, %s, %s\n", registers[dr], registers[sr1], registers[sr2]);	
			}
			break;
		}
		case OP_LD: {
			uint16_t dr = (instr >> 9) & 0x7;
			uint16_t pc_offset = (instr & 0x1ff) + counter + origin;
			printf("\tLD %s, %X\n", registers[dr], pc_offset);
			break;
		}
		case OP_ST: {
			uint16_t sr = (instr >> 9) & 0x7;
			uint16_t pc_offset = (instr & 0x1ff) + counter + origin;
			printf("\tST %s, %X\n", registers[sr], pc_offset + counter + origin);
			break;
		}
		case OP_JSR: {
			uint16_t flag = (instr >> 11) & 0x1;
			if (flag) {
				uint16_t pc_offset = (instr & 0x7ff) + counter + origin;
				printf("\tJSR %X\n", pc_offset);
			} else {
				uint16_t base_r = (instr >> 6) & 0x7;
				printf("\tJSRr %X\n", base_r);
			}
			break;
		}
		case OP_AND: {
			uint16_t dr = (instr >> 9) & 0x7;
			uint16_t sr1 = (instr >> 6) & 0x7;
			uint16_t flag = (instr >> 5) & 0x1;
			if (flag) {
				int16_t imm5 = instr & 0x1f;
				if ((imm5 >> 4) & 0x1) {
					imm5 = read_negative(imm5, 5);
				}
				printf("\tAND %s, %s, #%d\n", registers[dr], registers[sr1], imm5);
			} else {
				uint16_t sr2 = instr & 0x7;
				printf("\tAND %s, %s, %s\n", registers[dr], registers[sr1], registers[sr2]);
			}
			break;
		}
		case OP_LDR: {
			uint16_t dr = (instr >> 9) & 0x7;
			uint16_t base_r = (instr >> 6) & 0x7;
			uint16_t offset = instr & 0x3f;
			printf("\tLDR %s, %s, %d\n", registers[dr], registers[base_r], offset);
			break;
		}
		case OP_STR: {
			uint16_t sr = (instr >> 9) & 0x7;
			uint16_t base_r = (instr >> 6) & 0x7;
			uint16_t offset = instr & 0x3f;
			printf("\tSTR %s, %s, #%d\n", registers[sr], registers[base_r], offset);
			break;
		}
		case OP_RTI: {
			printf("\tRTI\n");
			break;
		}
		case OP_NOT: {
			uint16_t dr = (instr >> 9) & 0x7;
			uint16_t sr = (instr >> 6) & 0x7;
			printf("\tNOT %s, %s\n", registers[dr], registers[sr]);
			break;
		}
		case OP_LDI: {
			uint16_t dr = (instr >> 9) & 0x7;
			uint16_t pc_offset = (instr & 0x1ff) + counter + origin;
			printf("\tLDI %s, %X\n", registers[dr], pc_offset);
			break;
		}
		case OP_STI: {
			uint16_t sr = (instr >> 9) & 0x7;
			uint16_t pc_offset = (instr & 0x1ff) + counter + origin;
			printf("\tSTI %s, %X\n", registers[sr], pc_offset);
			break;
		}
		case OP_RET: {
			printf("\tRET\n");
			break;
		}
		case OP_ILLEGAL: {
			printf("ILLEGAL OPCODE!\n");
			break;
		}
		case OP_LEA: {
			uint16_t dr = (instr >> 9) & 0x7;
			uint16_t pc_offset = (instr & 0x1ff) + counter + origin;
			printf("\tLEA %s, %X\n", registers[dr], pc_offset);
			break;
		}
		case OP_TRAP: {
			// if bits 8-11 are set, then it isn't a trap instruction
			if ((instr >> 8) & 0xff) {
				print_fill(instr);
				break;
			}
			uint16_t trap_vect = instr & 0xff;
			printf("\tTRAP %X ", trap_vect);
			switch (trap_vect) {
				case GETC:
					printf("GETC\n");
					break;
				case OUT:
					printf("OUT\n");
					break;
				case PUTS:
					printf("PUTS\n");
					break;
				case IN: 
					printf("IN\n");
					break;
				case PUTSP:
					printf("PUTSP\n");
					break;
				case HALT:
					printf("HALT\n");
					break;
				default:
					printf("Illegal trap vector!\n");
					break;
			} // end switch(trap_vect)
			break;
		}
	}// end switch(op)
}

void print_fill(uint16_t instr) {
	uint16_t data = instr & 0xffff;
	printf("\t.FILL x%.4X\n", data);
}

void printbin(uint16_t n) {
	int single_n;	// each single digit in the 4 digit hex number
	int idx;
	char bin[5];	//binary string 
	for (int i=3; i>=0; i--) {
		idx = 4;
		single_n = (n >> i*4) & 0xf;
		while (idx > 0) {
			bin[--idx] = (single_n & 1) + '0';
			single_n >>= 1;
		}
		bin[4] = '\0';
		printf("%s ", bin);
	}// end for 
}

int read_negative(int n, int len) {
	int neg_num = pow(2, len-1) * (-1);
	int digit;
	for (int i=len-2; i>=0; i--) {
		digit = (n >> i) & 0x1;
		neg_num += digit * pow(2, i);
	}
	return neg_num;
}
