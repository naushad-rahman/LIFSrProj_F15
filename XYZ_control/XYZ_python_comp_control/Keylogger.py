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
command_count = 0

def setTextBox(start, end, text):
	textBox.delete(start, end)
	textBox.insert(start, text)

def key(event):
	if event.keysym == 'Escape':
		root.destroy()
	if event.char == event.keysym:
		global distance_text
		global command_count
		message = "0"
		#This is a normal number or letter char
		#insert what you want it to do here
		print( 'Normal Key %r' % event.char )
		if event.char == '4':
			message = "1500"
			distance_text = 192
			print( 'Distance set to ' + message)
			setTextBox("1.0", "1.24", "Distance changed to " + message)
		elif event.char == '3':
			message = "125"
			print( 'Distance set to ' + message )
			distance_text = 16
			setTextBox("1.0", "1.24", "Distance changed to " + message)
		elif event.char == '2':
			message = "46.875"
			print( 'Distance set to ' + message )
			distance_text = 6
			setTextBox("1.0", "1.24", "Distance changed to " + message)
		elif event.char == '1':
			message = "7.8"
			print( 'Distance set to ' + message )
			distance_text = 1
			command_count += 1
			setTextBox("1.0", "1.24", "Distance changed to " + message)
		elif event.char == 'w':
			print( 'Moving up' )
			send_movement_command(1,'y','+',distance_text)
		elif event.char == 'a':
			print( 'Moving left' )
			send_movement_command(1,'x','-',distance_text)
		elif event.char == 's':
			print( 'Moving down' )
			send_movement_command(1,'y','-',distance_text)
		elif event.char == 'd':
			print( 'Moving right' )
			send_movement_command(1,'x','+',distance_text)
            		#print('Command count ' + str(command_count))
		elif event.char == 'r':
			print( 'Moving zup' )
			send_movement_command(1,'z','+',distance_text)
		elif event.char == 'f':
			print( 'Moving zdown' )
			send_movement_command(1,'z','-',distance_text)
		elif event.char == 'z':
			print( 'Testing Z movement' )
			runTestZ()
		elif event.char == 'x':
			print( 'Testing X movement' )
			runTestX()
		elif event.char == 'y':
			print( 'Testing Y movement' )
			runTestY()
		elif event.char == 'm':
			runMicroStep()
        else:
            print('Nothing doing...')
                
def change_port(value):
	ser.port = value
	print "Serial port changed to " + ser.port
#	current_ports[0] = value
#	print "Port changed to " + str(value)
	portBox.delete("1.0", END)
	portBox.insert(INSERT, "Serial port changed to " + ser.port)
	
def runTestZ(): 
	send_movement_command(1,'z','-',1500)
	time.sleep(1)
	send_movement_command(1,'z','+',1500)
	time.sleep(1)
	send_movement_command(1,'z','-',200)
	time.sleep(1)
	send_movement_command(1,'z','+',200)
	time.sleep(1)
	send_movement_command(1,'z','-',10)
	time.sleep(1)
	send_movement_command(1,'z','+',10)
    
def runMicroStep():
	for x in range(0,200):
		send_movement_command(1,'y','-',1)
		send_movement_command(1,'x','+',1)
		send_movement_command(1,'z','-',1)
		time.sleep(1)

def runTestX(): 
	send_movement_command(1,'x','-',1500)
	time.sleep(1)
	send_movement_command(1,'x','+',1500)
	time.sleep(1)
	send_movement_command(1,'x','-',200)
	time.sleep(1)
	send_movement_command(1,'x','+',200)
	time.sleep(1)
	send_movement_command(1,'x','-',10)
	time.sleep(1)
	send_movement_command(1,'x','+',10)
	
def runTestY(): 
	send_movement_command(1,'y','-',1500)
	time.sleep(1)
	send_movement_command(1,'y','+',1500)
	time.sleep(1)
	send_movement_command(1,'y','-',200)
	time.sleep(1)
	send_movement_command(1,'y','+',200)
	time.sleep(1)
	send_movement_command(1,'y','-',10)
	time.sleep(1)
	send_movement_command(1,'y','+',10)

def send_movement_command(XYZ_set, axis, direction, distance):
    to_write = str(XYZ_set)+':'+str(axis)+':'+str(direction)+':'+str(distance)+"\n"
    ser.write(to_write)
    print "Sent: " + to_write
print("Command count " + str(command_count))

root = tk.Tk()
menubar = Menu(root)
portmenu = Menu(menubar, tearoff=0)
textBox = Text(root)
portBox = Text(root)
textBox.insert(INSERT, "Distance changed to " + str(distance_text))
portBox.insert(INSERT, "Port is set to " + str(current_ports[0]))
textBox.pack()
portBox.pack()

for x in current_ports:
	portmenu.add_command(label=str(x), command=lambda x=x: change_port(x))

menubar.add_cascade(label="Ports...", menu=portmenu)
print( "Press a key. Let's see if this works...")
root.bind_all('<Key>', key)
#this hides the GUI window. We can replace this soon
#root.withdraw()
root.config(menu=menubar)
root.mainloop()