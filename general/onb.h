/*
code developed by
    Rong-Qiang He (rqhe@ruc.edu.cn, RUC, China)
date 2013 - 2017
*/

#ifndef _ONB_H_
#define _ONB_H_

// occupancy number basis

#include "stdpfx.h"
#include "vec.h"
#include "mat.h"
#include "number.h"

// occupancy number basis state (for example |01001101>)
// a binary number, 1 for occupied, 0 for unoccupied, spinless
// 0-based numbering is used
// the state of the i-th site is denoted by the i-th digit (from low to high)
// Int pos denote the position of an site
// for ns <= 32, use UInt for bs
// for ns <= 64, use ULLInt for bs
// for ns > 64, change the code
// (const Int &lw, const Int &up) means [lw, up)

class Onb {
    friend inline std::ostream &operator<<(std::ostream &os, const Onb &a);
private:
	static const BnmlFast bf;
	static const Int max_ns;		// max of valid ns
	typedef UInt Tbs;				// type for bs
	Tbs bs;				// use the first ns digits of an UInt to represet the state
	Int ne;				// number of electrons
	Int ns;				// number of sites
private:
	Int first_pos_filled(const Int &lw, const Int &up) const { return lw + binary_trailing_zeros((bs >> lw) & ((1 << (up - lw)) - 1)); }
	Int first_pos_unfilled(const Int &lw, const Int &up) const { return lw + binary_trailing_ones((bs >> lw) & ((1 << (up - lw)) - 1)); }
	void fill(const Int &pos) { bs |= (1 << pos); }
	void fill(const Int &lw, const Int &up) { bs |= (((1 << (up - lw)) - 1) << lw); }
	void fill(const Int &lw, const Int &up, const Int &except) { bs |= (((1 << (up - lw)) - 1) << lw) ^ (1 << except); }
	Int count_electrons(const Int &lw, const Int &up) const { return lw >= up ? 0 : count_binary_ones((bs >> lw) & ((1 << (up - lw)) - 1)); }
public:
	inline Onb(): bs(0), ne(0), ns(0) {}
	inline Onb(Int ne_i, Int ns_i, Idx id);
	inline Onb(const Onb &lhs, const Onb &rhs);
	bool isuno(const Int &pos) const { return !isocc(pos); }
	bool isocc(const Int &pos) const { return (bs >> pos) & 1; }
	Int operator[](const Int &i) const { return (bs >> i) & 1; }	// n_i = c_i^+ c_i
	Onb &crt(const Int &pos) { ++ne; bs ^= 1 << pos; return *this; }
	Onb &ann(const Int &pos) { --ne; bs ^= 1 << pos; return *this; }
	Onb &hop(const Int &pi, const Int &pj) { bs ^= 1 << pi; bs ^= 1 << pj; return *this; }		// c_i^+ c_j or c_j^+ c_i
	Int sgn(Int pi, Int pj) const { if (pi > pj) SWAP(pi, pj); return 1 - 2 * (1 & count_electrons(pi + 1, pj)); }		// sign for hop
	Int sgn(const Int &pos) const { return 1 - 2 * (1 & count_electrons(0, pos)); }		// sign for crt and ann
	Int getne() const { return ne; }
	Int getns() const { return ns; }
	Int sgn_of_shift(Int ns_l) const { Int ne_l = count_electrons(0, ns_l); return 1 - 2 * (ne_l & 1 & (ne - ne_l)); }
	Onb shift(Int ns_l) { Onb z(*this); z.bs = (bs >> ns_l) | ((bs & ((1 << ns_l) - 1)) << (ns - ns_l)); return z; }
	inline Idx idx() const;
	inline MatReal slaterdet() const;
	bool operator<(const Onb &b) {
		if (ns == b.ns && ne == b.ne) return bs < b.bs;
		return ns == b.ns ? ne < b.ne : ns < b.ns;
	}
};

typedef Vec<Onb> VecOnb;
typedef Mat<Onb> MatOnb;


std::ostream &operator<<(std::ostream &os, const Onb &a)
{
	for_Int (i, 0, a.ns) os << (((a.bs >> i) & 1) ? '1' : '0');
	return os;
}

Onb::Onb(const Onb &lhs, const Onb &rhs): ne(lhs.ne + rhs.ne), ns(lhs.ns + rhs.ns), bs(lhs.bs | (rhs.bs << lhs.ns))
{
#ifdef _ASSERTION_
	if (ns > max_ns) ERR(NAV4(ns, max_ns, lhs, rhs));
#endif
}

// index to state, [lw, up) digits of bs, bf is a fast binomial
// C(n, m) = C(n - 1, m - 1) + C(n - 1, m)
// this function may be accelerated for a long state
Onb::Onb(Int ne_i, Int ns_i, Idx id): ne(ne_i), ns(ns_i)
{
	Int en = this->ne;
	Int lw = 0;
	const Int &up = this->ns;
#ifdef _ASSERTION_
	if (en > up - lw || en < 0 || lw < 0 || id >= bf(up - lw, en))
		ERR(NAV5(lw, up, en, id, bf(up - lw, en)));
#endif
	bs = 0;
	while (1 < en && en < up - lw - 1) {
		Int i = bf(up - lw - 1, en - 1);
		if (id < i) {
			fill(lw);
			--en;
		} else {
			id -= i;
		}
		++lw;
	}

	if (!en) ;
	else if (en == 1) fill(lw + Int(id));
	else if (en == up - lw - 1) fill(lw, up, up - Int(id) - 1);
	else if (en == up - lw) fill(lw, up);
#ifdef _ASSERTION_
	else ERR(NAV5(lw, up, en, id, bf(up - lw, en)));
#endif
}

// state to index, [lw, up) digits of bs, bf is a fast binomial
// C(n, m) = C(n - 1, m - 1) + C(n - 1, m)
// this function may be accelerated for a long state
Idx Onb::idx() const
{
	Int en = this->ne;
	Int lw = 0;
	const Int &up = this->ns;
#ifdef _ASSERTION_
	if (en > up - lw || en < 0 || lw < 0 || en != count_electrons(lw, up))
		ERR(NAV4(lw, up, en, count_electrons(lw, up)));
#endif

	Idx id = 0;
	while (1 < en && en < up - lw - 1) {
		Int i = bf(up - lw - 1, en - 1);
		if (isocc(lw)) {
			--en;
		} else {
			id += i;
		}
		++lw;
	}

	if (!en) ;
	else if (en == 1) id += first_pos_filled(lw, up) - lw;
	else if (en == up - lw - 1) id += up - 1 - first_pos_unfilled(lw, up);
	else if (en == up - lw) ;
#ifdef _ASSERTION_
	else ERR(NAV4(lw, up, en, count_electrons(lw, up)));
#endif

	return id;
}

MatReal Onb::slaterdet() const
{
	MatReal sr(ns, ne, 0.);
	Int i = 0;
	for_Int (c, 0, ne) {
		while (!(*this)[i]) ++i;
		sr[i++][c] = 1.;
	}
	return sr;
}

#endif /* _ONB_H_ */
