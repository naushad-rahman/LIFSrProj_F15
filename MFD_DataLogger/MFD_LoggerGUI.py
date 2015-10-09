from Tkinter import *
import os


master = Tk()
master.wm_title("Dr. Nordin's Superfriends")

def write_data():
	titlename = 'Device_[%s].txt' % (e1.get())
	if(os.path.isfile(titlename)):
		print 'MFD DATA ALREADY EXISTS!'

	else:
		dir_path = os.path.dirname(os.path.realpath(__file__))
		save_path = dir_path + '/MFD_Data/'
		print "THIS: " + save_path
		completeName = os.path.join(save_path, titlename)
		f = open(completeName, 'w')
		f.write('Device Number: \t\t\t\t\t%s\n' % e1.get())
		f.write('Date: \t\t\t\t\t\t%s\n' % e2.get())
		f.write('Template Status (Cleanliness/Usage): \t\t%s\n' % e3.get())
		f.write('Reservoir Glass Color/Type: \t\t\t%s\n' % e4.get())
		f.write('Time Imprinting in Oven: \t\t\t%s\n' % e5.get())
		f.write('Time Flattening in Oven: \t\t\t%s\n' % e6.get())
		f.write('Temperature of Oven1 (Hotter Oven): \t\t%s\n' % e7.get())
		f.write('Temperature of Oven2 (Colder Oven): \t\t%s\n' % e8.get())
		f.write('Time Cooling - Flattening: \t\t\t%s\n' % e9.get())
		f.write('Time Cooling - Imprinting: \t\t\t%s\n' % e10.get())
		f.write('Time in Oven Bonding: \t\t\t\t%s\n' % e11.get())
		f.write('Time Cooling after Final Bonding: \t\t%s\n' % e12.get())
		f.write('MFD Status: \t\t\t\t\t%s\n' % e13.get())
		f.write('Initials: \t\t\t\t\t%s\n' % e14.get())
		f.close()
		exit()

Label(master, text="Device Number").grid(row=0)
Label(master, text="Date (MM/DD/YY)").grid(row=1)
Label(master, text="Template Status").grid(row=2)
Label(master, text="Reservoir Glass Color").grid(row=3)
Label(master, text="Time Imprinting in Oven").grid(row=4)
Label(master, text="Time Flattening in Oven").grid(row=5)
Label(master, text="Temperature of Oven1 (Hotter)").grid(row=6)
Label(master, text="Temerature of Oven2 (Cooler)").grid(row=7)
Label(master, text="Time Cooling - Flattening").grid(row=8)
Label(master, text="Time Cooling - Imprinting").grid(row=9)
Label(master, text="Time in Oven Bonding").grid(row=10)
Label(master, text="Time Cooling after Final Bonding").grid(row=11)
Label(master, text="MFD Status (Success/Fail)").grid(row=12)
Label(master, text="Initials").grid(row=13)
Button(master, text = "SUBMIT DATA", command = write_data).grid(row=14)

e1 = Entry(master)
e2 = Entry(master)
e3 = Entry(master)
e4 = Entry(master)
e5 = Entry(master)
e6 = Entry(master)
e7 = Entry(master)
e8 = Entry(master)
e9 = Entry(master)
e10 = Entry(master)
e11 = Entry(master)
e12 = Entry(master)
e13 = Entry(master)
e14 = Entry(master)

e1.grid(row=0, column=1)
e2.grid(row=1, column=1)
e3.grid(row=2, column=1)
e4.grid(row=3, column=1)
e5.grid(row=4, column=1)
e6.grid(row=5, column=1)
e7.grid(row=6, column=1)
e8.grid(row=7, column=1)
e9.grid(row=8, column=1)
e10.grid(row=9, column=1)
e11.grid(row=10, column=1)
e12.grid(row=11, column=1)
e13.grid(row=12, column=1)
e14.grid(row=13, column=1)

mainloop( )