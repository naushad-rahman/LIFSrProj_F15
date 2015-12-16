This folder contains all of the 3D printed parts and dependencies for the optical bench system current for Senior Project Fall 2015.
That includes parts that have been deemed satisfactory (XYZ stage) and those still undergoing iterations as of Fall 2015 (optical bench).
If you would prefer to use Inventor, that would require redoing most of the work that has been done in OpenScad, but I included the .ipt files of the dependencies just in case.

**NOTE** To familiarize yourself with OpenSCAD, consider starting with the following links:
http://www.openscad.org/documentation.html
https://en.wikibooks.org/wiki/OpenSCAD_User_Manual

**Note** Any .scad file with a dependency needs to have the file path for the dependency updated. The .ipt files are not required for dependency, they are included to be thorough.


List of files that should be found: (If not found, previous iterations available on previous year's wikis)

- 490HV_SampleConnector.ipt	(Inventor file for electrical lead holder. Not currently printed)
- 490HV_SampleConnector.stl	(3D view/print file)
- 490OpticalSystem.ipt		(.ipt file for dependency)
- 490OpticalSystem.stl		(Dependency for Optical_Bench.scad)
- Optical_Bench.scad		(OpenScad file of the optical bench)
- Optical_Benc_current.stl	(3D view/print file)
- Part1Test1.stl 		(3D view/print file for current PMT holder)
- PMT_holder.scad 		(the OpenSCAD file is not current, but this is the closest file I could find)
- Sample_Holder.scad		(OpenScad file of sample holder)
- Sample_Holder.stl		(3D view/print file)
- sampleholderdraft5.ipt	(.ipt file for dependency)
- sampleholderdraft5.stl	(Dependency for Sample_Holder.scad)

