/*
 *  Main authors:
 *     Christian Schulte <cschulte@kth.se>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/int.hh>
#include <math.h>

using namespace Gecode;

using namespace Gecode::Int;

/*
 * Custom brancher for forcing mandatory parts
 *
 */
class IntervalBrancher : public Brancher {
protected:
	// Views for x-coordinates (or y-coordinates)
	ViewArray<IntView> x;
	// Width (or height) of rectangles
	int* w;
	// Percentage for obligatory part
	double p;
	// Cache of first unassigned view
	mutable int start;
	// Description
	class Description : public Choice {
	public:
		// Position of view
		int pos, val;
		// You might need more information, please add here

		/* Initialize description for brancher b, number of
		 *  alternatives a, position p, and ???.
		 */
		Description(const Brancher& b, unsigned int a, int p, int v)
			: Choice(b, a), pos(p), val(v) {}
		// Report size occupied
		virtual size_t size(void) const {
			return sizeof(Description);
		}
		// Archive the choice's information in e
		virtual void archive(Archive& e) const {
			Choice::archive(e);
			// You must also archive the additional information
			e << pos << val;
		}
	};
public:
	// Construct branching
	IntervalBrancher(Home home,
		ViewArray<IntView>& x0, int w0[], double p0)
		: Brancher(home), x(x0), w(w0), p(p0), start(0) {}
	// Post branching
	static void post(Home home, ViewArray<IntView>& x, int w[], double p) {
		(void) new (home) IntervalBrancher(home, x, w, p);
	}

	// Copy constructor used during cloning of b
	IntervalBrancher(Space& home, IntervalBrancher& b)
		: Brancher(home, b), p(b.p), start(b.start) {
		x.update(home, b.x);
		w = home.alloc<int>(x.size());
		for (int i = x.size(); i--; )
			w[i] = b.w[i];
	}
	// Copy brancher
	virtual Actor* copy(Space& home) {
		return new (home) IntervalBrancher(home, *this);
	}

	// Check status of brancher, return true if alternatives left
	virtual bool status(const Space& home) const {

		int subIntervalSize = 0;
		for (int i = start; i < x.size(); i++) {
			if (!x[i].assigned()) {
				subIntervalSize = w[i] - floor(p*w[i]);
				if (subIntervalSize <= x[i].size()) {
					return true;
				}
			}
		}

		return false;

	}
	// Return choice as description
	virtual const Choice* choice(Space& home) {

		int subIntervalSize = 0;
		for (int i = start; i < x.size(); i++) {
			if (!x[i].assigned()) {
				subIntervalSize = w[i] - floor(p*w[i]);
				if (subIntervalSize <= x[i].size()) {
					return new Description(*this, 2, i, subIntervalSize);
				}
			}
		}
		GECODE_NEVER; // Since we check for this in the status function, this code should never be executed
		return NULL;
	}
	// Construct choice from archive e
	virtual const Choice* choice(const Space&, Archive& e) {
		

		int pos, val;
		e >> pos >> val;
		return new Description(*this, 2, pos, val);
	}
	// Perform commit for choice c and alternative a
	virtual ExecStatus commit(Space& home,
		const Choice& c,
		unsigned int a) {
		const Description& d = static_cast<const Description&>(c);
		int pos = d.pos; int subIntervalSize = d.val;
		if (a == 0)
			return me_failed(x[pos].lq(home, x[pos].min() + subIntervalSize)) ? ES_FAILED : ES_OK;
		else
			return me_failed(x[pos].gr(home, x[pos].min() + subIntervalSize)) ? ES_FAILED : ES_OK;
	}
	// Print some information on stream o (used by Gist, from Gecode 4.0.1 on)
	virtual void print(const Space& home, const Choice& c, unsigned int a,
		std::ostream& o) const {

		// FILL IN HERE
		const Description& d = static_cast<const Description&>(c);
		int pos = d.pos; int subIntervalSize = d.val;
		if (a == 0)
			o << "x[" << pos << "] <= " << x[pos].min() + subIntervalSize;
		else
			o << "x[" << pos << "] > " << x[pos].min() + subIntervalSize;


	}
};

// This posts the interval branching
void interval(Home home, const IntVarArgs& x, const IntArgs& w, double p) {
	// Check whether arguments make sense
	if (x.size() != w.size())
		throw ArgumentSizeMismatch("interval");
	// Never post a branching in a failed space
	if (home.failed()) return;
	// Create an array of integer views
	ViewArray<IntView> vx(home, x);
	// Create an array of integers
	int* wc = static_cast<Space&>(home).alloc<int>(x.size());
	for (int i = x.size(); i--; )
		wc[i] = w[i];
	// Post the brancher
	IntervalBrancher::post(home, vx, wc, p);
}


/*
*
*	Co Authors
*		Joey Öhman, <joeyoh@kth.se>
*		Nicolas Jeitziner, <njei@kth.se>
*
*	We split the interval into two subintervals instead of several. 
*
*/

