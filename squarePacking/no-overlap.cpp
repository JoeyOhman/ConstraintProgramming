/*
 *  Main author:
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

using namespace Gecode;
using namespace Gecode::Int;

// The no-overlap propagator
class NoOverlap : public Propagator {
protected:
  // The x-coordinates
  ViewArray<IntView> x;
  // The width (array)
  int* w;
  // The y-coordinates
  ViewArray<IntView> y;
  // The heights (array)
  int* h;
public:
  // Create propagator and initialize
  NoOverlap(Home home, 
            ViewArray<IntView>& x0, int w0[], 
            ViewArray<IntView>& y0, int h0[])
    : Propagator(home), x(x0), w(w0), y(y0), h(h0) {
    x.subscribe(home,*this,PC_INT_BND);
    y.subscribe(home,*this,PC_INT_BND);
  }
  // Post no-overlap propagator
  static ExecStatus post(Home home, 
                         ViewArray<IntView>& x, int w[], 
                         ViewArray<IntView>& y, int h[]) {
    // Only if there is something to propagate
    if (x.size() > 1)
      (void) new (home) NoOverlap(home,x,w,y,h);
    return ES_OK;
  }

  // Copy constructor during cloning
  NoOverlap(Space& home, NoOverlap& p)
    : Propagator(home,p) {
    x.update(home,p.x);
    y.update(home,p.y);
    // Also copy width and height arrays
    w = home.alloc<int>(x.size());
    h = home.alloc<int>(y.size());
    for (int i=x.size(); i--; ) {
      w[i]=p.w[i]; h[i]=p.h[i];
    }
  }
  // Create copy during cloning
  virtual Propagator* copy(Space& home) {
    return new (home) NoOverlap(home,*this);
  }

  // Re-schedule function after propagator has been re-enabled
  virtual void reschedule(Space& home) {
     x.reschedule(home,*this,PC_INT_BND);
     y.reschedule(home,*this,PC_INT_BND);
  }

  // Return cost (defined as cheap quadratic)
  virtual PropCost cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::LO,2*x.size());
  }

  // Perform propagation
  /* Authors of this function
  *		Joey Öhman, joeyoh@kth.se
  *		Nicolas Jeitziner, njei@kth.se
  */
  virtual ExecStatus propagate(Space& home, const ModEventDelta&) {

	  for (int i = 0; i < x.size(); i++) {
		  for (int j = 0; j < x.size(); j++) {
			  if (j != i) {
				  if (x[j].assigned() && y[j].assigned()) { 

					  if (x[i].assigned() && x[i].val() >= x[j].val() && x[i].val() < x[j].val() + w[j]) {
						  //x are colliding, prune y
						  for (int k = y[j].val(); k < y[j].val() + h[j]; k++) {
							  GECODE_ME_CHECK(y[i].nq(home, k));
						  }
					  }
					  if (y[i].assigned() && y[i].val() >= y[j].val() && y[i].val() < y[j].val() + h[j]) {
						  //y are colliding, prune x
						  for (int k = x[j].val(); k < x[j].val() + w[j]; k++) {
							  GECODE_ME_CHECK(x[i].nq(home, k));
						  }
					  }
				  }
			  }
		  }
	  }

	  // Since our propagator checks that they are not colliding, 
	  // we only need to check that they are all assigned to be sure that the propagator is subsumed
	  if (x.assigned() && y.assigned())
		  return home.ES_SUBSUMED(*this);
	  
	  else
		  return ES_FIX;

	  /* COMMENTS:
		This propagator was subscribed to being called everytime the bounds of any of the variables domain is changed.
		We think that there will be useless calls to the propagator, since we propagate only when values are assigned.
		We would recommend to change the subscription to PC_INT_VAL.
	  */

  }

  // Dispose propagator and return its size
  virtual size_t dispose(Space& home) {
    x.cancel(home,*this,PC_INT_BND);
    y.cancel(home,*this,PC_INT_BND);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }
};

/*
 * Post the constraint that the rectangles defined by the coordinates
 * x and y and width w and height h do not overlap.
 *
 * This is the function that you will call from your model. The best
 * is to paste the entire file into your model.
 */
void nooverlap(Home home, 
               const IntVarArgs& x, const IntArgs& w,
               const IntVarArgs& y, const IntArgs& h) {
  // Check whether the arguments make sense
  if ((x.size() != y.size()) || (x.size() != w.size()) ||
      (y.size() != h.size()))
    throw ArgumentSizeMismatch("nooverlap");
  // Never post a propagator in a failed space
  if (home.failed()) return;
  // Set up array of views for the coordinates
  ViewArray<IntView> vx(home,x);
  ViewArray<IntView> vy(home,y);
  // Set up arrays (allocated in home) for width and height and initialize
  int* wc = static_cast<Space&>(home).alloc<int>(x.size());
  int* hc = static_cast<Space&>(home).alloc<int>(y.size());
  for (int i=x.size(); i--; ) {
    wc[i]=w[i]; hc[i]=h[i];
  }
  // If posting failed, fail space
  if (NoOverlap::post(home,vx,wc,vy,hc) != ES_OK)
    home.fail();
}

