#ifndef POINT_H_
#define POINT_H_

/** Point.h
 * 
 * by Nate Solyntjes
 */


class Point {
	public:
	uint16_t x,y;

	Point(uint16_t x0=0, uint16_t y0=0){
		x = x0; 
		y = y0;
	}
	/** copy constructor? */
	Point(Point& p){
		Point(p.x, p.y);
	}
	virtual ~Point(){
		
	}
};

#endif /* POINT_H_ */