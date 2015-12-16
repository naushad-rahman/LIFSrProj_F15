//This is the holder for the PMT

module wall(x,y,z) {
	 color("yellow") cube(size=[x,y,z], center=false);
}

module slab(x,y,z) {
  color("green") cube(size=[x,y,z], center=false);
}

module PMT_slot(x,y,z) {
  color("red") cube(size=[x,y,z], center=false);
}

difference(){
	wall(70,40,3);
	translate([20,30,0]) cylinder(h = 21, r=4, center = true, $fn=100);
	translate([50,30,0]) cylinder(h = 21, r=4, center = true, $fn=100);
}

difference(){
	slab(70, 3, 40);

}

	translate([0, 0, 35]) PMT_slot(70, 20,5);
	translate([0, 0, 20]) PMT_slot(70, 20,5);






