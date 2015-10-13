#HV Control &
#Read and Plot from the PMT

#This code is to record the data that is received into the Teensy's ADC.
#Includes the HV control and replotting the results at the end.

#See CSV Dataplot notebook to plot old experiment data.

from pyqtgraph import QtGui, QtCore #Provides usage of PyQt4's libraries which aids in UI design
import pyqtgraph as pg              #Initiation of plotting code
import serial                       #Communication with the serial port is done using the pySerial 2.7 package
from datetime import datetime       #Allows us to look at current date and time
from time import strftime           #Allows for better formatting of time
import numpy as np
from Tkinter import *               #For the notes prompt
import json                         #For saving tags
import threading                    #For multithreading
import Queue
import pprint                       #For pretty debug printing

## Always start by initializing Qt (only once per application)
app = QtGui.QApplication([])

## Define a top-level widget to hold everything (a window)
w = QtGui.QWidget()
w.resize(1000,600)
w.setWindowTitle('Voltage Plots')

startBtnClicked = False

## This function contains the behavior we want to see when the start button is clicked
def startButtonClicked():
    global startBtnClicked
    global startBtn
    if (startBtnClicked == False):
        teensySerialData.flushInput() #empty serial buffer for input from the teensy
        startBtnClicked = True
        startBtn.setText('Stop')

        #start threads
        serial_read_thread = serialReadThread()
        serial_read_thread.daemon = True
        serial_read_thread.start()
        data_converter_thread = dataConverterThread()
        data_converter_thread.daemon = True
        data_converter_thread.start()
        time_data_thread = timeDataThread()
        time_data_thread.daemon = True
        time_data_thread.start()
        pmt_data_thread = pmtDataThread()
        pmt_data_thread.daemon = True
        pmt_data_thread.start()
        pmt_graph_thread = pmtGraphThread()
        pmt_graph_thread.daemon = True
        pmt_graph_thread.start()
        data_write_thread = dataWriteThread()
        data_write_thread.daemon = True
        data_write_thread.start()
        #graphing_thread = graphingThread()
        #graphing_thread.daemon = True
        #graphing_thread.start()

    elif (startBtnClicked == True):
        startBtnClicked = False
        startBtn.setText('Start')

## Below at the end of the update function we check the value of quitBtnClicked
def quitButtonClicked():
    ## Close the file and close the window.
    if (startBtnClicked == False):  ##don't quit while still running.
        f.close()
        w.close()

        #showNow = True
        data = np.loadtxt(open('RecordedData\\' + fileName,"rb"),delimiter=",",skiprows=2)
        numSamples2 = data.shape[0]
        if(len(data) > 0):
            pmtCurve2.setData(data[:,1])

def quitButtonClicked2():
    w2.close()

## Buttons to control the High Voltage
def HVoffButtonClicked():
    teensySerialData.write('0')
    print("HV Off")

def HVonButtonClicked():
    teensySerialData.write('1')
    print("HV On")

def insertionButtonClicked():
    teensySerialData.write('3')
    print("Insertion")

def separationButtonClicked():
    teensySerialData.write('2')
    print("Separation")

#Start Recording in Widget
## Create widgets to be placed inside

startBtn = QtGui.QPushButton('Start')
startBtn.setToolTip('Click to begin graphing') #This message appears while hovering mouse over button

quitBtn = QtGui.QPushButton('Quit')
quitBtn.setToolTip('Click to quit program')

quitBtn2 = QtGui.QPushButton('Quit')
quitBtn2.setToolTip('Click to quit program')

HVonBtn = QtGui.QPushButton("HV on")
HVonBtn.setToolTip('Click to turn the high voltage on')

HVoffBtn = QtGui.QPushButton("HV off")
HVoffBtn.setToolTip('Click to turn the high voltage off')

insBtn = QtGui.QPushButton("Insertion")
insBtn.setToolTip('Click to start insertion (#3)')

sepBtn = QtGui.QPushButton("Separation")
sepBtn.setToolTip('Click to start separation (#2)')

## Functions in parantheses are to be called when buttons are clicked
startBtn.clicked.connect(startButtonClicked)
quitBtn.clicked.connect(quitButtonClicked)
quitBtn2.clicked.connect(quitButtonClicked2)
HVonBtn.clicked.connect(HVonButtonClicked)
HVoffBtn.clicked.connect(HVoffButtonClicked)
insBtn.clicked.connect(insertionButtonClicked)
sepBtn.clicked.connect(separationButtonClicked)

## xSamples is the maximum amount of samples we want graphed at a time
xSamples = 30000

## To plot the entire csv file after the experiment.
## Plots at the click of the quit button

## Define a top-level widget to hold everything
w2 = QtGui.QWidget()
w2.resize(1000,600)
w2.setWindowTitle('Voltage Plot')

#Create Plot Widgets
pmtPlotWidget2 = pg.PlotWidget()
pmtPlotWidget2.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
pmtPlotWidget2.setYRange(0, 4096)
pmtPlotWidget2.setLabel('top', text = "PMT")
pmtCurve2 = pmtPlotWidget2.plot()

lowerBoundValue2 = 0
upperBoundValue2 = xSamples
pmtPlotWidget2.setXRange(0, upperBoundValue2)

## Create a grid layout to manage the widgets size and position
layout2 = QtGui.QGridLayout()
w2.setLayout(layout2)

## Add widgets to the layout in their proper positions
layout2.addWidget(quitBtn2, 0, 0)
layout2.addWidget(pmtPlotWidget2, 1, 2, 1, 1)  # wGL goes on right side, spanning 3 rows

## Display the widget as a new window
w2.show()


## Create plot widget for PMT signal input
## For real time plotting
pmtPlotWidget = pg.PlotWidget()
pmtPlotWidget.setYRange(0, 4096)
pmtPlotWidget.setXRange(0, xSamples)
pmtPlotWidget.setLabel('top', text = "PMT") #Title to appear at top of widget

## Create a grid layout to manage the widgets size and position
## The grid layout allows us to place a widget in a given column and row
layout = QtGui.QGridLayout()
w.setLayout(layout)

## Add widgets to the layout in their proper positions
## The first number in parantheses is the row, the second is the column
layout.addWidget(quitBtn, 0, 0)
layout.addWidget(startBtn, 2, 0)
layout.addWidget(HVonBtn, 0, 2)
layout.addWidget(insBtn, 2, 2)
layout.addWidget(sepBtn, 3, 2)
layout.addWidget(HVoffBtn, 4, 2)

layout.addWidget(pmtPlotWidget, 1, 1)

## Display the widget as a new window
w.show()

## Initialize all global variables

## Whenever we plot a range of samples, xLeftIndex is the x value on the
## PlotWidget where we start plotting the samples, xRightIndex is where we stop
## These values will reset when they reach the value of xSamples
xRightIndex = 0
xLeftIndex = 0

## These arrays will hold the unplotted voltage values from the pmt
## and the peak detector until we are able to update the plot
pmtData = []

## Used to determine how often we plot a range of values
graphCount = 0

## Time values in microseconds read from the teensy are stored in these variables
## Before timeElapsed is updated, we store its old value in timeElapsedPrev
timeElapsed = 0
timeElapsedPrev = 0

## Determines if we are running through the update loop for the first time
firstRun = True

## Create new file, with the name being today's date and current time and write headings to file in CSV format
i = datetime.now()
#fileName = str(i.year) + str(i.month) + str(i.day) + "_" + str(i.hour) + str(i.minute) + str(i.second) + ".csv"
fileName = strftime("%Y%m%d_%H%M%S.csv")

## File is saved to Documents/IPython Notebooks/RecordedData
f = open('RecordedData\\' + fileName, 'a')
f.write("#Data from " + str(i.year) + "-" + str(i.month) + "-" + str(i.day) + " at " + str(i.hour) + ":" + str(i.minute) + ":" + str(i.second) + '\n')
f.write("Timestamp,PMT\n")

## Initialize the container for our voltage values read in from the teensy
## IMPORTANT NOTE: The com port value needs to be updated if the com value
## changes. It's the same number that appears on the bottom right corner of the
## window containing the TeensyDataWrite.ino code

teensySerialData = serial.Serial("COM4", 115200)
inputBytes = []
recieved_data = Queue.Queue()
time_data = Queue.Queue()
pmt_data = Queue.Queue()
time_write = Queue.Queue()
pmt_write = Queue.Queue()
pmt_graph = Queue.Queue()
graph_sema = threading.Semaphore()

class serialReadThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        ## Set global precedence to previously defined values
        global firstRun
        global startBtnClicked
        while (startBtnClicked):
            inputBytes = teensySerialData.read(size = 6)
            if (firstRun == True):
                ## Only run once to ensure buffer is completely flushed
                firstRun = False
                teensySerialData.flushInput()
                continue
            #Bytes read in and stored in a char array of size six
            recieved_data.put(inputBytes)

#this is the weakest link and needs to be sped up.
class dataConverterThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global startBtnClicked

        while (startBtnClicked):
            #Get data to work with from queue
            working_data = recieved_data.get()

            #The ord function converts a char to its corresponding ASCII integer, which we can then convert to a float
            timeByte3 = float(ord(working_data[0]))
            timeByte2 = float(ord(working_data[1]))
            timeByte1 = float(ord(working_data[2]))
            timeByte0 = float(ord(working_data[3]))
            pmtByte1 = float(ord(working_data[4]))
            pmtByte0 = float(ord(working_data[5]))

            recieved_data.task_done()
            time_data.put([timeByte0, timeByte1, timeByte2, timeByte3])
            pmt_data.put([pmtByte0, pmtByte1])
            #print("current size of queue: " + str(recieved_data.qsize()))

class timeDataThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global timeElapsed
        global timeElapsedPrev
        global startBtnClicked

        while (startBtnClicked):
            timeBytes = time_data.get()
            timeElapsedPrev = timeElapsed
            timeElapsed = timeBytes[3]*256*256*256 + timeBytes[2]*256*256 + timeBytes[1]*256 + timeBytes[0] #There are 8 bits in a byte, 2^8 = 256
            time_data.task_done()
            if (timeElapsedPrev == 0):
                timeElapsedPrev = timeElapsed   #So we won't get a warning on the first packet received.

            # We'll add all our values to this string until we're ready to exit the loop, at which point it will be written to a file
            time_write.put(str(timeElapsed))

            ## This difference calucalted in the if statement is the amount of time in microseconds since the last value
            ## we read in and wrote to a file. If this value is significantly greater than 100, we know we have missed some
            ## values, probably as a result of the buffer filling up and scrapping old values to make room for new values.
            ## The number we print out will be the approximate number of values we failed to read in.
            ## This is useful to determine if your code is running too slow
            #if (timeElapsed - timeElapsedPrev > 8000):
            #    print(str((timeElapsed-timeElapsedPrev)/7400))
            if (timeElapsed - timeElapsedPrev > 150):
                print("missed time: " + str((timeElapsed-timeElapsedPrev)/100))

class pmtDataThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global startBtnClicked

        while (startBtnClicked):
            pmtBytes = pmt_data.get()
            numData = pmtBytes[1]*256 + pmtBytes[0]
            numData = numData*3.3/1024
            numDataRounded = numData - numData%.001 #Round voltage value to 3 decimal points
            pmt_data.task_done()
            pmt_graph.put(numDataRounded)
            pmt_write.put(str(numDataRounded))

class pmtGraphThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global xRightIndex
        global pmtData
        global startBtnClicked

        while (startBtnClicked):
            numDataRounded = pmt_graph.get()
            graph_sema.acquire()
            pmtData.append(numDataRounded)
            xRightIndex = xRightIndex + 1
            graph_sema.release()
            pmt_graph.task_done()

class dataWriteThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global startBtnClicked

        while (startBtnClicked):
            localTimeElapsed = time_write.get()
            pmtNumDataRounded = pmt_write.get()

            stringToWrite = localTimeElapsed + "," + pmtNumDataRounded + '\n'
            f.write(stringToWrite)

            time_write.task_done()
            pmt_write.task_done()

#class graphingThread (threading.Thread):   #Use this instead of "def update():" to turn it back into a thread.
#    def __init__(self):
#        threading.Thread.__init__(self)
#    def run(self):
def update():
    global xLeftIndex
    global xRightIndex
    global pmtData
    global xSamples
    #global startBtnClicked

    #while (startBtnClicked):#used when this was a thread

    if (len(pmtData) >= 100): #We will plot new values once we have this many values to plot
        if (xLeftIndex == 0):
            ## Remove all PlotDataItems from the PlotWidgets. This will effectively reset the graphs (approximately every 30000 samples)
            pmtPlotWidget.clear()

        ## pmtCurve are of the PlotDataItem type and are added to the PlotWidget.
        ## Documentation for these types can be found on pyqtgraph's website

        graph_sema.acquire()

        pmtCurve = pmtPlotWidget.plot()
        xRange = range(xLeftIndex,xRightIndex)
        pmtCurve.setData(xRange, pmtData)

        ## Now that we've plotting the values, we no longer need these arrays to store them
        pmtData = []
        xLeftIndex = xRightIndex
        graphCount = 0
        if(xRightIndex >= xSamples):
            xRightIndex = 0
            xLeftIndex = 0
            pmtData = []

        graph_sema.release()
#    if (startBtnClicked):
#        print("recieved_data: " + str(recieved_data.qsize()) + "    time_data:" + str(time_data.qsize()) +
#        "    pmt_data:" + str(pmt_data.qsize()) + "    time_write:" + str(time_write.qsize()) +
#        "    pmt_write:" + str(pmt_write.qsize()) + "    pmt_graph:" + str(pmt_graph.qsize())

## Run update function in response to a timer
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)

## Start the Qt event loop
app.exec_()

# Prompts the user to add a description for the test data and saves it in RecordedData\TestNotes.txt
## Tkinter is used to create this window
testNotes = Tk()

## The label telling the user what to do
l = Label(testNotes, text = "Describe your experiment here.")
l.pack(padx = 10, pady = 5, anchor = W)

## The textbox where the notes are written
textBox = Text(testNotes)
textBox.pack(padx = 10, pady = 5)
textBox.focus_set()

## textbox for microfluidic device number
inputBoxFrame = Frame(testNotes)
inputBoxFrame.pack(pady = 5)
deviceNumLabel = Label(testNotes, text = "Microfluidic device number: ")
deviceNumEntry = Entry(testNotes)
deviceNumUsedLabel = Label(testNotes, text = "Times device has been used: ")
deviceNumUsedEntry = Entry(testNotes)
laserPosLabel = Label(testNotes, text = "Laser position: ")
laserPosEntry = Entry(testNotes)
analogGainLabel = Label(testNotes, text = "Analog gain: ")
analogGainEntry = Entry(testNotes)
laserVoltLabel = Label(testNotes, text = "Laser voltage: ")
laserVoltEntry = Entry(testNotes)
pmtVoltLabel = Label(testNotes, text = "PMT control voltage: ")
pmtVoltEntry = Entry(testNotes)
hvSettingsLabel = Label(testNotes, text = "HV settings: ")
hvSettingsEntry = Entry(testNotes)
buffSolLabel = Label(testNotes, text = "Buffer solution used: ")
buffSolEntry = Entry(testNotes)
fluorophoreLabel = Label(testNotes, text = "Fluorophore used: ")
fluorophoreEntry = Entry(testNotes)
fluorophoreConLabel = Label(testNotes, text = "Fluorophore concentration: ")
fluorophoreConEntry = Entry(testNotes)

deviceNumLabel.pack(in_ = inputBoxFrame, side = LEFT)
deviceNumEntry.pack(in_ = inputBoxFrame, side = LEFT)
deviceNumUsedLabel.pack(in_ = inputBoxFrame, side = LEFT)
deviceNumUsedEntry.pack(in_ = inputBoxFrame, side = LEFT)
laserPosLabel.pack(in_ = inputBoxFrame, side = LEFT)
laserPosEntry.pack(in_ = inputBoxFrame, side = LEFT)
analogGainLabel.pack(in_ = inputBoxFrame, side = LEFT)
analogGainEntry.pack(in_ = inputBoxFrame, side = LEFT)
laserVoltLabel.pack(in_ = inputBoxFrame, side = LEFT)
laserVoltEntry.pack(in_ = inputBoxFrame, side = LEFT)
pmtVoltLabel.pack(in_ = inputBoxFrame, side = LEFT)
pmtVoltEntry.pack(in_ = inputBoxFrame, side = LEFT)
hvSettingsLabel.pack(in_ = inputBoxFrame, side = LEFT)
hvSettingsEntry.pack(in_ = inputBoxFrame, side = LEFT)
buffSolLabel.pack(in_ = inputBoxFrame, side = LEFT)
buffSolEntry.pack(in_ = inputBoxFrame, side = LEFT)
fluorophoreLabel.pack(in_ = inputBoxFrame, side = LEFT)
fluorophoreEntry.pack(in_ = inputBoxFrame, side = LEFT)
fluorophoreConLabel.pack(in_ = inputBoxFrame, side = LEFT)
fluorophoreConEntry.pack(in_ = inputBoxFrame, side = LEFT)

## Checkboxes are used to add some tags to the data
checks = Frame(testNotes)
checks.pack()

## Variables showing if the boxes are checked (1) or not (0)
broken = IntVar()
success = IntVar()
wrong = IntVar()

## Create the checkboxes
brokenCheck = Checkbutton(testNotes, text="Equipment failure", variable=broken, onvalue = 1, offvalue = 0)
successCheck = Checkbutton(testNotes, text="Successful experiment", variable=success, onvalue = 1, offvalue = 0)
wrongCheck = Checkbutton(testNotes, text="I just don't know what went wrong", variable=wrong, onvalue = 1, offvalue = 0)
brokenCheck.pack(in_ = checks, side = LEFT, padx = 10)
successCheck.pack(in_ = checks, side = LEFT, padx = 10)
wrongCheck.pack(in_ = checks, side = LEFT, padx = 10)

## Dictionary for keeping track of tags
with open('RecordedData\\tags.json', 'r') as fp:
    checkDict = json.load(fp)

## This function runs when the submit button is pressed
def submit():
    ## save the text in the box
    text = textBox.get("1.0",'end-1c').strip()
    deviceNum = deviceNumEntry.get()
    #deviceNum = filter(str.isdigit, deviceNumEntry.get()).rjust(3, '0')    # Uncomment this line if you don't trust the user to give a number, and change the if statement below. WARNING: Works in Python 2.7, but not Python 3
    ## If the user didn't write anything, add some text noting that
    if text == "":
        text = "[No notes were included for this test.]"
    ## create new dictionary entry
    checkDict[fileName] = {"deviceNum": deviceNum, "deviceUsed": deviceNumEntry.get(), "laserPos": laserPosEntry.get(), "analogGain": analogGainEntry.get(),
        "laserVolt": laserVoltEntry.get(), "pmtVolt": pmtVoltEntry.get(), "hvSettings": hvSettingsEntry.get(), "buffSol": buffSolEntry.get(),
        "fluorophore": fluorophoreEntry.get(), "fluorophoreCon": fluorophoreConEntry.get(), "success": success.get(), "broken": broken.get(),
        "wrong": wrong.get(), "text": text}
    ## add other data to the front of the text for printing
    if deviceNum != "":     # If the line above was uncommented, change this to 'if deviceNum != "000":'
        text = "[Microfluidic device #" + deviceNum + "]\n" + text
    if wrong.get() == 1:
        text = "[I just don't know what went wrong]\n" + text
    if broken.get() == 1:
        text = "[Equipment failure]\n" + text
    if success.get() == 1:
        text = "[Successful experiment]\n" + text
    ## Write to the file
    fNote = open('RecordedData\\TestNotes.txt','a')
    fNote.write("\n\n***" + fileName + "***\n" + text + "\n")
    fNote.close()
    ## Write tags to .json file
    with open('RecordedData\\tags.json', 'w') as fp:
        json.dump(checkDict, fp)
    ## Close the window
    testNotes.destroy()

## The submit button
submitButton = Button(testNotes, text = "Submit", width = 10, command = submit)
submitButton.pack(pady = 10)

## This actually runs the prompt
testNotes.mainloop()
