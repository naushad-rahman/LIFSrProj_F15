/*

TODO:
    Ensure each screw hole is aligned
    Etch into bottom angled layer

*/
x = 10.4;
x_pos = 3.8;
x_neg = -3.7;
y_for = .75;
y_back = 3.1;
union(){
    difference(){
        union(){
            import("C:\\Users\\Trevor\\Documents\\GitHub\\LIFSrProj_F14\\InventorParts\\490OpticalSystem.stl", convexity=10);
            translate([3.3,0,2]) cube([1.75,2.4,1.55]);
    difference(){  
            translate([2.25,-1.25,-4]) cube([4,5,6]);
        translate([2.5,-.75,-3.7]) cube([5,4,5.25]);
        translate([-1,0,-3.1]) cube([10,2,2]);
    }       
        }
        color([1,0,0]) translate([1.3,.77,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) translate([3.8,.77,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) translate([3.8,3.3,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) translate([-3.7,3.3,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) translate([-3.7,.77,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) rotate([90,0,0]) translate([-1.3,5,-3.3]) cylinder(r=.4, h=2, $fn=20);
        color([0,0,0])translate([0,0,-4.7]) rotate([30,0,0]) translate([0,x/2,-2]) cube([3*x,2*x,4], center=true);
        color([1,1,1]) translate([-5.8,2.3,-4]) cube([4,1,7]);
    };
    translate([-5.1,.5,3.5]) difference(){
        rotate([20,0,0]) difference(){
            cube([10.4,3.8,1.75]);
            color([1,0,1]) translate([(x-1.5),y_for,-10]) cylinder(r=.4, h=20, $fn=20);
        color([1,0,1]) translate([1.4,y_for,-10]) cylinder(r=.4, h=20, $fn=20);
        color([1,0,1]) translate([1.4,y_back,-10]) cylinder(r=.4, h=20, $fn=20);
        color([1,0,1]) translate([4,y_for,-10]) cylinder(r=.4, h=20, $fn=20);
        color([1,0,1]) translate([4,y_back,-10]) cylinder(r=.4, h=20, $fn=20);
        color([1,0,1]) translate([4,y_back,-10]) cylinder(r=.4, h=20, $fn=20);
        color([1,0,1]) translate([x-1.5,y_back,-10]) cylinder(r=.4, h=20, $fn=20);
        color([1,0,1]) translate([x-4,y_for,-10]) cylinder(r=.4, h=20, $fn=20);
        color([1,0,1]) translate([x-3.7,y_back,-10]) cylinder(r=.4, h=20, $fn=20);
        }
        translate([-1,-10,0]) rotate([0,0,0]) cube([12,20,1.5]);
    };
};