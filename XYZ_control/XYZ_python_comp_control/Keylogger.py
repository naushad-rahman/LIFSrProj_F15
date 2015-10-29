import Tkinter as tk
from Tkinter import *
import sys
import glob
import serial
import time

def serial_ports():
	if sys.platform.startswith('win'):
		ports = ['COM' + str(i + 1) for i in range(256)]
	elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
		ports = glob.glob('/dev/tty[A-Za-z]*')
	elif sys.platform.startswith('darwin'):
		ports = glob.glob('/dev/tty.*')
	else:
		raise EnvironmentError('Unsupported platform')

	result = []
	for port in ports:
		try:
			s = serial.Serial(port)
			s.close()
			result.append(port)
		except (OSError, serial.SerialException):
			pass
	return result

current_ports = serial_ports()
#current_ports = [1,2,3,4,5]
ser = serial.Serial(current_ports[0], 9600, timeout=1)
print 'Running on serial port (to be configured)'
distance_text = 0
global x_pos
global y_pos
global z_pos

def setTextBox(start, end, text):
	textBox.delete(start, end)
	textBox.insert(start, text)

def key(event):
	if event.keysym == 'Escape':
		writeFile()
		root.destroy()
	if event.char == event.keysym:
		global distance_text
		global command_count
		global x_pos
		global y_pos
		global z_pos
		message = "0"
		#This is a normal number or letter char
		#insert what you want it to do here
		print( 'Normal Key %r' % event.char )
		if event.char == '4':
			message = "1500"
			distance_text = 192
			print( 'Distance set to ' + message)
			setTextBox("1.0", "1.50", "Distance changed to " + message)
		elif event.char == '3':
			message = "125"
			print( 'Distance set to ' + message )
			distance_text = 16
			setTextBox("1.0", "1.50", "Distance changed to " + message)
		elif event.char == '2':
			message = "46.875"
			print( 'Distance set to ' + message )
			distance_text = 6
			setTextBox("1.0", "1.50", "Distance changed to " + message)
		elif event.char == '1':
			message = "7.8"
			print( 'Distance set to ' + message )
			distance_text = 1
			setTextBox("1.0", "1.50", "Distance changed to " + message)
		elif event.char == 'w':
			print( 'Moving up' )
			send_movement_command(1,'y','+',distance_text)
			y_pos += distance_text
		elif event.char == 'a':
			print( 'Moving left' )
			send_movement_command(1,'x','-',distance_text)
			x_pos -= distance_text
		elif event.char == 's':
			print( 'Moving down' )
			send_movement_command(1,'y','-',distance_text)
			y_pos -= distance_text
		elif event.char == 'd':
			print( 'Moving right' )
			send_movement_command(1,'x','+',distance_text)
			x_pos += distance_text
            		#print('Command count ' + str(command_count))
		elif event.char == 'r':
			print( 'Moving zup' )
			send_movement_command(1,'z','+',distance_text)
			z_pos += distance_text
		elif event.char == 'f':
			print( 'Moving zdown' )
			send_movement_command(1,'z','-',distance_text)
			z_pos -= distance_text
		elif event.char == 'o':
			setZero()
		elif event.char == 't':
			for x in range(200):
				send_movement_command(1,'x','+',1)
		elif event.char == 'p':
			setTextBox("1.0", "1.100", "Current position: " + str(x_pos) + ":" + str(y_pos) + ":" + str(z_pos))
	else:
            print('Nothing doing...')
                
def change_port(value):
	ser.port = value
	print "Serial port changed to " + ser.port
#	current_ports[0] = value
#	print "Port changed to " + str(value)
	portBox.delete("1.0", END)
	portBox.insert(INSERT, "Serial port changed to " + ser.port)

position = []

def readFile():
	print("Reading file...")
	file = open('position.txt', 'r')
	f = file.readlines()
	global position
	global x_pos
	global y_pos
	global z_pos
	pos = f[0].split(':')
	x_pos = int(pos[0])
	y_pos = int(pos[1])
	z_pos = int(pos[2])
	position = f[1:]
	print("Done.")
	
def writeFile():
	global x_pos
	global y_pos
	global z_pos
	global position
	print("Writing to file...")
	file = open('position.txt','w')
	file.writelines(str(x_pos) + ":" + str(y_pos) + ":" + str(z_pos) + "\n")
	file.writelines(position)

def send_movement_command(XYZ_set, axis, direction, distance):
	to_write = str(XYZ_set)+':'+str(axis)+':'+str(direction)+':'+str(distance)+"\n"
	ser.write(to_write)
	print "Sent: " + to_write
	setTextBox("1.0", "1.100", "Sent: " + to_write)

def send_position(command):
	com = command.split(':')
	global x_pos
	global y_pos
	global z_pos
	x_move = int(com[1])
	y_move = int(com[2])
	z_move = int(com[3])
	x_final = abs(x_pos - x_move)
	y_final = abs(y_pos - y_move)
	z_final = abs(z_pos - z_move)
	if x_move > x_pos:
		setDir = '+'
	else:
		setDir = '-'
	send_movement_command(1,'x',setDir,x_final)
	if y_move > y_pos:
		setDir = '+'
	else:
		setDir = '-'
	send_movement_command(1,'y',setDir,y_final)
	if z_move > z_pos:
		setDir = '+'
	else:
		setDir = '-'
	send_movement_command(1,'z',setDir,z_final)
	print "Sent position: " + command
	x_pos = x_move
	y_pos = y_move
	z_pos = z_move
	
def setZero():
	global x_pos
	global y_pos
	global z_pos
	x_pos = 0
	y_pos = 0
	z_pos = 0
	print("Set zero")
	
def setPosition(name):
	global x_pos
	global y_pos
	global z_pos
	global position
	setString = name
	t = 0
	for x in position:
		n = x.split(':')
		print(n[0] + " " + name)
		if (n[0] == name):
			setString += ":" + str(x_pos) + ":" + str(y_pos) + ":" + str(z_pos)
			position[t] = setString
		t += 1
	print("Position set." + str(position))
			

root = tk.Tk()
readFile()
menubar = Menu(root)
portmenu = Menu(menubar, tearoff=0)
positionmenu = Menu(menubar, tearoff=0)
set_position_menu = Menu(menubar, tearoff=0)
textBox = Text(root)
portBox = Text(root)
textBox.insert(INSERT, "Distance changed to " + str(distance_text))
portBox.insert(INSERT, "Port is set to " + str(current_ports[0]))
textBox.pack()
portBox.pack()

for x in current_ports:
	portmenu.add_command(label=str(x), command=lambda x=x: change_port(x))

for x in position:
	name = x.split(':')
	positionmenu.add_command(label=str(name[0]), command=lambda x=x: send_position(x))
	
for x in position:
	name = x.split(':')
	set_position_menu.add_command(label=str(name[0]), command=lambda x=x: setPosition(name[0]))

menubar.add_cascade(label="Ports...", menu=portmenu)
menubar.add_cascade(label="Positions...", menu=positionmenu)
menubar.add_cascade(label="Set Position", menu=set_position_menu)
#x_pos = 0
#y_pos = 0
#z_pos = 0
print( "Press a key. Let's see if this works...")
root.bind_all('<Key>', key)
#this hides the GUI window. We can replace this soon
#root.withdraw()
root.config(menu=menubar)
root.mainloop()
