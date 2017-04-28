#ifndef BOUNDS_H_
#define BOUNDS_H_ 

#include "Point.h"

class Bounds {
	public: 
	Point* p1 = NULL, *p2 = NULL;
	Bounds(Point& a, Point& b){
		*p1 = a;
		*p2 = b;
	}
	virtual ~Bounds(){
		delete p1;
		delete p2;
	}
};


#endif /* BOUNDS_H_ */