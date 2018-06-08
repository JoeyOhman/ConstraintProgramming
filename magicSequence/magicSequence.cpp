#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

int n;

class MagicSequence : public Script {
public:
	IntVarArray seq;
	
	MagicSequence(const SizeOptions& opt) : Script(opt), seq(*this, opt.size(), 0, opt.size()-1)  {
		n = opt.size();

		// Constraints defining a magic sequence
		BoolVarArray b(*this, n, 0, 1); // seq[i] == (occurrences of i) <==> b[i] == 1 (reification)
		for (int i = 0; i < n; i++) {
			BoolVarArgs occs(n); // occs[j] == 1 => seq[j] == i
			for (int j = 0; j < n; j++) {
				BoolVar bv(*this, 0, 1);
				rel(*this, seq[j], IRT_EQ, i, bv);
				occs[j] = bv;
			}
			linear(*this, occs, IRT_EQ, seq[i], b[i]);
		}
		linear(*this, b, IRT_EQ, n);

		// Sum of sequence corresponds to sum of occurrences, which must be equal to n
		rel(*this, sum(seq) == n);

		// Implied constraint on sum, explained in MPG. 
		IntArgs c(n);
		for (int i = 0; i < n; i++)
			c[i] = i - 1;
		linear(*this, c, seq, IRT_EQ, 0);

		// First fail, assignment gives incredible propagation
		branch(*this, seq, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}

	MagicSequence(MagicSequence& s) : Script(s) {
		seq.update(*this, s.seq);
	}

	virtual Space* copy(void) {
		return new MagicSequence(*this);
	}

	virtual void print(std::ostream& os) const {
		os << "Magic Sequence solution for size " << n << " :" << std::endl;
		os << "<";
		for (int i = 0; i < n; i++)
			os << " " << seq[i] << " ";
		os << ">" << std::endl;
	}
};

int main(int argc, char* argv[]) {
	SizeOptions opt("Magic Sequence");
	opt.parse(argc, argv);
	Script::run<MagicSequence, DFS, SizeOptions>(opt);

	return 0;
}