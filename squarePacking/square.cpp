/* Main authors
*		Joey Öhman, joeyoh@kth.se
*		Nicolas Jeitziner, njei@kth.se
*/

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;
static const int forbiddenGaps[] = { 2,3,2,3,3,3,3,4,4,4,5,5,5,5,5,5,6,6,6,6,7,7,7,7,7,7,7,7,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,10 };

class Square : public Script {
public:
	IntVar s;
	IntVarArray x, y; 
	static const int n = 10;
	static const int sMax = (n * (n + 1)) / 2; // Sum of all widths, if every square were to be placed in a row
	
	Square(const SizeOptions& opt) :
		Script(opt), 
		s(*this, 2 * n - 1, sMax), // Lower bound: the 2 greatest squares needs to be next to each other
		x(*this, n-1, 0, sMax - 1), // Don't place the 1x1 square
		y(*this, n-1, 0, sMax - 1) {

		// Total area constraint
		rel(*this, s*s >= n*(n+1)*((2*n)+1)/6);

		// Symmetry removal
		rel(*this, x[0] <= (s-n)/2); 
		rel(*this, y[0] <= x[0]);

		// Initial domain reduction, forbidden gaps, only applied to 2 sides of the enclosing square
		for (int i = n - 2; i >= 0; i--) {
			for (int j = 1; j <= forbiddenGaps[n - i - 2]; j++) {
				rel(*this, x[i] != j);
				rel(*this, y[i] != j);
			}
		}

		// Set the upper bound for each square
		for (int i = 0; i < n-1; i++) {
			rel(*this, x[i] <= s - sizeOfSquare(i));
			rel(*this, y[i] <= s - sizeOfSquare(i));
		}
		
		// Iterates over each pair once
		for (int i = 0; i < n-2; i++) {
			for (int j = i+1; j < n-1; j++) { 
				rel(*this,  // Reified constraints, checking collision
					x[i] + sizeOfSquare(i) <= x[j] || // square i left of square j
					x[j] + sizeOfSquare(j) <= x[i] || // j left of i
					y[i] + sizeOfSquare(i) <= y[j] || // j above i
					y[j] + sizeOfSquare(j) <= y[i] // i above j
				); 
			}
		}
		
		for (int i = 0; i < s.max(); i++) { // Looping through each column
			BoolVarArgs belongsToCol(*this, n - 1, 0, 1); // Booleans for reification

			for (int j = 0; j < n - 1; j++) {
				dom(*this, x[j], i - sizeOfSquare(j) + 1, i, belongsToCol[j]); // Check for "collision" between column and square
			}
			linear(*this, belongsToCol, IRT_LQ, s); // Total square width over row can not exceed enclosing squares width
		}

		for (int i = 0; i < s.max(); i++) { // Same as for the columns
			BoolVarArgs belongsToRow(*this, n - 1, 0, 1);

			for (int j = 0; j < n - 1; j++) {
				dom(*this, y[j], i - sizeOfSquare(j) + 1, i, belongsToRow[j]);
			}
			linear(*this, belongsToRow, IRT_LQ, s);
		}
		
		// choosing s as min => optimal solution
		branch(*this, s, INT_VAL_MIN()); 
		branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN()); // INT_VAR_NONE => go in order => greatest square first
		branch(*this, y, INT_VAR_NONE(), INT_VAL_MIN()); // INT_VAL_MIN => Try leftmost/topmost (lowest coordinates) first

	}

	Square(Square& sq) : Script(sq) {
		s.update(*this, sq.s);
		x.update(*this, sq.x);
		y.update(*this, sq.y);
	}

	virtual Space* copy(void) {
		return new Square(*this);
	}

	// Prints for every point: the square size of the square placed at that point, 0 if none
	virtual void print(std::ostream& os) const {
		os << "Square Packing:" << std::endl;
		os << "s: " << s << std::endl;
		os << "n: " << n << std::endl;
		os << "x: " << x << std::endl;
		os << "y: " << y << std::endl;
		int size = s.val();
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				os << getSquare(i, j) << "  ";
			}
			os << std::endl;
		}
	}
	
	// Helper for print
	int getSquare(int xCoord, int yCoord) const {
		for (int i = 0; i < n-1; i++) {
			if (x[i].assigned() && y[i].assigned()) {
				if (xCoord >= x[i].val() && xCoord < x[i].val() + sizeOfSquare(i))
					if (yCoord >= y[i].val() && yCoord < y[i].val() + sizeOfSquare(i))
						return n - i; // 1-indexed print, more intuitive
			}
		}
		return 0;
	}
	
	static int sizeOfSquare(int i) {
		return n - i;
	}

};

int main(int argc, char* argv[]) {
	SizeOptions opt("Square");
	//opt.size(3);
	opt.parse(argc, argv);
	Script::run<Square, DFS, SizeOptions>(opt);
	return 0;
}

/* Comments:

Branching:
	Branching on s first is a good branching heuristic as it prunes the search tree early on. 
	Assigning s to low values first, will remove values for 2(n-1) variables at a cheap propagation cost.
	This is close to first fail.

	Branching on x first then y (or y then x, which leads to the same tree because of symmetry) is a much worse
	branching heuristic as it tries a great number of combinations of x and y that leads 
	to solutions with big values for s first.

	We are branching on s first, since it is similiar to first fail and it makes the first solution we find
	the optimal solution. We then branch on x first, we branch on the greatest square first and try
	to place it furthest to the left first. Then we branch on y, and branch on the greatest square first, trying
	to place it at the top (lowest coordinate).


	PRINTOUT:

	Square Packing:
	s: 13
	n: 7
	x: {0, 0, 6, 7, 7, 10}
	y: {0, 7, 7, 0, 4, 4}
	7  7  7  7  7  7  7  6  6  6  6  6  6
	7  7  7  7  7  7  7  6  6  6  6  6  6
	7  7  7  7  7  7  7  6  6  6  6  6  6
	7  7  7  7  7  7  7  6  6  6  6  6  6
	7  7  7  7  7  7  7  6  6  6  6  6  6
	7  7  7  7  7  7  7  6  6  6  6  6  6
	7  7  7  7  7  7  7  5  5  5  5  5  0
	4  4  4  4  3  3  3  5  5  5  5  5  0
	4  4  4  4  3  3  3  5  5  5  5  5  0
	4  4  4  4  3  3  3  5  5  5  5  5  0
	4  4  4  4  2  2  0  5  5  5  5  5  0
	0  0  0  0  2  2  0  0  0  0  0  0  0
	0  0  0  0  0  0  0  0  0  0  0  0  0

	Initial
			propagators: 428
			branchers:   3

	Summary
			runtime:      0.054 (54.000 ms)
			solutions:    1
			propagations: 3387
			nodes:        54
			failures:     22
			restarts:     0
			no-goods:     0
			peak depth:   19


*/
