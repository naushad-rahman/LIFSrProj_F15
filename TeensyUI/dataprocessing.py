def CSV_Data_plot(current_file):

    from pyqtgraph import QtGui, QtCore
    import pyqtgraph as pg
    import numpy as np

    data = np.loadtxt(open('RecordedData\\' + current_file,"rb"),delimiter=",",skiprows=2)

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
    pmtPlotWidget.setYRange(0, 3.3)
    pmtPlotWidget.setLabel('top', text = "Plot 1")
    pmtCurve = pmtPlotWidget.plot()

    pdPlotWidget = pg.PlotWidget()
    pdPlotWidget.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
    pdPlotWidget.setYRange(0, 3.3)
    pdPlotWidget.setLabel('top', text = "Plot 2")
    pdCurve = pdPlotWidget.plot()

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
    layout.addWidget(pdPlotWidget, 1, 3, 1, 1)

    pmtCurve.setData(data[:,1])
    pdCurve.setData(data[:,2])

    ## Display the widget as a new window
    w.show()

    def update_csv():
        pmtPlotWidget.setXRange(lowerBound.value(), upperBound.value())
        pdPlotWidget.setXRange(lowerBound.value(), upperBound.value())
        upperBound.setMinimum(lowerBound.value() + 1)
        lowerBound.setMaximum(upperBound.value() - 1)

    timer = QtCore.QTimer()
    timer.timeout.connect(update_csv)
    timer.start(50)

    ## Start the Qt event loop
    app.exec_()