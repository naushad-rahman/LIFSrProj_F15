difference(){
union(){
translate([0,3.35,0]) {
    difference(){
        rotate([90,0,0]) import("C:\\Users\\Canute\\Documents\\OpenSCAD\\SpSu14_CAD\\sampleholderdraft5.stl", convexity=10);
        translate([-6,-3.35,-1]) cube([10,10,2]);
        color([1,1,1]) translate([-1,-3.35,1.45]) cube([6,2,.9], center = true);
    }
};
color([1,0,0]) translate([-8.55,4.75,0])  rotate([90,0,90]) import("C:\\Users\\Canute\\Documents\\OpenSCAD\\SpSu14_CAD\\sampleholderdraft5.stl", convexity=10);
intersection(){
    color([0,0,1]) translate([-5,-.2,0]) cube([8.25,8.2,.75]);
    color([0,1,0]) translate([0,-10,0]) rotate([0,0,45]) translate([1.1,-.2,-.5]) cube([8.25,28.25,1.5]);
};
};
color([1,1,0]) translate([-10,0,.5])cube([28.25,8.25,3]);
color([1,0,1]) translate([-9.79, 1.2, .35]) cube([2.5,5.3,.15]);
};
