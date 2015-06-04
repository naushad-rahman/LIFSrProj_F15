difference(){
import("C:\\Users\\Trevor\\Documents\\GitHub\\LIFSrProj_F14\\InventorParts\\490OpticalSystem.stl", convexity=10);
color([1,0,0]) translate([1.4,.8,4.5]) cylinder(r=.6, h=1, $fn=20);
color([1,0,0]) translate([3.9,.8,4.5]) cylinder(r=.6, h=1, $fn=20);
color([1,0,0]) translate([3.9,3.2,4.5]) cylinder(r=.6, h=1, $fn=20);
color([1,0,0]) translate([-3.7,3.2,4.5]) cylinder(r=.6, h=1, $fn=20);
color([1,0,0]) translate([-3.7,.8,4.5]) cylinder(r=.6, h=1, $fn=20);
color([1,0,0]) rotate([90,0,0]) translate([-1.3,5,-3.3]) cylinder(r=.4, h=2, $fn=20);
};


rotate([180,0,180]) translate([-.1,2,-5.6]) difference(){
   cube([10.4,4,1], center = true);
   color([1,0,0]) translate([0,5,.6]) rotate([13,0,0]) cube([11,20,1], center = true);
};