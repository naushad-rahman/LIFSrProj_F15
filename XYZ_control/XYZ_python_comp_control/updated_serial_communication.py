
# coding: utf-8

# In[13]:

import sys
import glob
import serial

from IPython.html.widgets import interact, interactive, fixed
from IPython.html import widgets
from IPython.display import clear_output, display, HTML


# In[14]:

def serial_ports():
    """Lists serial ports

    :raises EnvironmentError:
        On unsupported or unknown platforms
    :returns:
        A list of available serial ports
    """
    if sys.platform.startswith('win'):
        ports = ['COM' + str(i + 1) for i in range(256)]

    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this is to exclude your current terminal "/dev/tty"
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


# In[15]:

current_ports = serial_ports()
ser = serial.Serial(current_ports[0], 9600, timeout=1)
print 'Running on serial port ' + current_ports[0]


# In[16]:

#define interface items that are not buttons
#-------- Serial Port Dropdown -------------
port_select = widgets.Dropdown(values=serial_ports())

def change_port(name, value):
    ser.port = value
    print "Serial port changed to " + ser.port

port_select.on_trait_change(change_port, 'value')

#--------- Travel Distance Box --------------
distance_text = widgets.IntText(description="Travel Distance (in micron): ")


# In[19]:

#define buttons
container_x = widgets.Box()
container_y = widgets.Box()
container_z = widgets.Box()
container_all = widgets.Box()

title_x = widgets.Latex(value="X Axis")
title_y = widgets.Latex(value="Y Axis")
title_z = widgets.Latex(value="Z Axis")

x_plus = widgets.Button(description='+')
x_minus = widgets.Button(description='-')

y_plus = widgets.Button(description='+')
y_minus = widgets.Button(description='-')

z_plus = widgets.Button(description='+')
z_minus = widgets.Button(description='-')

#Assign buttons and labels to containers
container_x.children = [title_x,x_plus,x_minus]
container_y.children = [title_y,y_plus,y_minus]
container_z.children = [title_z,z_plus,z_minus]
container_all.children = [container_x,container_y,container_z]


# In[18]:

#button action methods
def send_movement_command(XYZ_set, axis, direction, distance):
    to_write = str(XYZ_set)+':'+str(axis)+':'+str(direction)+':'+str(distance)+"\n"
    ser.write(to_write)
    print "Sent: " + to_write
    
def move_x_plus(button):
    send_movement_command(1,'x','+',distance_text.value)

def move_y_plus(button):
    send_movement_command(1,'y','+',distance_text.value)
    
def move_z_plus(button):
    send_movement_command(1,'z','+',distance_text.value)
    
def move_x_minus(button):
    send_movement_command(1,'x','-',distance_text.value)
    
def move_y_minus(button):
    send_movement_command(1,'y','-',distance_text.value)

def move_z_minus(button):
    send_movement_command(1,'z','-',distance_text.value)
    
#button action assignments
x_plus.on_click(move_x_plus)
x_minus.on_click(move_x_minus)
y_plus.on_click(move_y_plus)
y_minus.on_click(move_y_minus)
z_plus.on_click(move_z_plus)
z_minus.on_click(move_z_minus)


# In[7]:

#define and set styles
title_css = {"font-size":"20px",
             "text-decoration":"underline",
             "margin-bottom":"10px"}
button_css = {"width":"40px",
              "margin":"0 auto 2px"}
container_css = {"margin-right":"10px"}
container_all_css = {"flex-direction":"row"}

title_x.set_css(title_css)
title_y.set_css(title_css)
title_z.set_css(title_css)

x_plus.set_css(button_css)
x_minus.set_css(button_css)
y_plus.set_css(button_css)
y_minus.set_css(button_css)
z_plus.set_css(button_css)
z_minus.set_css(button_css)

container_x.set_css(container_css)
container_y.set_css(container_css)
container_z.set_css(container_css)

container_all.set_css(container_all_css)


# In[ ]:

display(port_select)
display(distance_text)
display(container_all)


# In[ ]:

ser.readline()


# In[ ]:




# In[ ]:




# In[ ]:




# In[ ]:




# In[ ]:




# In[ ]:



