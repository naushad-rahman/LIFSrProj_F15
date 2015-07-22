from pyqtgraph import QtGui, QtCore #Provides usage of PyQt4's libraries which aids in UI design
import pyqtgraph as pg              #Initiation of plotting code
import serial                       #Communication with the serial port is done using the pySerial 2.7 package
from datetime import datetime       #Allows us to look at current date and time
import dataprocessing               #code for plotting the data from the CSV

## Always start by initializing Qt (only once per application)
app = QtGui.QApplication([])

## Define a top-level widget to hold everything (a window)
w = QtGui.QWidget()
w.resize(1000,600)
w.setWindowTitle('Voltage Plots')

startBtnClicked = False
quitBtnClicked = False

## This function contains the behavior we want to see when the start button is clicked
def startButtonClicked():
    global startBtnClicked
    global startBtn
    if (startBtnClicked == False):
        teensySerialData.flushInput() #empty serial buffer for input from the teensy
        startBtnClicked = True
        startBtn.setText('Stop')
            
    elif (startBtnClicked == True):
        startBtnClicked = False
        startBtn.setText('Start')

## Below at the end of the update function we check the value of quitBtnClicked
def quitButtonClicked():
    global quitBtnClicked
    quitBtnClicked = True
	
	
teensySerialData = serial.Serial("COM4", 115200)
teensySerialData.write("1")
teensySerialData.write('3')
teensySerialData.write('2')
teensySerialData.write('0')

## Create widgets to be placed inside
startBtn = QtGui.QPushButton('Start')
startBtn.setToolTip('Click to begin graphing') #This message appears while hovering mouse over button

quitBtn = QtGui.QPushButton('Quit')
quitBtn.setToolTip('Click to quit program')

## Functions in parantheses are to be called when buttons are clicked
startBtn.clicked.connect(startButtonClicked)
quitBtn.clicked.connect(quitButtonClicked)

## xSamples is the maximum amount of samples we want graphed at a time
xSamples = 30000

## Create plot widget for PMT plot
## Documentation for PlotWidget can be found on the pyqtgraph website
#pmtPlotWidget = pg.PlotWidget()
#pmtPlotWidget.setYRange(0, 3.3)
#pmtPlotWidget.setXRange(0, xSamples)
#pmtPlotWidget.setLabel('top', text = "N/A") #Title to appear at top of widget

## Create plot widget for peak detector plot
pdPlotWidget = pg.PlotWidget()
pdPlotWidget.setYRange(0, 3.3)
pdPlotWidget.setXRange(0, xSamples)
pdPlotWidget.setLabel('top', text = "PMT") #Title to appear at top of widget

## Create a grid layout to manage the widgets size and position
## The grid layout allows us to place a widget in a given column and row
layout = QtGui.QGridLayout()
w.setLayout(layout)

## Add widgets to the layout in their proper positions
## The first number in parantheses is the row, the second is the column
layout.addWidget(quitBtn, 0, 0)
layout.addWidget(startBtn, 2, 0)
#layout.addWidget(pmtPlotWidget, 1, 1)
layout.addWidget(pdPlotWidget, 1, 2)

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
#pmtData = []
pdData = []

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
fileName = str(i.year) + str(i.month) + str(i.day) + "_" + str(i.hour) + str(i.minute) + str(i.second) + ".csv"

## File is saved to Documents/IPython Notebooks/RecordedData
f = open('RecordedData\\' + fileName, 'a')
f.write("#Data from " + str(i.year) + "-" + str(i.month) + "-" + str(i.day) + " at " + str(i.hour) + ":" + str(i.minute) + ":" + str(i.second) + '\n')
f.write("Timestamp,PMT,PeakDetector\n")

## Initialize the container for our voltage values read in from the teensy
## IMPORTANT NOTE: The com port value needs to be updated if the com value
## changes. It's the same number that appears on the bottom right corner of the
## window containing the TeensyDataWrite.ino code
# teensySerialData = serial.Serial("com38", 9600)

def update():
    ## Set global precedence to previously defined values
    global xSamples
    global xRightIndex
    global xLeftIndex
    #global pmtData
    global pdData
    global graphCount
    global timeElapsed
    global timeElapsedPrev
    global firstRun
    
    ## The number of bytes currently waiting to be read in.
    ## We want to read these values as soon as possible, because
    ## we will lose them if the buffer fills up
    bufferSize = teensySerialData.inWaiting()
    runCount = bufferSize/8 # since we write 8 bytes at a time, we similarly want to read them 8 at a time
    while (runCount > 0):
        if (startBtnClicked == True):
            #Bytes read in and stored in a char array of size eight
            inputBytes = teensySerialData.read(size = 8)
            #The ord function converts a char to its corresponding ASCII integer, which we can then convert to a float
            timeByte3 = float(ord(inputBytes[0]))
            timeByte2 = float(ord(inputBytes[1]))
            timeByte1 = float(ord(inputBytes[2]))
            timeByte0 = float(ord(inputBytes[3]))
            pmtByte1 = float(ord(inputBytes[4]))
            pmtByte0 = float(ord(inputBytes[5]))
            pdByte1 = float(ord(inputBytes[6]))
            pdByte0 = float(ord(inputBytes[7]))
            timeElapsedPrev = timeElapsed
            timeElapsed = timeByte3*256*256*256 + timeByte2*256*256 + timeByte1*256 + timeByte0 #There are 8 bits in a byte, 2^8 = 256
            if (firstRun == True):
                ## Only run once to ensure buffer is completely flushed
                firstRun = False
                teensySerialData.flushInput()
                break
                
            # We'll add all our values to this string until we're ready to exit the loop, at which point it will be written to a file
            stringToWrite = str(timeElapsed) + ","
            
            ## This difference calucalted in the if statement is the amount of time in microseconds since the last value 
            ## we read in and wrote to a file. If this value is significantly greater than 100, we know we have missed some 
            ## values, probably as a result of the buffer filling up and scrapping old values to make room for new values.
            ## The number we print out will be the approximate number of values we failed to read in.
            ## This is useful to determine if your code is running too slow
            if (timeElapsed - timeElapsedPrev > 150):
                print(str((timeElapsed-timeElapsedPrev)/100))
                
            numData = pmtByte1*256 + pmtByte0
            #numData = numData*3.3/1024
            #numDataRounded = numData - numData%.001 #Round voltage value to 3 decimal points
            #pmtData.append(numDataRounded)
            stringToWrite = stringToWrite + str(numDataRounded) + ","
            numData = pdByte1*256 + pdByte0
            #numData = numData*3.3/1024
            #numDataRounded = numData - numData%.001
            pdData.append(numDataRounded)
            stringToWrite = stringToWrite + str(numDataRounded) + '\n'
            f.write(stringToWrite)
            graphCount = graphCount + 1
            xRightIndex = xRightIndex + 1
        runCount = runCount - 1
        
    ## We will start plotting when the start button is clicked
    if startBtnClicked == True:
        if (graphCount >= 500): #We will plot new values once we have this many values to plot
            if (xLeftIndex == 0):
                ## Remove all PlotDataItems from the PlotWidgets. This will effectively reset the graphs (approximately every 30000 samples)
                #pmtPlotWidget.clear()
                pdPlotWidget.clear()
                
            ## pmtCurve and pdCurve are of the PlotDataItem type and are added to the PlotWidget.
            ## Documentation for these types can be found on pyqtgraph's website
            #pmtCurve = pmtPlotWidget.plot()
            pdCurve = pdPlotWidget.plot()
            xRange = range(xLeftIndex,xRightIndex)
            #pmtCurve.setData(xRange, pmtData)
            pdCurve.setData(xRange, pdData)
            
            ## Now that we've plotting the values, we no longer need these arrays to store them
            pdData = []
            #pmtData = []
            xLeftIndex = xRightIndex
            graphCount = 0
            if(xRightIndex >= xSamples):
                xRightIndex = 0
                xLeftIndex = 0
               # pmtData = []
                pdData = []
                
    if(quitBtnClicked == True):
        ## Close the file and close the window. Performing this action here ensures values we want to write to the file won't be cut off
        f.close()
        w.close()
        
## Run update function in response to a timer    
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)


## Start the Qt event loop
app.exec_()