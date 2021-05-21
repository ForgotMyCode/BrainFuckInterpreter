#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define TAPE_SIZE 30000

#define ARGS byte** dataPointer, int* instructionPointer, int extra

typedef unsigned char byte;

typedef struct {
	void (*instructionExecutor)(ARGS);
	int extra;
} Instruction;

void INSTRUCT_incrementDataPointer(ARGS) {
	++*dataPointer;
}

void INSTRUCT_decrementDataPointer(ARGS) {
	--*dataPointer;
}

void INSTRUCT_increment(ARGS) {
	++**dataPointer;
}

void INSTRUCT_decrement(ARGS) {
	--**dataPointer;
}

void INSTRUCT_conditionalJumpForwards(ARGS) {
	if (**dataPointer == 0) {
		*instructionPointer += extra;
	}
}

void INSTRUCT_conditionalJumpBackwards(ARGS) {
	if (**dataPointer) {
		*instructionPointer -= extra;
	}
}

void INSTRUCT_out(ARGS) {
	putchar(**dataPointer);
}

void INSTRUCT_in(ARGS) {
	int in = getchar();
	assert(in != EOF);

	**dataPointer = (byte) in;
}

bool isInstruction(char c) {
	return 	c == '>' || c == '<' || c == '+' || c == '-' || c == '.' || c == ',' || c == '[' || c == ']';
}

int seekOpeningMatch(char* instructions, int offset, int nInstructions) {
	int level = 0;
	int comments = 0;

	for (int i = offset; i >= 0; --i) {
		if (instructions[i] == '[') {
			if (level == 1) {
				return offset - i - comments;
			}
			// else
			--level;
		}

		if (instructions[i] == ']') {
			++level;
		}

		if (!isInstruction(instructions[i])) {
			++comments;
		}

	}
	return 0;
}

int seekClosingMatch(char* instructions, int offset, int nInstructions) {
	int level = 0;
	int comments = 0;

	for (int i = offset; i < nInstructions; ++i) {
		if (instructions[i] == ']') {
			if (level == 1) {
				return i - offset - comments;
			}
			// else
			--level;
		}

		if (instructions[i] == '[') {
			++level;
		}

		if (!isInstruction(instructions[i])) {
			++comments;
		}
	}
	return 0;
}

void compile(char* instructions, Instruction* compiledInstructions, long* nInstructionsPtr) {
	int nInstructions = (int) *nInstructionsPtr;
	for (int i = 0, j = 0; i < nInstructions; ++i, ++j) {
		const char instruction = instructions[i];

		if (instruction == '>') {
			compiledInstructions[j].instructionExecutor = INSTRUCT_incrementDataPointer;
		}
		else if (instruction == '<') {
			compiledInstructions[j].instructionExecutor = INSTRUCT_decrementDataPointer;	
		}
		else if (instruction == '+') {
			compiledInstructions[j].instructionExecutor = INSTRUCT_increment;			
		}
		else if (instruction == '-') {
			compiledInstructions[j].instructionExecutor = INSTRUCT_decrement;
		}
		else if (instruction == '.') {
			compiledInstructions[j].instructionExecutor = INSTRUCT_out;
		}
		else if (instruction == ',') {
			compiledInstructions[j].instructionExecutor = INSTRUCT_in;
		}
		else if (instruction == '[') {
			int extra = seekClosingMatch(instructions, i, nInstructions);
			assert(extra);

			compiledInstructions[j].instructionExecutor = INSTRUCT_conditionalJumpForwards;
			compiledInstructions[j].extra = extra;
		}
		else if (instruction == ']') {
			int extra = seekOpeningMatch(instructions, i, nInstructions);
			assert(extra);

			compiledInstructions[j].instructionExecutor = INSTRUCT_conditionalJumpBackwards;
			compiledInstructions[j].extra = extra;
		}
		else {
			--*nInstructionsPtr;
			--j;
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("One argument required!");
		exit(0);
	}

	FILE* f = fopen(argv[1], "rb");
	assert(f);

	fseek(f, 0L, SEEK_END);
	const long nInstructions = ftell(f);
	fseek(f, 0L, SEEK_SET);

	char* instructions = calloc(nInstructions, sizeof(char));
	byte* tape = calloc(TAPE_SIZE, sizeof(byte));
	Instruction* compiledInstructions = calloc(nInstructions, sizeof(Instruction));
	assert(instructions && tape && compiledInstructions);

	fread(instructions, sizeof(*instructions), nInstructions, f);

	fclose(f);

	compile(instructions, compiledInstructions, &nInstructions);

	free(instructions);

	byte* dataPointer = tape + (TAPE_SIZE / 2);

	for (int instructionIndex = 0; instructionIndex < nInstructions; ++instructionIndex) {
		compiledInstructions[instructionIndex].instructionExecutor(
			&dataPointer,
			&instructionIndex,
			compiledInstructions[instructionIndex].extra
		);
	}

	free(tape);

	free(compiledInstructions);

	return 0;
}
