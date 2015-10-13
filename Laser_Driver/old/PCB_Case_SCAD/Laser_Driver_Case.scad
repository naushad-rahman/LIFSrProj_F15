difference()
{
    cube([36,43,2]);
    
    translate([11,38,-2]) cylinder(h=4,r1=2,r2=2);
    translate([11,38,1]) cylinder(h=1,r1=4,r2=4);
}
difference()
{
    translate([0,0,2]) cube([36,3,15]);
    
    translate([0,0,2]) cube([11,3,15]);
}
difference()
{
    translate([0,31,2]) cube([36,3,15]);
    
    translate([5,31,2]) cube([6,3,15]);
}
difference()
{
    translate([0,0,2]) cube([3,33,15]);
    
    translate([0,0,2]) cube([3,8,15]);
}
difference()
{
    translate([33,0,2]) cube([3,33,15]);
    translate([33,6,2]) cube([3,10,15]);
    translate([33,20,2]) cube([3,10,15]);
}
