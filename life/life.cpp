/* Authors:
*	Nicolas Jeitziner <njei@kth.se>
*	Joey Öhman <joeyoh@kth.se>
*/


#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

class Life : public IntMaximizeScript {
public:
	IntVarArray cells;
	IntVarArray subgridDensities;
	IntVar singleDensity;
	IntVar aliveCells;

	static const int N = 9;
	static const int NB = N + 4;
	static const int totalAmountOfCells = N * N;
	static const int amountOfSubGrids = (N / 3) * (N / 3);
	static const int cellsInSubGrid = 9;

	Life(const Options& opt) : IntMaximizeScript(opt),
		cells(*this, NB*NB, 0, 1),
		subgridDensities(*this, amountOfSubGrids, 0, 6),
		singleDensity(*this, 0, totalAmountOfCells - (amountOfSubGrids * cellsInSubGrid)),
		aliveCells(*this, 0, totalAmountOfCells) {

		Matrix<IntVarArray> mat(cells, NB);

		// Set the border to 0
		rel(*this, sum(mat.col(0)) == 0);
		rel(*this, sum(mat.col(1)) == 0);
		rel(*this, sum(mat.col(NB - 1)) == 0);
		rel(*this, sum(mat.col(NB - 2)) == 0);

		rel(*this, sum(mat.row(0)) == 0);
		rel(*this, sum(mat.row(1)) == 0);
		rel(*this, sum(mat.row(NB - 1)) == 0);
		rel(*this, sum(mat.row(NB - 2)) == 0);


		// Neighbour constraints, inner boarder and within
		for (int i = 1; i < NB - 1; i++) {
			for (int j = 1; j < NB - 1; j++) {
				BoolVar b(*this, 0, 1);
				rel(*this, mat(i, j), IRT_EQ, 1, b);

				rel(*this,
					(b &&
					(sum(mat.slice(i - 1, i + 2, j - 1, j + 2)) == 3 || sum(mat.slice(i - 1, i + 2, j - 1, j + 2)) == 4))
					||
					(!b && sum(mat.slice(i - 1, i + 2, j - 1, j + 2)) != 3)
				);
			}
		}

		// Max sum of each 3x3 grid is 6
		for (int i = 3; i < NB - 3; i++) {
			for (int j = 3; j < NB - 3; j++) {
				rel(*this, sum(mat.slice(i - 1, i + 2, j - 1, j + 2)) <= 6);
			}
		}
		std::cout << "Summing subgrids" << std::endl;

		// Set the densities of the subgrids
		int gridCounter = 0;
		for (int i = 3; i < NB - 3; i += 3) {
			for (int j = 3; j < NB - 3; j += 3) {

				rel(*this, subgridDensities[gridCounter] == sum(mat.slice(i - 1, i + 2, j - 1, j + 2)));
				gridCounter++;
			}
		}

		// Set the density of the single cells which did not fit in the subgrids
		int start = 2 + (N / 3) * 3; // First single cell in a row/column
		rel(*this, singleDensity == sum(mat.slice(start, NB - 2, 2, NB - 2)) + sum(mat.slice(2, start, start, NB - 2)));
		rel(*this, aliveCells == sum(subgridDensities) + singleDensity);


		// First fail
		branch(*this, cells, INT_VAR_SIZE_MIN(), INT_VAL_MAX());
		branch(*this, subgridDensities, INT_VAR_SIZE_MIN(), INT_VAL_MAX());
		branch(*this, singleDensity, INT_VAL_MAX());
		branch(*this, aliveCells, INT_VAL_MAX());

	}

	Life(Life& s) : IntMaximizeScript(s) {
		cells.update(*this, s.cells);
		aliveCells.update(*this, s.aliveCells);
	}

	virtual Space* copy(void) {
		return new Life(*this);
	}

	virtual void print(std::ostream& os) const {
		Matrix<IntVarArray> mat(cells, NB);
		os << std::endl << "Size: " << N << std::endl;
		os << "Alive cells: " << aliveCells.val() << std::endl;

		for (int i = 2; i < NB - 2; i++) {
			for (int j = 2; j < NB - 2; j++) {
				os << mat(i, j) << "  ";
			}
			os << std::endl;
		}
	}

	virtual IntVar cost(void) const override {
		return aliveCells;
	}

};

int main(int argc, char* argv[]) {
	try {
		Options opt("Life");
		opt.parse(argc, argv);
		Script::run<Life, BAB, Options>(opt);
	}
	catch (Exception e) {
		std::cerr << "Gecode exception: " << e.what() << std::endl;
	}
	return 0;
}

/*  Printout:

	Size: 8
	Alive cells: 36
	1  1  0  1  1  0  1  1
	1  1  0  1  1  0  1  1
	0  0  0  0  0  0  0  0
	1  1  0  1  1  0  1  1
	1  1  0  1  1  0  1  1
	0  0  0  0  0  0  0  0
	1  1  0  1  1  0  1  1
	1  1  0  1  1  0  1  1

	Initial
	propagators: 850
	branchers:   4

	Summary
	runtime:      40.850 (40850.000 ms)
	solutions:    4
	propagations: 172404764
	nodes:        1590343
	failures:     795168
	restarts:     0
	no-goods:     0
	peak depth:   34


	Size: 9
	Alive cells : 43
	1  1  0  0  1  1  0  1  1
	1  0  0  1  0  1  0  1  1
	0  1  0  1  0  1  0  0  0
	1  1  0  1  0  1  1  1  1
	1  0  0  1  0  0  0  0  1
	0  1  0  1  1  1  1  0  0
	1  1  0  0  0  0  1  1  0
	1  0  0  1  1  0  0  0  1
	0  1  1  0  1  1  0  1  1

	Initial
	propagators : 1035
	branchers : 4

	Summary
	runtime : 3 : 55.029 (235029.000 ms)
	solutions : 5
	propagations : 956356155
	nodes : 8866841
	failures : 4433415
	restarts : 0
	no - goods : 0
	peak depth : 49

	*/