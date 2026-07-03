from PySide6 import QtWidgets, QtCore
from .log_parser import LogParser
from .charts_plotter import ChartPlotter


class LogVisualizerWindow(QtWidgets.QDialog):

    WINDOW_NAME = "ImprovedDMVisualizer"

    def __init__(self):
        super(LogVisualizerWindow, self).__init__()

        self.setWindowTitle("Improved Delta Mush Visualizer")
        self.setMinimumSize(1000, 700)

        self.parser = LogParser()
        self.frames = []

        self.create_widgets()
        self.create_layout()
        self.create_connections()

        self.chart = ChartPlotter(self.graphLayout)  


    # UI
    def create_widgets(self):

        # File
        self.fileLineEdit = QtWidgets.QLineEdit()
        self.browseButton = QtWidgets.QPushButton("Browse")
        self.loadButton = QtWidgets.QPushButton("Load Log")

        # Frame
        self.frameSpinBox = QtWidgets.QSpinBox()
        self.frameSpinBox.setMinimum(0)

        # Metric (IMPORTANT: map clean keys)
        self.metricCombo = QtWidgets.QComboBox()
        self.metricCombo.addItems([
            "alpha",
            "collision_time",
            "ccd_time",
            "vertices",
            "faces"
        ])

        # Chart type (NEW)
        self.chartTypeCombo = QtWidgets.QComboBox()
        self.chartTypeCombo.addItems(["line", "bar"])

        # Summary
        self.summaryText = QtWidgets.QTextEdit()
        self.summaryText.setReadOnly(True)

        # Graph container (FIXED: no QLabel anymore)
        self.graphFrame = QtWidgets.QFrame()
        self.graphFrame.setFrameShape(QtWidgets.QFrame.Box)
        self.graphLayout = QtWidgets.QVBoxLayout(self.graphFrame)

        # Refresh
        self.refreshButton = QtWidgets.QPushButton("Refresh Graph")

   
    # Layout
    def create_layout(self):

        mainLayout = QtWidgets.QVBoxLayout(self)

        fileLayout = QtWidgets.QHBoxLayout()
        fileLayout.addWidget(QtWidgets.QLabel("Log File"))
        fileLayout.addWidget(self.fileLineEdit)
        fileLayout.addWidget(self.browseButton)
        fileLayout.addWidget(self.loadButton)

        optionsLayout = QtWidgets.QHBoxLayout()
        optionsLayout.addWidget(QtWidgets.QLabel("Frame"))
        optionsLayout.addWidget(self.frameSpinBox)

        optionsLayout.addSpacing(10)

        optionsLayout.addWidget(QtWidgets.QLabel("Metric"))
        optionsLayout.addWidget(self.metricCombo)

        optionsLayout.addSpacing(10)

        optionsLayout.addWidget(QtWidgets.QLabel("Type"))
        optionsLayout.addWidget(self.chartTypeCombo)

        optionsLayout.addStretch()

        splitter = QtWidgets.QSplitter()
        splitter.setOrientation(QtCore.Qt.Horizontal)

        splitter.addWidget(self.summaryText)
        splitter.addWidget(self.graphFrame)

        splitter.setSizes([250, 700])

        mainLayout.addLayout(fileLayout)
        mainLayout.addLayout(optionsLayout)
        mainLayout.addWidget(splitter)
        mainLayout.addWidget(self.refreshButton)


    # Connections
    def create_connections(self):

        self.browseButton.clicked.connect(self.browse_file)
        self.loadButton.clicked.connect(self.load_log)
        self.refreshButton.clicked.connect(self.refresh_graph)


    # File load
    def browse_file(self):

        filename, _ = QtWidgets.QFileDialog.getOpenFileName(
            self,
            "Select Log File",
            "",
            "Text Files (*.txt)"
        )

        if filename:
            self.fileLineEdit.setText(filename)

    def load_log(self):

        filename = self.fileLineEdit.text()

        if not filename:
            return

        self.frames = self.parser.parse(filename)

        if not self.frames:
            return

        self.frameSpinBox.setMaximum(len(self.frames) - 1)

        self.summaryText.setPlainText(
            f"Loaded {len(self.frames)} frames successfully."
        )

        # auto draw first frame
        self.refresh_graph()


    # GRAPH CORE 
    def refresh_graph(self):

        if not self.frames:
            return

        frame_index = self.frameSpinBox.value()
        metric = self.metricCombo.currentText()
        chart_type = self.chartTypeCombo.currentText()

        frame = self.frames[frame_index]


        # Iteration-based metrics
        if metric in ["alpha", "collision_time", "ccd_time", "vertices", "faces"]:

            self.chart.plot_iteration_data(
                frame,
                key=metric,
                chart_type=chart_type
            )

        else:
            print("Unknown metric:", metric)

    # Maya singleton safe open
    @classmethod
    def show_window(cls):

        try:
            if hasattr(cls, "_instance") and cls._instance:
                cls._instance.close()
                cls._instance.deleteLater()
        except:
            pass

        cls._instance = cls()
        cls._instance.setObjectName(cls.WINDOW_NAME)
        cls._instance.show()

        return cls._instance
    
