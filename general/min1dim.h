/*
code developed by
    Rong-Qiang He (rqhe@ruc.edu.cn, RUC, China)
date 2016-05-13

    copy from mins.h in Numerical Recipes in C++
    one-dimensional minimization
*/

#ifndef _MIN1DIM_H_
#define _MIN1DIM_H_

#include "toolbox.h"

struct Bracketmethod {
	Real ax,bx,cx,fa,fb,fc;
	template <class T>
	void bracket(const Real a, const Real b, T &func)
	{
		const Real GOLD=1.618034,GLIMIT=100.0,TINY=1.0e-20;
		ax=a; bx=b;
		Real fu;
		fa=func(ax);
		fb=func(bx);
		if (fb > fa) {
			SWAP(ax,bx);
			SWAP(fb,fa);
		}
		cx=bx+GOLD*(bx-ax);
		fc=func(cx);
		while (fb > fc) {
			Real r=(bx-ax)*(fb-fc);
			Real q=(bx-cx)*(fb-fa);
			Real u=bx-((bx-cx)*q-(bx-ax)*r)/(2.0*SIGN(MAX(ABS(q-r),TINY),q-r));
			Real ulim=bx+GLIMIT*(cx-bx);
			if ((bx-u)*(u-cx) > 0.0) {
				fu=func(u);
				if (fu < fc) {
					ax=bx;
					bx=u;
					fa=fb;
					fb=fu;
					return;
				} else if (fu > fb) {
					cx=u;
					fc=fu;
					return;
				}
				u=cx+GOLD*(cx-bx);
				fu=func(u);
			} else if ((cx-u)*(u-ulim) > 0.0) {
				fu=func(u);
				if (fu < fc) {
					shft3(bx,cx,u,u+GOLD*(u-cx));
					shft3(fb,fc,fu,func(u));
				}
			} else if ((u-ulim)*(ulim-cx) >= 0.0) {
				u=ulim;
				fu=func(u);
			} else {
				u=cx+GOLD*(cx-bx);
				fu=func(u);
			}
			shft3(ax,bx,cx,u);
			shft3(fa,fb,fc,fu);
		}
	}
	void shft2(Real &a, Real &b, const Real c)
	{
		a=b;
		b=c;
	}
	void shft3(Real &a, Real &b, Real &c, const Real d)
	{
		a=b;
		b=c;
		c=d;
	}
	void mov3(Real &a, Real &b, Real &c, const Real d, const Real e, const Real f)
	{
		a=d; b=e; c=f;
	}
};
struct Golden : Bracketmethod {
	Real xmin,fmin;
	const Real tol;
	Golden(const Real toll=3.0e-8) : tol(toll) {}
	template <class T>
	Real minimize(T &func)
	{
		const Real R=0.61803399,C=1.0-R;
		Real x1,x2;
		Real x0=ax;
		Real x3=cx;
		if (ABS(cx-bx) > ABS(bx-ax)) {
			x1=bx;
			x2=bx+C*(cx-bx);
		} else {
			x2=bx;
			x1=bx-C*(bx-ax);
		}
		Real f1=func(x1);
		Real f2=func(x2);
		while (ABS(x3-x0) > tol*(ABS(x1)+ABS(x2))) {
			if (f2 < f1) {
				shft3(x0,x1,x2,R*x2+C*x3);
				shft2(f1,f2,func(x2));
			} else {
				shft3(x3,x2,x1,R*x1+C*x0);
				shft2(f2,f1,func(x1));
			}
		}
		if (f1 < f2) {
			xmin=x1;
			fmin=f1;
		} else {
			xmin=x2;
			fmin=f2;
		}
		return xmin;
	}
};
struct Brent : Bracketmethod {
	Real xmin,fmin;
	const Real tol;
	Brent(const Real toll=3.0e-8) : tol(toll) {}
	template <class T>
	Real minimize(T &func)
	{
		const Int ITMAX=100;
		const Real CGOLD=0.3819660;
		const Real ZEPS=std::numeric_limits<Real>::epsilon()*1.0e-3;
		Real a,b,d=0.0,etemp,fu,fv,fw,fx;
		Real p,q,r,tol1,tol2,u,v,w,x,xm;
		Real e=0.0;
	
		a=(ax < cx ? ax : cx);
		b=(ax > cx ? ax : cx);
		x=w=v=bx;
		fw=fv=fx=func(x);
		for (Int iter=0;iter<ITMAX;iter++) {
			xm=0.5*(a+b);
			tol2=2.0*(tol1=tol*ABS(x)+ZEPS);
			if (ABS(x-xm) <= (tol2-0.5*(b-a))) {
				fmin=fx;
				return xmin=x;
			}
			if (ABS(e) > tol1) {
				r=(x-w)*(fx-fv);
				q=(x-v)*(fx-fw);
				p=(x-v)*q-(x-w)*r;
				q=2.0*(q-r);
				if (q > 0.0) p = -p;
				q=ABS(q);
				etemp=e;
				e=d;
				if (ABS(p) >= ABS(0.5*q*etemp) || p <= q*(a-x)
						|| p >= q*(b-x))
					d=CGOLD*(e=(x >= xm ? a-x : b-x));
				else {
					d=p/q;
					u=x+d;
					if (u-a < tol2 || b-u < tol2)
						d=SIGN(tol1,xm-x);
				}
			} else {
				d=CGOLD*(e=(x >= xm ? a-x : b-x));
			}
			u=(ABS(d) >= tol1 ? x+d : x+SIGN(tol1,d));
			fu=func(u);
			if (fu <= fx) {
				if (u >= x) a=x; else b=x;
				shft3(v,w,x,u);
				shft3(fv,fw,fx,fu);
			} else {
				if (u < x) a=u; else b=u;
				if (fu <= fw || w == x) {
					v=w;
					w=u;
					fv=fw;
					fw=fu;
				} else if (fu <= fv || v == x || v == w) {
					v=u;
					fv=fu;
				}
			}
		}
		ERR("Too many iterations in brent");
	}
};
struct Dbrent : Bracketmethod {
	Real xmin,fmin;
	const Real tol;
	Dbrent(const Real toll=3.0e-8) : tol(toll) {}
	template <class T>
	Real minimize(T &funcd)
	{
		const Int ITMAX=100;
		const Real ZEPS=std::numeric_limits<Real>::epsilon()*1.0e-3;
		Bool ok1,ok2;
		Real a,b,d=0.0,d1,d2,du,dv,dw,dx,e=0.0;
		Real fu,fv,fw,fx,olde,tol1,tol2,u,u1,u2,v,w,x,xm;
	
		a=(ax < cx ? ax : cx);
		b=(ax > cx ? ax : cx);
		x=w=v=bx;
		fw=fv=fx=funcd(x);
		dw=dv=dx=funcd.df(x);
		for (Int iter=0;iter<ITMAX;iter++) {
			xm=0.5*(a+b);
			tol1=tol*ABS(x)+ZEPS;
			tol2=2.0*tol1;
			if (ABS(x-xm) <= (tol2-0.5*(b-a))) {
				fmin=fx;
				return xmin=x;
			}
			if (ABS(e) > tol1) {
				d1=2.0*(b-a);
				d2=d1;
				if (dw != dx) d1=(w-x)*dx/(dx-dw);
				if (dv != dx) d2=(v-x)*dx/(dx-dv);
				u1=x+d1;
				u2=x+d2;
				ok1 = (a-u1)*(u1-b) > 0.0 && dx*d1 <= 0.0;
				ok2 = (a-u2)*(u2-b) > 0.0 && dx*d2 <= 0.0;
				olde=e;
				e=d;
				if (ok1 || ok2) {
					if (ok1 && ok2)
						d=(ABS(d1) < ABS(d2) ? d1 : d2);
					else if (ok1)
						d=d1;
					else
						d=d2;
					if (ABS(d) <= ABS(0.5*olde)) {
						u=x+d;
						if (u-a < tol2 || b-u < tol2)
							d=SIGN(tol1,xm-x);
					} else {
						d=0.5*(e=(dx >= 0.0 ? a-x : b-x));
					}
				} else {
					d=0.5*(e=(dx >= 0.0 ? a-x : b-x));
				}
			} else {
				d=0.5*(e=(dx >= 0.0 ? a-x : b-x));
			}
			if (ABS(d) >= tol1) {
				u=x+d;
				fu=funcd(u);
			} else {
				u=x+SIGN(tol1,d);
				fu=funcd(u);
				if (fu > fx) {
					fmin=fx;
					return xmin=x;
				}
			}
			du=funcd.df(u);
			if (fu <= fx) {
				if (u >= x) a=x; else b=x;
				mov3(v,fv,dv,w,fw,dw);
				mov3(w,fw,dw,x,fx,dx);
				mov3(x,fx,dx,u,fu,du);
			} else {
				if (u < x) a=u; else b=u;
				if (fu <= fw || w == x) {
					mov3(v,fv,dv,w,fw,dw);
					mov3(w,fw,dw,u,fu,du);
				} else if (fu < fv || v == x || v == w) {
					mov3(v,fv,dv,u,fu,du);
				}
			}
		}
		ERR("Too many iterations in routine dbrent");
	}
};

#endif /* _MIN1DIM_H_ */
