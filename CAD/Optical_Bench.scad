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
            import("C:\\Users\\Canute\\Documents\\OpenSCAD\\F14_CAD\\InventorParts\\490OpticalSystem.stl", convexity=10);//Original Bench part
            translate([3.3,0,2]) cube([1.75,2.4,1.55]);
            // erases light well so it can be shifted
    difference(){  
        color([0,0,0,])translate([2.25,-1.25,-4]) cube([4,5,6]);
        color([0,0,0,])translate([2.5,-.75,-3.7]) cube([5,4,5.25]);
        color([0,0,0,])translate([-1,0,-3.1]) cube([10,2,2]);
         
    }     
        }
        color([1,0,0]) translate([1.3,.77,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) translate([3.8,.77,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) translate([3.8,3.3,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) translate([-3.7,3.3,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) translate([-3.7,.77,4.5]) cylinder(r=.75, h=1, $fn=20);
        color([1,0,0]) rotate([90,0,0]) translate([-1.3,5,-3.3]) cylinder(r=.4, h=2, $fn=20);
        //Takes off top at angle so it will fit under sample holder when attached to table
        color([0,0,0])translate([0,0,-4.7]) rotate([30,0,0]) translate([0,x/2,-2]) cube([3*x,2*x,4], center=true);
        color([1,1,1]) translate([-5.8,0,-4]) cube([3.55,3,8.75]); 
        color ([1,0,0]) translate([-2.5,2,-4]) cube([1, 1, 5]); 
    };
    
    difference(){
        union () {
            // imports new bench which is then shifted and extraneous parts are removed
    translate([-0.4, 0, 0]) color([1,1,1]) import("C:\\Users\\Canute\\Documents\\OpenSCAD\\F14_CAD\\InventorParts\\490OpticalSystem.stl", convexity=10);
           // these clean up the design so it's not so rough looking
        color([1,0,0]) translate([-2.9, 0, -3]) cube([.8, 2, 1]);
        color([1,0,0]) translate([-2.7, 0, -.9]) cube([.5, 2, .5]);
        color([1,0,0]) translate([-3.11, 2, -2.9]) cube ([.3, 1, .5]);
            }
          // removes most of the extraneous imported bench
        color([0, 0 , 0]) translate([1.3,1,0]) cube([7,6,15], center = true);
          // angles the top as above
        color([0,0,0])translate([0,0,-4.7]) rotate([30,0,0]) translate([0,x/2,-2]) cube([3*x,2*x,4], center=true);
          // more extraneous stuff removal
            color ([0, 1 , 0]) translate([-2.45, .2, -2.7]) rotate([0,45,0]) cube([.15, 1.8, 1.85]) ;
       color([0, 1, 1]) translate([-4.8,0, 2]) cube ([7,5,4]);
    }

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
    color([0, 1, 1]) translate ([-2.7, 0, 4.45]) cube([.5, .9, .35]);
};