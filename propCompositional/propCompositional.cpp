#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

int s1Arr[3], s2Arr[3];
int s1x, s2x;

class S1 : public Script {
public:
	IntVar a, b, c, x;
	//IntVarArray left;
	IntVarArgs arr(4);
	//IntVar x;


	S1(const Options& opt) : Script(opt), a(*this, 1, 5), b(*this, 1, 5), c(*this, 1, 5), x(*this, 4, 20) {
		arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = x;
		rel(*this, a + b + c + b == x); // a+b+c+b=x, a+b+b-x=-c
		branch(*this, arr, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		// branch(*this, a, b, c, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}

	S1(S1& s) : Script(s) {
		left.update(*this, s.left);
		//a.update(*this, s.a);
		//b.update(*this, s.b);
		//c.update(*this, s.c);
		x.update(*this, s.x);
	}

	virtual Space* copy(void) {
		return new S1(*this);
	}

	virtual void print(std::ostream& os) const {
		os << "Compositional Propagation Test Solution:" << std::endl;
		// os << "a: " << a << ", b: " << b << ", c: " << c << ", x: " << x << std::endl;
		os << "a: " << left[0] << ", b: " << left[1] << ", c: " << left[2] << ", x: " << x << std::endl;
		for (int i = 0; i < left.size(); i++)
			s1Arr[i] = left[i].val();
		s1x = x.val();
	}
};

class S2 : public Script {
public:
	//IntVar a, b, c, x;
	IntVarArray left;
	IntVar x, u;

	S2(const Options& opt) : Script(opt), left(*this, 3, 1, 5), x(*this, 4, 20), u(*this, 2, 10) {
		rel(*this, left[0] + left[1] == u); // a+b=u
		rel(*this, u + left[1] + left[2] == x); // u+b+c=x
		branch(*this, left, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}

	S2(S2& s) : Script(s) {
		left.update(*this, s.left);
		//a.update(*this, s.a);
		//b.update(*this, s.b);
		//c.update(*this, s.c);
		x.update(*this, s.x);
		u.update(*this, s.u);
	}

	virtual Space* copy(void) {
		return new S2(*this);
	}

	virtual void print(std::ostream& os) const {
		os << "Compositional Propagation Test Solution:" << std::endl;
		// os << "a: " << a << ", b: " << b << ", c: " << c << ", x: " << x << std::endl;
		os << "a: " << left[0] << ", b: " << left[1] << ", c: " << left[2] << ", x: " << x << std::endl;
		for (int i = 0; i < left.size(); i++)
			s2Arr[i] = left[i].val();
		s2x = x.val();
	}
};

int main(int argc, char* argv[]) {
	Options opt("Compositional Propagation Test");
	opt.parse(argc, argv);
	Script::run<S1, DFS, Options>(opt);
	Script::run<S2, DFS, Options>(opt);
	bool equalArr = true;
	for (int i = 0; i < 3; i++)
		if (s1Arr[i] != s2Arr[i])
			equalArr = false;
	if (s1x == s2x) {
		std::cout << "Equal x!" << std::endl;
	}
	if(equalArr)
		std::cout << "Equal a, b, c!" << std::endl;

	return 0;
}