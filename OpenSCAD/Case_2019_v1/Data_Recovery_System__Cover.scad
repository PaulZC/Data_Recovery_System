// Cover for the Data Recovery System

$fn=90; // fragments

pcb_width = 108; // PCB width (X)
pcb_length = 305; // PCB length (Y)
pcb_thickness = 2.0; // PCB thickness (includes clearance)

module PCB()
{
    translate([0,0,(0-(pcb_thickness/2))])
        union()
        {
            cylinder(h=pcb_thickness, r=(pcb_width / 2));
            translate([(0-(pcb_width/2)),0,0])
                cube([pcb_width, (pcb_length - (pcb_width / 2)), pcb_thickness]);
        }
}

pi_height = 25.0; // Height of the Pi mounting screws above the PCB

USB = 1.1; // Width clearance for the USB connector

module void()
{
    translate([0,0,(0-wall)])
        union()
        {
            cylinder(h=(pi_height + (pcb_thickness/2) + wall), r=((pcb_width / 2) + USB));
            translate([(0-((pcb_width/2)+USB)),0,0])
                cube([(pcb_width + (2 * USB)), (pcb_length - (pcb_width / 2)), (pi_height + (pcb_thickness/2) + wall)]);
        }
}

wall = 3.0; // wall thickness

module floor()
{
    translate([(0-((pcb_width/2)+USB+(2*wall))),(0-((pcb_width / 2) + USB + (2 * wall))),(0-(2 * wall))])
        cube([(pcb_width + (2 * USB) + (4 * wall)), (pcb_length + USB + (4 * wall)), (2 * wall)]);
}

module shell()
{
    difference()
    {
        minkowski()
        {
            void();
            sphere(r=wall);
        }
        floor();
        void();
    }
}

cross_wall_y = 205; // Y position of cross wall

module cross_wall()
{
    wall_width = (pcb_width + (2 * USB) + wall);
    translate([(0-(wall_width / 2)),(cross_wall_y - (pcb_width / 2)), 0])
        cube([wall_width, wall, (pi_height + (wall / 2) + (pcb_thickness / 2))]);
}

battery_wall_y = 128; // Y position of cross wall above battery
battery_height = 18; // Z height of the battery above the PCB surface
battery_tab_length = 25; // Y length of battery tab
battery_tab_offset = 20; // X offset of right side of battery tab

module battery_wall()
{
    wall_width = (pcb_width + (2 * USB) + wall);
    translate([(0-(wall_width / 2)),(battery_wall_y - ((pcb_width / 2) + (wall/2))), (battery_height + (pcb_thickness / 2))])
        cube([wall_width, wall, (pi_height + (wall / 2) - battery_height)]);
}

module battery_tab()
{
    translate([(0-(battery_tab_offset + wall)),(battery_wall_y - ((pcb_width / 2) + (battery_tab_length/2))), 0])
        cube([wall, battery_tab_length, (pi_height + (wall / 2) + (pcb_thickness / 2))]);    
}

antenna_width = 20; // X width of antenna slot
antenna_height = 12; // Z height of antenna slot above PCB (worst case)

module antenna_wall()
{
    wall_width = (pcb_width + (2 * USB) + wall);
    difference()
    {
        translate([(0-(wall_width / 2)), 0, 0])
            cube([wall_width, wall, (pi_height + (wall / 2) + (pcb_thickness / 2))]);
        translate([(0-(antenna_width / 2)), (0-(wall/2)), 0])
            cube([antenna_width, (2*wall), (antenna_height + (pcb_thickness / 2))]);
    }
}

pi_usb_wall_y = 164.5; // Y position of the Pi USB retaining wall
pi_usb_wall_offset = 47; // X offset of the left side of the USB retaining wall
pi_usb_wall_length = 28; // Y length of the USB retaining wall

module pi_usb_wall()
{
    translate([pi_usb_wall_offset,(pi_usb_wall_y - ((pcb_width / 2) + (pi_usb_wall_length/2))), 0])
        cube([wall, pi_usb_wall_length, (pi_height + (wall / 2) + (pcb_thickness / 2))]);    
}

shark_width = 28; // X width of the shark opening
shark_offset = 6; // X offset of the shark opening
shark_height = 13.5; // Z height of the shark opening above the PCB

module shark()
{
    translate([(0-shark_offset),(pcb_length - ((pcb_width/2) + (wall/2))), (0-(pcb_thickness / 2))])
        cube([shark_width, (2 * wall), (shark_height + pcb_thickness)]);    
}

module nose()
{
    translate([(0-(wall/2)),(0-((pcb_width/2) + USB + (wall/2))), 0])
        cube([wall, ((2 * wall) + USB), (pi_height + (wall / 2) + (pcb_thickness / 2))]);    
}

tab_width = 0.8; // tab X width
tab_reduce = 10; // start and end the tab this far from the end of each straight edge
tab_height = 1; // tab Z height
slot_gap_x = 0.2; // total X slot clearance
slot_gap_y = 2; // total Y slot clearance
slot_gap_z = 0.5; // Z slot clearance

module tab()
{
    translate([(0-((pcb_width/2) + USB + tab_width + ((wall-tab_width)/2))), tab_reduce, (0 - tab_height)])
        cube([tab_width, (pcb_length - ((pcb_width / 2) + (2 * tab_reduce))), (2 * tab_height)]);    
}

module slot()
{
    slot_width = tab_width + slot_gap_x;
    translate([((pcb_width/2) + USB + ((wall-slot_width)/2)), (tab_reduce - (slot_gap_y / 2)), (0 - (tab_height + slot_gap_z))])
        cube([slot_width, (pcb_length  + slot_gap_y - ((pcb_width / 2) + (2 * tab_reduce))), (2 * (tab_height + slot_gap_z))]);    
}

module finished_shell()
{
    difference()
    {
        union()
        {
            shell();
            cross_wall();
            battery_wall();
            battery_tab();
            antenna_wall();
            pi_usb_wall();
            nose();
            tab();
        }
        PCB();
        shark();
        slot();
    }
}

module flipped_shell()
{
    translate([0,0,((pcb_thickness/2)+pi_height+wall)])
        rotate(a=[0,180,0])
            finished_shell();
}

flipped_shell();
