from pyqtgraph import QtGui, QtCore
import pyqtgraph as pg
import numpy as np

input_var = raw_input("Enter the name of the CSV file: ")
input_var
data = np.loadtxt(open('RecordedData\\' + input_var + '.csv',"rb"),delimiter=",",skiprows=2)

## Always start by initializing Qt (only once per application)
app = QtGui.QApplication([])

## Define a top-level widget to hold everything
w = QtGui.QWidget()
w.resize(1000,600)
w.setWindowTitle('Voltage Plots')

numSamples = data.shape[0]

#Create Plot Widgets
pmtPlotWidget = pg.PlotWidget()
pmtPlotWidget.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
pmtPlotWidget.setYRange(0, 4096)
pmtPlotWidget.setLabel('top', text = "PMT")
pmtCurve = pmtPlotWidget.plot()

lowerBound = QtGui.QSpinBox()
lowerBound.setRange(0,numSamples)
lowerBound.setSingleStep(100)

upperBound = QtGui.QSpinBox()
upperBound.setRange(0,numSamples)
upperBound.setSingleStep(100)
upperBound.setValue(numSamples)

lowerBoundValue = 0
upperBoundValue = numSamples

## Create a grid layout to manage the widgets size and position
layout = QtGui.QGridLayout()
w.setLayout(layout)

## Add widgets to the layout in their proper positions
layout.addWidget(lowerBound, 1, 0)
layout.addWidget(upperBound, 1, 1)
layout.addWidget(pmtPlotWidget, 1, 2, 1, 1)  # wGL goes on right side, spanning 3 rows

pmtCurve.setData(data[:,1])

## Display the widget as a new window
w.show()

def update():
    pmtPlotWidget.setXRange(lowerBound.value(), upperBound.value())
    upperBound.setMinimum(lowerBound.value() + 1)
    lowerBound.setMaximum(upperBound.value() - 1)

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(50)

## Start the Qt event loop
app.exec_()