from PySide6 import QtWidgets, QtCore
from .log_parser import LogParser
from .charts_plotter import ChartPlotter
from .statistics import StatisticsGenerator
import maya.cmds as cmds

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
        self.update_ui()

        self.chart = ChartPlotter(self.graphLayout)  

        


    # UI
    def create_widgets(self):

        # File
        self.fileLineEdit = QtWidgets.QLineEdit()
        self.browseButton = QtWidgets.QPushButton("Browse")

        # Frame
        self.frameSpinBox = QtWidgets.QSpinBox()
        self.frameSpinBox.setMinimum(0)

        self.plotTypeCombo = QtWidgets.QComboBox()
        self.plotTypeCombo.addItems(["Iteration", "Frame"])

        # Metric (IMPORTANT: map clean keys)
        self.iterationMetricCombo = QtWidgets.QComboBox()
        self.iterationMetricCombo.addItems([
            "alpha",
            "collision_time",
            "ccd_time",
            "vertices",
            "faces"
        ])

        self.frameMetricCombo = QtWidgets.QComboBox()

        self.frameMetricCombo.addItems([
            "smooth_time",
            "filter_time",
            "total_time",
            "iterations",
            "filtered_faces"
        ])

        # Chart type (NEW)
        self.chartTypeCombo = QtWidgets.QComboBox()
        self.chartTypeCombo.addItems(["line", "bar"])

        # Labels
        self.plotTypeLabel = QtWidgets.QLabel("Plot Mode")
        self.frameLabel = QtWidgets.QLabel("Frame")
        self.metricLabel = QtWidgets.QLabel("Metric")
        self.frameMetricLabel = QtWidgets.QLabel("Frame Metric")
        self.chartTypeLabel = QtWidgets.QLabel("Chart Type")

        # Summary
        self.summaryText = QtWidgets.QTextEdit()
        self.summaryText.setReadOnly(True)

        # Graph container 
        self.graphFrame = QtWidgets.QFrame()
        self.graphFrame.setFrameShape(QtWidgets.QFrame.Box)
        self.graphLayout = QtWidgets.QVBoxLayout(self.graphFrame)

        # Refresh
        self.refreshButton = QtWidgets.QPushButton("Refresh Graph")



    def on_frame_changed(self, value):
        import maya.cmds as cmds
        cmds.currentTime(value, e=True, u=True)

    def update_ui(self):
        iteration_mode = self.plotTypeCombo.currentText() == "Iteration"

        # Frame controls
        self.frameLabel.setVisible(iteration_mode)
        self.frameSpinBox.setVisible(iteration_mode)

        # Iteration metric controls
        self.metricLabel.setVisible(iteration_mode)
        self.iterationMetricCombo.setVisible(iteration_mode)

        # Frame metric controls
        self.frameMetricLabel.setVisible(not iteration_mode)
        self.frameMetricCombo.setVisible(not iteration_mode)

        # Refresh graph automatically
        if self.frames:
            self.refresh_graph()
    
    # Layout
    def create_layout(self):

        mainLayout = QtWidgets.QVBoxLayout(self)

        fileLayout = QtWidgets.QHBoxLayout()
        fileLayout.addWidget(QtWidgets.QLabel("Log File"))
        fileLayout.addWidget(self.fileLineEdit)
        fileLayout.addWidget(self.browseButton)

        optionsLayout = QtWidgets.QHBoxLayout()
        optionsLayout.addWidget(self.plotTypeLabel)
        optionsLayout.addWidget(self.plotTypeCombo)

        optionsLayout.addSpacing(10)

        optionsLayout.addWidget(self.metricLabel)
        optionsLayout.addWidget(self.iterationMetricCombo)

        optionsLayout.addSpacing(10)

        optionsLayout.addWidget(self.frameMetricLabel)
        optionsLayout.addWidget(self.frameMetricCombo)

        optionsLayout.addSpacing(10)

        optionsLayout.addWidget(self.chartTypeLabel)
        optionsLayout.addWidget(self.chartTypeCombo)

        optionsLayout.addSpacing(10)
        optionsLayout.addWidget(self.frameLabel)
        optionsLayout.addWidget(self.frameSpinBox)

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
        self.refreshButton.clicked.connect(self.refresh_graph)
        self.plotTypeCombo.currentIndexChanged.connect(self.update_ui)
        self.frameSpinBox.valueChanged.connect(self.refresh_graph)
        self.iterationMetricCombo.currentIndexChanged.connect(self.refresh_graph)
        self.frameMetricCombo.currentIndexChanged.connect(self.refresh_graph)
        self.chartTypeCombo.currentIndexChanged.connect(self.refresh_graph)
        self.plotTypeCombo.currentIndexChanged.connect(self.update_ui)
        self.frameSpinBox.valueChanged.connect(self.on_frame_changed)



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

        self.load_log()

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

        chart_type = self.chartTypeCombo.currentText()

        if self.plotTypeCombo.currentText() == "Iteration":

            frame_index = self.frameSpinBox.value()

            if frame_index >= len(self.frames):
                return

            frame = self.frames[frame_index]

            self.summaryText.setPlainText(
                StatisticsGenerator.frame_summary(frame)
            )

            self.chart.plot_iteration_data(
                frame,
                self.iterationMetricCombo.currentText(),
                chart_type
            )

        else:

            self.summaryText.setPlainText(
                StatisticsGenerator.overall_summary(self.frames)
            )

            self.chart.plot_frame_data(
                self.frames,
                self.frameMetricCombo.currentText(),
                chart_type
            )

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
    
