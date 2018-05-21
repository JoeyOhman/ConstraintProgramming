#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

class Sudoku : public Script {
public:
	IntVarArray numbers;
	enum {
		BRANCH_FIRSTFAIL,
		BRANCH_MIDDLEVALUE
	};
	int example[9][9] = {
	{ 0,0,0, 2,0,5, 0,0,0 },
	{ 0,9,0, 0,0,0, 7,3,0 },
	{ 0,0,2, 0,0,9, 0,6,0 },

	{ 2,0,0, 0,0,0, 4,0,9 },
	{ 0,0,0, 0,7,0, 0,0,0 },
	{ 6,0,9, 0,0,0, 0,0,1 },

	{ 0,8,0, 4,0,0, 1,0,0 },
	{ 0,6,3, 0,0,0, 0,8,0 },
	{ 0,0,0, 6,0,8, 0,0,0 }
	}/*
	{
	{ 3,0,0, 9,0,4, 0,0,1 },
	{ 0,0,2, 0,0,0, 4,0,0 },
	{ 0,6,1, 0,0,0, 7,9,0 },

	{ 6,0,0, 2,4,7, 0,0,5 },
	{ 0,0,0, 0,0,0, 0,0,0 },
	{ 2,0,0, 8,3,6, 0,0,4 },

	{ 0,4,6, 0,0,0, 2,3,0 },
	{ 0,0,9, 0,0,0, 6,0,0 },
	{ 5,0,0, 3,0,9, 0,0,8 }
	}
	{
	{ 0,0,0, 0,1,0, 0,0,0 },
	{ 3,0,1, 4,0,0, 8,6,0 },
	{ 9,0,0, 5,0,0, 2,0,0 },

	{ 7,0,0, 1,6,0, 0,0,0 },
	{ 0,2,0, 8,0,5, 0,1,0 },
	{ 0,0,0, 0,9,7, 0,0,4 },

	{ 0,0,3, 0,0,4, 0,0,6 },
	{ 0,4,8, 0,0,6, 9,0,7 },
	{ 0,0,0, 0,8,0, 0,0,0 }
	}
	{	// Fiendish puzzle April 21, 2005 Times London
	{ 0,0,4, 0,0,3, 0,7,0 },
	{ 0,8,0, 0,7,0, 0,0,0 },
	{ 0,7,0, 0,0,8, 2,0,5 },

	{ 4,0,0, 0,0,0, 3,1,0 },
	{ 9,0,0, 0,0,0, 0,0,8 },
	{ 0,1,5, 0,0,0, 0,0,4 },

	{ 1,0,6, 9,0,0, 0,3,0 },
	{ 0,0,0, 0,2,0, 0,6,0 },
	{ 0,2,0, 4,0,0, 5,0,0 }
	}
	{	// This one requires search
	{ 0,4,3, 0,8,0, 2,5,0 },
	{ 6,0,0, 0,0,0, 0,0,0 },
	{ 0,0,0, 0,0,1, 0,9,4 },

	{ 9,0,0, 0,0,4, 0,7,0 },
	{ 0,0,0, 6,0,8, 0,0,0 },
	{ 0,1,0, 2,0,0, 0,0,3 },

	{ 8,2,0, 5,0,0, 0,0,0 },
	{ 0,0,0, 0,0,0, 0,0,5 },
	{ 0,3,4, 0,9,0, 7,1,0 }
	}
	{	// Hard one from http://www.cs.mu.oz.au/671/proj3/node5.html
	{ 0,0,0, 0,0,3, 0,6,0 },
	{ 0,0,0, 0,0,0, 0,1,0 },
	{ 0,9,7, 5,0,0, 0,8,0 },

	{ 0,0,0, 0,9,0, 2,0,0 },
	{ 0,0,8, 0,7,0, 4,0,0 },
	{ 0,0,3, 0,6,0, 0,0,0 },

	{ 0,1,0, 0,0,2, 8,9,0 },
	{ 0,4,0, 0,0,0, 0,0,0 },
	{ 0,5,0, 1,0,0, 0,0,0 }
	}
	{ // Puzzle 1 from http://www.sudoku.org.uk/bifurcation.htm
	{ 1,0,0, 9,0,7, 0,0,3 },
	{ 0,8,0, 0,0,0, 0,7,0 },
	{ 0,0,9, 0,0,0, 6,0,0 },
	{ 0,0,7, 2,0,9, 4,0,0 },
	{ 4,1,0, 0,0,0, 0,9,5 },
	{ 0,0,8, 5,0,4, 3,0,0 },
	{ 0,0,3, 0,0,0, 7,0,0 },
	{ 0,5,0, 0,0,0, 0,4,0 },
	{ 2,0,0, 8,0,6, 0,0,9 }
	}
	{ // Puzzle 2 from http://www.sudoku.org.uk/bifurcation.htm
	{ 0,0,0, 3,0,2, 0,0,0 },
	{ 0,5,0, 7,9,8, 0,3,0 },
	{ 0,0,7, 0,0,0, 8,0,0 },
	{ 0,0,8, 6,0,7, 3,0,0 },
	{ 0,7,0, 0,0,0, 0,6,0 },
	{ 0,0,3, 5,0,4, 1,0,0 },
	{ 0,0,5, 0,0,0, 6,0,0 },
	{ 0,2,0, 4,1,9, 0,5,0 },
	{ 0,0,0, 8,0,6, 0,0,0 }
	}
	{ // Puzzle 3 from http://www.sudoku.org.uk/bifurcation.htm
	{ 0,0,0, 8,0,0, 0,0,6 },
	{ 0,0,1, 6,2,0, 4,3,0 },
	{ 4,0,0, 0,7,1, 0,0,2 },
	{ 0,0,7, 2,0,0, 0,8,0 },
	{ 0,0,0, 0,1,0, 0,0,0 },
	{ 0,1,0, 0,0,6, 2,0,0 },
	{ 1,0,0, 7,3,0, 0,0,4 },
	{ 0,2,6, 0,4,8, 1,0,0 },
	{ 3,0,0, 0,0,5, 0,0,0 }
	}
	{ // Puzzle 4 from http://www.sudoku.org.uk/bifurcation.htm
	{ 3,0,5, 0,0,4, 0,7,0 },
	{ 0,7,0, 0,0,0, 0,0,1 },
	{ 0,4,0, 9,0,0, 0,3,0 },
	{ 4,0,0, 0,5,1, 0,0,6 },
	{ 0,9,0, 0,0,0, 0,4,0 },
	{ 2,0,0, 8,4,0, 0,0,7 },
	{ 0,2,0, 0,0,7, 0,6,0 },
	{ 8,0,0, 0,0,0, 0,9,0 },
	{ 0,6,0, 4,0,0, 2,0,8 }
	}
	{ // Puzzle 5 from http://www.sudoku.org.uk/bifurcation.htm
	{ 0,0,0, 7,0,0, 3,0,0 },
	{ 0,6,0, 0,0,0, 5,7,0 },
	{ 0,7,3, 8,0,0, 4,1,0 },
	{ 0,0,9, 2,8,0, 0,0,0 },
	{ 5,0,0, 0,0,0, 0,0,9 },
	{ 0,0,0, 0,9,3, 6,0,0 },
	{ 0,9,8, 0,0,7, 1,5,0 },
	{ 0,5,4, 0,0,0, 0,6,0 },
	{ 0,0,1, 0,0,9, 0,0,0 }
	}
	{ // Puzzle 6 from http://www.sudoku.org.uk/bifurcation.htm
	{ 0,0,0, 6,0,0, 0,0,4 },
	{ 0,3,0, 0,9,0, 0,2,0 },
	{ 0,6,0, 8,0,0, 7,0,0 },
	{ 0,0,5, 0,6,0, 0,0,1 },
	{ 6,7,0, 3,0,1, 0,5,8 },
	{ 9,0,0, 0,5,0, 4,0,0 },
	{ 0,0,6, 0,0,3, 0,9,0 },
	{ 0,1,0, 0,8,0, 0,6,0 },
	{ 2,0,0, 0,0,6, 0,0,0 }
	}
	{ // Puzzle 7 from http://www.sudoku.org.uk/bifurcation.htm
	{ 8,0,0, 0,0,1, 0,4,0 },
	{ 2,0,6, 0,9,0, 0,1,0 },
	{ 0,0,9, 0,0,6, 0,8,0 },
	{ 1,2,4, 0,0,0, 0,0,9 },
	{ 0,0,0, 0,0,0, 0,0,0 },
	{ 9,0,0, 0,0,0, 8,2,4 },
	{ 0,5,0, 4,0,0, 1,0,0 },
	{ 0,8,0, 0,7,0, 2,0,5 },
	{ 0,9,0, 5,0,0, 0,0,7 }
	}
	{ // Puzzle 8 from http://www.sudoku.org.uk/bifurcation.htm
	{ 6,5,2, 0,4,8, 0,0,7 },
	{ 0,7,0, 2,0,5, 4,0,0 },
	{ 0,0,0, 0,0,0, 0,0,0 },
	{ 0,6,4, 1,0,0, 0,7,0 },
	{ 0,0,0, 0,8,0, 0,0,0 },
	{ 0,8,0, 0,0,4, 5,6,0 },
	{ 0,0,0, 0,0,0, 0,0,0 },
	{ 0,0,8, 6,0,7, 0,2,0 },
	{ 2,0,0, 8,9,0, 7,5,1 }
	}
	{ // Puzzle 9 from http://www.sudoku.org.uk/bifurcation.htm
	{ 0,0,6, 0,0,2, 0,0,9 },
	{ 1,0,0, 5,0,0, 0,2,0 },
	{ 0,4,7, 3,0,6, 0,0,1 },
	{ 0,0,0, 0,0,8, 0,4,0 },
	{ 0,3,0, 0,0,0, 0,7,0 },
	{ 0,1,0, 6,0,0, 0,0,0 },
	{ 4,0,0, 8,0,3, 2,1,0 },
	{ 0,6,0, 0,0,1, 0,0,4 },
	{ 3,0,0, 4,0,0, 9,0,0 }
	}
	{ // Puzzle 10 from http://www.sudoku.org.uk/bifurcation.htm
	{ 0,0,4, 0,5,0, 9,0,0 },
	{ 0,0,0, 0,7,0, 0,0,6 },
	{ 3,7,0, 0,0,0, 0,0,2 },
	{ 0,0,9, 5,0,0, 0,8,0 },
	{ 0,0,1, 2,0,4, 3,0,0 },
	{ 0,6,0, 0,0,9, 2,0,0 },
	{ 2,0,0, 0,0,0, 0,9,3 },
	{ 1,0,0, 0,4,0, 0,0,0 },
	{ 0,0,6, 0,2,0, 7,0,0 }
	}
	{ // Puzzle 11 from http://www.sudoku.org.uk/bifurcation.htm
	{ 0,0,0, 0,3,0, 7,9,0 },
	{ 3,0,0, 0,0,0, 0,0,5 },
	{ 0,0,0, 4,0,7, 3,0,6 },
	{ 0,5,3, 0,9,4, 0,7,0 },
	{ 0,0,0, 0,7,0, 0,0,0 },
	{ 0,1,0, 8,2,0, 6,4,0 },
	{ 7,0,1, 9,0,8, 0,0,0 },
	{ 8,0,0, 0,0,0, 0,0,1 },
	{ 0,9,4, 0,1,0, 0,0,0 }
	}
	{ // From http://www.sudoku.org.uk/discus/messages/29/51.html?1131034031
	{ 2,5,8, 1,0,4, 0,3,7 },
	{ 9,3,6, 8,2,7, 5,1,4 },
	{ 4,7,1, 5,3,0, 2,8,0 },

	{ 7,1,5, 2,0,3, 0,4,0 },
	{ 8,4,9, 6,7,5, 3,2,1 },
	{ 3,6,2, 4,1,0, 0,7,5 },

	{ 1,2,4, 9,0,0, 7,5,3 },
	{ 5,9,3, 7,4,2, 1,6,8 },
	{ 6,8,7, 3,5,1, 4,9,2 }
	}*/;

		Sudoku(const Options& opt) : Script(opt), numbers(*this, 81, 1, 9) {
		for (int i = 0; i <= 8; i++) {
			distinct(*this, numbers.slice(i * 9, 1, 9), opt.ipl()); // Rows
			distinct(*this, numbers.slice(i, 9, 9), opt.ipl()); // Columns
			
			// The digits defined by the puzzle given
			for (int j = 0; j < 9; ++j) {
				if (example[i][j] != 0) {
					rel(*this, numbers[i * 9 + j] == example[i][j]);
				}
			}
		}

		// Blocks, 3 blocks per row
		for (int row = 0; row < 9; row += 3) {
			distinct(*this, numbers.slice(row, 1, 3) + numbers.slice(row + 9, 1, 3) + numbers.slice(row + 18, 1, 3), opt.ipl());
			distinct(*this, numbers.slice(row + 27, 1, 3) + numbers.slice(row + 27 + 9, 1, 3) + numbers.slice(row + 27 + 18, 1, 3), opt.ipl());
			distinct(*this, numbers.slice(row + 54, 1, 3) + numbers.slice(row + 54 + 9, 1, 3) + numbers.slice(row + 54 + 18, 1, 3), opt.ipl());
		}

		
		switch (opt.branching()) {
			case BRANCH_FIRSTFAIL:
				branch(*this, numbers, INT_VAR_SIZE_MIN(), INT_VAL_MIN()); 
				break;
			case BRANCH_MIDDLEVALUE:
				branch(*this, numbers, INT_VAR_SIZE_MIN(), INT_VAL_SPLIT_MAX()); 
				break;
		};
	}

	Sudoku(Sudoku& s) : Script(s) {
		numbers.update(*this, s.numbers);
	}

	virtual Space* copy(void) {
		return new Sudoku(*this);
	}	

	virtual void print(std::ostream& os) const {
		os << "Sudoku:" << std::endl;
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 9; j++) {
				os << numbers[i * 9 + j] << " ";
			}
			os << std::endl;
		}
	}
};

int main(int argc, char* argv[]) {
	Options opt("Sudoku");
	opt.branching(Sudoku::BRANCH_FIRSTFAIL);
	opt.branching(Sudoku::BRANCH_FIRSTFAIL, "firstfail", "First fail heuristic");
	opt.branching(Sudoku::BRANCH_MIDDLEVALUE, "middle", "Select middle value");
	opt.parse(argc, argv);
	Script::run<Sudoku, DFS, Options>(opt);
	return 0;
}

/* COMMENTS ABOUT HEURISTICS */

// We tried the opposite of first fail, with INT_VAR_SIZE_MAX(). It was terrible. 
// First fail, good heuristic, as we only want the one solution, makes the tree smaller
// First fail, but with selecting a middle value, slightly worse than firstfail, at least in the puzzles we tried

/* COMMENTS ABOUT PROPAGATION STRENGTHS (using first fail heuristic) */

// IPL_DEF: depth: 16, 311 fails, 311 internal nodes
// IPL_VAL: same result as IPL_DEF
// IPL_BND: depth: 7, 12 fails, 12 internal nodes
// IPL_DOM: depth: 0, 0 fails, 0 internal nodes