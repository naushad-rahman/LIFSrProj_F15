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
from collections import deque
import pprint                       #For pretty debug printing
import binascii
import struct
import ufluidics_dsp as udsp        #For DSP processing

## Always start by initializing Qt (only once per application)
app = QtGui.QApplication([])

## Define a top-level widget to hold everything (a window)
w = QtGui.QWidget()
w.resize(1000, 600)
w.setWindowTitle('Voltage Plots')

startBtnClicked = False
calBtnClicked = False

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
        time_data_thread = timeDataThread()
        time_data_thread.daemon = True
        time_data_thread.start()
        pmt_data_thread = pmtDataThread()
        pmt_data_thread.daemon = True
        pmt_data_thread.start()
        data_write_thread = dataWriteThread()
        data_write_thread.daemon = True
        data_write_thread.start()

    elif (startBtnClicked == True):
        startBtnClicked = False
        startBtn.setText('Start')

## Below at the end of the update function we check the value of quitBtnClicked
def quitButtonClicked():
    ## Close the file and close the window.
    if (startBtnClicked == False):  ##don't quit while still running.
        csvFile.close()
        w.close()

        #showNow = True
        data = np.loadtxt(open(folderName + fileTime + '.csv' ,"rb"),delimiter=",",skiprows=2)
        numSamples2 = data.shape[0]
        if(len(data) > 0):
            pmtCurve2.setData(data[:,1])

def quitButtonClicked2():
    w2.close()

## Buttons to control the High Voltage
def HVoffButtonClicked():
    teensySerialData.write('0')
    logFile.write('High Voltage Off: ' + str(timeElapsed) + '\n')
    print("HV Off")

def HVonButtonClicked():
    teensySerialData.write('1')
    logFile.write('High Voltage On: ' + str(timeElapsed) + '\n')
    print("HV On")

def insertionButtonClicked():
    teensySerialData.write('3')
    logFile.write('Sample Insertion: ' + str(timeElapsed) + '\n')
    print("Insertion")

def separationButtonClicked():
    teensySerialData.write('2')
    logFile.write('Sample Separation: ' + str(timeElapsed) + '\n')
    print("Separation")

## Button for Calibration
def calibrateButtonClicked():
    global calBtnClicked
    logFile.write('Signal Calibration: ' + str(timeElapsed) + '\n')
    calBtnClicked = True

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

calBtn = QtGui.QPushButton("Calibrate")
calBtn.setToolTip('Clock to callibrate signal sample')

## Functions in parantheses are to be called when buttons are clicked
startBtn.clicked.connect(startButtonClicked)
quitBtn.clicked.connect(quitButtonClicked)
quitBtn2.clicked.connect(quitButtonClicked2)
HVonBtn.clicked.connect(HVonButtonClicked)
HVoffBtn.clicked.connect(HVoffButtonClicked)
insBtn.clicked.connect(insertionButtonClicked)
sepBtn.clicked.connect(separationButtonClicked)
calBtn.clicked.connect(calibrateButtonClicked)

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

# Create and initialize plot widget for processed signals
PROCPLOT_TITLE = "Filtered Processed PMT Data"
procplot_widget = pg.PlotWidget()
procplot_widget.setYRange(0, 1)
procplot_widget.setXRange(0, xSamples)
procplot_widget.setLabel('top', text=PROCPLOT_TITLE)
#procplot_curve = procplot_widget.plot() # Check with this line it may be done later in code

## Create a grid layout to manage the widgets size and position
## The grid layout allows us to place a widget in a given column and row
layout = QtGui.QGridLayout()
w.setLayout(layout)

## Add widgets to the layout in their proper positions
## The first number in parantheses is the row, the second is the column
layout.addWidget(quitBtn, 0, 0)
layout.addWidget(startBtn, 3, 0)
layout.addWidget(HVonBtn, 0, 2)
layout.addWidget(insBtn, 3, 2)
layout.addWidget(sepBtn, 4, 2)
layout.addWidget(HVoffBtn, 5, 2)
layout.addWidget(calBtn, 6, 2)

layout.addWidget(pmtPlotWidget, 1, 1)
layout.addWidget(procplot_widget, 2, 1)

## Display the widget as a new window
w.show()

## Initialize all global variables

## Whenever we plot a range of samples, xLeftIndex is the x value on the
## PlotWidget where we start plotting the samples, xRightIndex is where we stop
## These values will reset when they reach the value of xSamples
xRightIndex = 0
xLeftIndex = 0

## Used to determine how often we plot a range of values
graphCount = 0

## Time values in microseconds read from the teensy are stored in these variables
## Before timeElapsed is updated, we store its old value in timeElapsedPrev
timeElapsed = 0L
timeElapsedPrev = 0L

## Determines if we are running through the update loop for the first time
firstRun = True

## Create new file, with the name being today's date and current time and write headings to file in CSV format
fileTime = strftime("%Y%m%d_%H%M%S")
folderName = 'RecordedData\\'

## File is saved to Documents/IPython Notebooks/RecordedData
i = datetime.now()
timeString = str(i.year) + "-" + str(i.month) + "-" + str(i.day) + " at " + \
	str(i.hour) + ":" + str(i.minute) + ":" + str(i.second)
csvFile = open(folderName + fileTime + '.csv', 'a')
csvFile.write("# Data from " + timeString + '\n')
csvFile.write("Timestamp,PMT\n")

## File for logging
logFile = open(folderName + fileTime + '.log', 'a')
logFile.write('# Log from ' + timeString + '\n\n')
logFile.write('# DSP Processing Report:\n')
logFile.write(udsp.PROCESSING_TYPE)
logFile.write('\n# Event: timestamp (from teensy)\n')

## Initialize the container for our voltage values read in from the teensy
## IMPORTANT NOTE: The com port value needs to be updated if the com value
## changes. It's the same number that appears on the bottom right corner of the
## window containing the TeensyDataWrite.ino code

teensySerialData = serial.Serial("COM6", 115200)

#change this to match the value in the Teensy's "timer0.begin(SampleVoltage, 110);" line
usecBetweenPackets = 110.0

packetsRecieved = 0L

# Data structures for data from Teensy
recieved_data = deque()
time_data = deque()
pmt_data = deque()
time_write = deque()
pmt_write = deque()
graph_sema = threading.Semaphore()

filter_block = []
time_block = []
pmt_filtered = []
pmt_thresholded = []
pmt_calibrate = []

pmt_baseline = 0.0
pmt_threshold = 100.0 #default threshold value
startTime = 0L;
endTime = 0L;

sample_detected = False
high_start = -1

#ignore this. It's just for testing
def serialThreadRun(times):
    ## Set global precedence to previously defined values
    global firstRun
    global startBtnClicked
    global recieved_data
    inputBytes = []
    while (times > 0):
        inputBytes = teensySerialData.read(size = 6)
        if (firstRun == True):
            ## Only run once to ensure buffer is completely flushed
            firstRun = False
            teensySerialData.flushInput()
            continue
        #Bytes read in and stored in a char array of size six
        recieved_data.append(inputBytes)
        times -= 1

# import cProfile
# import re
# cProfile.run('serialThreadRun("100")')
        
class serialReadThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        ## Set global precedence to previously defined values
        global firstRun
        global startBtnClicked
        global time_data
        global pmt_data
        #inputBytes = []
        while (startBtnClicked):
            #Reads the values from the serial port into time_value, adc_value, and trash.
            #In the ">LHH" argument, the ">" tells it the data is big-endian, the "L" tells it
            #that time_value is an unsigned long, and the "H"s tell it that adc_value and trash
            #are unsigned shorts. Trash is only there because things break if 6 bytes are sent at
            #a time. It is then ignored.
            time_value, adc_value, trash = struct.unpack(">LHH", teensySerialData.read(8))
            if (firstRun == True):
                ## Only run once to ensure buffer is completely flushed
                firstRun = False
                teensySerialData.flushInput()
                continue
            #Bytes read in and stored in a char array of size six
            time_data.append(time_value) #Append time value to time_data deque
            time_block.append(time_value) #Extend time value to time_block to report detections
            pmt_write.append(str(adc_value)) #Append string adc value to pmt_write deque
            pmt_data.append(adc_value) #Append adc value to pmt_data deque
            ##print out the recieved data in hex format for testing
            # out = ""
            # for d in struct.unpack(">LHH", teensySerialData.read(8)):
                # out += ('%08X' % d) + " "
            # print(out)

class timeDataThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global timeElapsed
        global timeElapsedPrev
        global startBtnClicked
        global startTime
        global time_data
        while (startBtnClicked):
            while(not time_data):
                pass
            timeElapsedPrev = timeElapsed
            timeElapsed = time_data.popleft()
            if (timeElapsedPrev == 0):
                startTime = timeElapsed
                timeElapsedPrev = timeElapsed

            # We'll add all our values to this string until we're ready to exit the loop, at which point it will be written to a file
            time_write.append(str(timeElapsed))

            ## This difference calucalted in the if statement is the amount of time in microseconds since the last value
            ## we read in and wrote to a file. If this value is significantly greater than 100, we know we have missed some
            ## values, probably as a result of the buffer filling up and scrapping old values to make room for new values.
            ## The number we print out will be the approximate number of values we failed to read in.
            ## This is useful to determine if your code is running too slow
            if (timeElapsed - timeElapsedPrev > (usecBetweenPackets*1.5)):
                print("missed time: " + str((timeElapsed-timeElapsedPrev)/usecBetweenPackets))
                logFile.write('Missed Sample: ' + str(timeElapsed) + '\n')

class pmtDataThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global startBtnClicked
        global calBtnClicked
        global pmt_data
        global filter_block
        global time_block
        global pmt_filtered
        global pmt_calibrate
        global pmt_baseline
        global pmt_threshold
        global sample_detected
        global high_start
        while (startBtnClicked):
            while(not pmt_data):
                pass
            
            filter_block.append(pmt_data.popleft())
            
            if len(filter_block) >= udsp.FILTBLK_SIZE:
                graph_sema.acquire()
                filter_block = udsp.filter_signal(filter_block)
                if calBtnClicked:
                    pmt_calibrate.extend(filter_block)
                filter_block[:] = [x - pmt_baseline for x in filter_block]
                pmt_filtered.extend(filter_block)
                filter_block[:] = [x > pmt_threshold for x in filter_block]
                filter_block = udsp.filter_threshold(filter_block)
                pmt_thresholded.extend(filter_block)
                detections, high_start = udsp.detect_sample(filter_block, time_block, high_start)
                filter_block = []
                time_block = []
                graph_sema.release()
                
                while detections:
                	peak, width = detections.pop()
                	logFile.write("Sample Detected: " + str(peak))
                	print("Sample Detected: " + str(peak))
                	logFile.write("Sample Width: " + str(width))
                	print("Sample Width: " + str(width))
            
            if calBtnClicked and pmt_calibrate:
                if len(pmt_calibrate) >= udsp.CALIBRATION_WIDTH:
                    pmt_baseline, pmt_threshold = udsp.calibrate(pmt_calibrate)
                    pmt_calibrate = []
                    calBtnClicked = False
                    print "PMT Baseline:", pmt_baseline, "PMT Threshold:", pmt_threshold

class dataWriteThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        global startBtnClicked
        global packetsRecieved

        while (startBtnClicked):
            while(not time_write or not pmt_write):
                pass
            localTimeElapsed = time_write.popleft()
            pmtNumDataRounded = pmt_write.popleft()
            
            stringToWrite = localTimeElapsed + "," + pmtNumDataRounded + '\n'
            csvFile.write(stringToWrite)
            packetsRecieved += 1

def update():
    global xLeftIndex
    global xRightIndex
    global pmt_filtered
    global pmt_thresholded
    global xSamples

    if (len(pmt_filtered) > 0): #We will plot new values once we have this many values to plot
        if (xLeftIndex == 0):
            ## Remove all PlotDataItems from the PlotWidgets. This will effectively reset the graphs (approximately every 30000 samples)
            pmtPlotWidget.clear()
            procplot_widget.clear()
                       
        ## pmtCurve are of the PlotDataItem type and are added to the PlotWidget.
        ## Documentation for these types can be found on pyqtgraph's website
        
        graph_sema.acquire()
        
        xRightIndex += len(pmt_filtered)

        pmtCurve = pmtPlotWidget.plot()
        procplot_curve = procplot_widget.plot()
        xRange = range(xLeftIndex, xRightIndex)
        pmtCurve.setData(xRange, pmt_filtered)
        procplot_curve.setData(xRange, pmt_thresholded)

        ## Now that we've plotting the values, we no longer need these arrays to store them
        pmt_filtered = []
        pmt_thresholded = []
        
        xLeftIndex = xRightIndex
        graphCount = 0
        if(xRightIndex >= xSamples):
            xRightIndex = 0
            xLeftIndex = 0
        
        graph_sema.release()

## Run update function in response to a timer
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)

## Start the Qt event loop
app.exec_()

#check how many packets were missed
endTime = timeElapsedPrev
packetsSent = (endTime - startTime) / usecBetweenPackets
packetsMissed = packetsSent - packetsRecieved
if packetsSent > 0:
    percentageMissed = (float(packetsMissed) / float(packetsSent)) * 100.0
else:
    percentageMissed = 0
print("start: " + str(startTime) + "  endTime: " + str(endTime) + "  received: " + str(packetsRecieved) + '\n')
print("You missed " + str(packetsMissed) + " out of " + str(packetsSent) + " packets sent. (" + str(percentageMissed) + "%)")

logFile.write('Start Time: ' + str(startTime) + '\n')
logFile.write('End Time: ' + str(endTime) + '\n')
logFile.write('Packets Received: ' + str(packetsRecieved) + '\n')
logFile.write('Packets Sent: ' + str(packetsSent) + '\n')
logFile.write('Packets Missed: ' + str(endTime) + ' (' + str(percentageMissed) + '%)' + '\n')
logFile.close()

# Prompts the user to add a description for the test data and saves it in RecordedData\TestNotes.txt
## Tkinter is used to create this window
testNotes = Tk()

## The label telling the user what to do
l = Label(testNotes, text = "Describe your experiment and results here.")
l.pack(padx = 10, pady = 5, anchor = W)

## The textbox where the notes are written
textBox = Text(testNotes, height = 10, width = 78)
textBox.pack(padx = 10, pady = 5, anchor = W)
textBox.focus_set()

## textbox for microfluidic device number
inputBoxFrame = Frame(testNotes)
inputBoxFrame.pack(padx = 10, anchor = W)
inputBoxLeftFrame = Frame(testNotes)
inputBoxLeftFrame.pack(in_ = inputBoxFrame, padx = 10, side = LEFT, anchor = W)
inputBoxRightFrame = Frame(testNotes)
inputBoxRightFrame.pack(in_ = inputBoxFrame, padx = 10, side = LEFT, anchor = W)
inputBoxFrameL0 = Frame(testNotes, pady = 2)
inputBoxFrameL0.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR0 = Frame(testNotes, pady = 2)
inputBoxFrameR0.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)
inputBoxFrameL1 = Frame(testNotes, pady = 2)
inputBoxFrameL1.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR1 = Frame(testNotes, pady = 2)
inputBoxFrameR1.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)
inputBoxFrameL2 = Frame(testNotes, pady = 2)
inputBoxFrameL2.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR2 = Frame(testNotes, pady = 2)
inputBoxFrameR2.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)
inputBoxFrameL3 = Frame(testNotes, pady = 2)
inputBoxFrameL3.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR3 = Frame(testNotes, pady = 2)
inputBoxFrameR3.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)
inputBoxFrameL4 = Frame(testNotes, pady = 2)
inputBoxFrameL4.pack(in_ = inputBoxLeftFrame, padx = 10, anchor = W)
inputBoxFrameR4 = Frame(testNotes, pady = 2)
inputBoxFrameR4.pack(in_ = inputBoxRightFrame, padx = 10, anchor = W)

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

deviceNumLabel.pack(in_ = inputBoxFrameL0, side = LEFT)
deviceNumEntry.pack(in_ = inputBoxFrameL0, side = LEFT)
deviceNumUsedLabel.pack(in_ = inputBoxFrameR0, side = LEFT)
deviceNumUsedEntry.pack(in_ = inputBoxFrameR0, side = LEFT)
laserPosLabel.pack(in_ = inputBoxFrameL1, side = LEFT)
laserPosEntry.pack(in_ = inputBoxFrameL1, side = LEFT)
analogGainLabel.pack(in_ = inputBoxFrameR1, side = LEFT)
analogGainEntry.pack(in_ = inputBoxFrameR1, side = LEFT)
laserVoltLabel.pack(in_ = inputBoxFrameL2, side = LEFT)
laserVoltEntry.pack(in_ = inputBoxFrameL2, side = LEFT)
pmtVoltLabel.pack(in_ = inputBoxFrameR2, side = LEFT)
pmtVoltEntry.pack(in_ = inputBoxFrameR2, side = LEFT)
hvSettingsLabel.pack(in_ = inputBoxFrameL3, side = LEFT)
hvSettingsEntry.pack(in_ = inputBoxFrameL3, side = LEFT)
buffSolLabel.pack(in_ = inputBoxFrameR3, side = LEFT)
buffSolEntry.pack(in_ = inputBoxFrameR3, side = LEFT)
fluorophoreLabel.pack(in_ = inputBoxFrameL4, side = LEFT)
fluorophoreEntry.pack(in_ = inputBoxFrameL4, side = LEFT)
fluorophoreConLabel.pack(in_ = inputBoxFrameR4, side = LEFT)
fluorophoreConEntry.pack(in_ = inputBoxFrameR4, side = LEFT)

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
with open(folderName + 'tags.json', 'r') as fp:
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
    checkDict[fileTime] = {"deviceNum": deviceNum, "deviceUsed": deviceNumUsedEntry.get(), "laserPos": laserPosEntry.get(), "analogGain": analogGainEntry.get(),
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
    fNote = open(folderName + 'TestNotes.txt','a')
    fNote.write("\n\n***" + fileTime + "***\n" + text + "\n")
    fNote.close()
    ## Write tags to .json file
    with open(folderName + 'tags.json', 'w') as fp:
        json.dump(checkDict, fp)
    ## Close the window
    testNotes.destroy()

## The submit button
submitButton = Button(testNotes, text = "Submit", width = 10, command = submit)
submitButton.pack(pady = 10)

## This actually runs the prompt
testNotes.mainloop()
