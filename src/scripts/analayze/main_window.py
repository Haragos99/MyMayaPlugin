from PySide6 import QtWidgets, QtCore
from .log_parser import LogParser
from .charts_plotter import ChartPlotter
from .statistics import StatisticsGenerator
from datetime import datetime
import os
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

        self.exportButton = QtWidgets.QPushButton("Export All Data")
        self.exportButton.setToolTip(
        "Export all statistics and all generated plot images to a selected folder"
        )



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

        # Buttons
        buttonLayout = QtWidgets.QHBoxLayout()
        buttonLayout.addWidget(self.refreshButton)
        buttonLayout.addWidget(self.exportButton)

        mainLayout.addLayout(fileLayout)
        mainLayout.addLayout(optionsLayout)
        mainLayout.addWidget(splitter)
        mainLayout.addLayout(buttonLayout)

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
        self.exportButton.clicked.connect(self.export_all_data)



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


    def export_all_data(self):

        if not self.frames:
            QtWidgets.QMessageBox.warning(
                self,
                "Export",
                "There is no loaded log data to export."
            )
            return

        # Ask user for destination folder
        export_dir = QtWidgets.QFileDialog.getExistingDirectory(
            self,
            "Choose Export Folder",
            ""
        )

        if not export_dir:
            return

        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        export_folder = os.path.join(
            export_dir,
            f"DeltaMush_Export_{timestamp}"
        )

        os.makedirs(export_folder, exist_ok=True)

        plots_folder = os.path.join(export_folder, "plots")
        os.makedirs(plots_folder, exist_ok=True)

        try:
            # ---------------------------------------------------------
            # 1. Export complete raw statistics
            # ---------------------------------------------------------

            statistics_file = os.path.join(
                export_folder,
                "statistics.txt"
            )

            with open(statistics_file, "w", encoding="utf-8") as file:

                file.write("IMPROVED DELTA MUSH - COMPLETE DATA EXPORT\n")
                file.write("=" * 70 + "\n\n")

                file.write(f"Frames: {len(self.frames)}\n\n")

                # Overall statistics
                file.write("OVERALL SUMMARY\n")
                file.write("-" * 70 + "\n")
                file.write(
                    StatisticsGenerator.overall_summary(self.frames)
                )
                file.write("\n\n")

                # Every frame
                for frame_index, frame in enumerate(self.frames):

                    file.write("\n")
                    file.write("=" * 70 + "\n")
                    file.write(f"FRAME {frame_index}\n")
                    file.write("=" * 70 + "\n\n")

                    file.write(
                        StatisticsGenerator.frame_summary(frame)
                    )
                    file.write("\n")

                    file.write("FRAME DATA\n")
                    file.write("-" * 70 + "\n")

                    file.write(f"Frame: {frame.frame}\n")
                    file.write(f"Smooth Time: {frame.smooth_time}\n")
                    file.write(f"Filter Time: {frame.filter_time}\n")
                    file.write(f"Filtered Faces: {frame.filtered_faces}\n")
                    file.write(
                        f"Intersected Objects: "
                        f"{frame.intersected_objects}\n"
                    )
                    file.write(
                        f"Final Smooth Time: "
                        f"{frame.final_smooth_time}\n"
                    )
                    file.write(
                        f"Improved Delta Mush Time: "
                        f"{getattr(frame, 'improved_dm_time', 0)}\n"
                    )
                    file.write(
                        f"Total Execution Time: "
                        f"{frame.total_execution_time}\n"
                    )
                    file.write(
                        f"Total Iterations: "
                        f"{frame.total_iterations}\n"
                    )
                    file.write(
                        f"Final Vertices: "
                        f"{frame.final_vertices}\n"
                    )
                    file.write(
                        f"Final Faces: "
                        f"{frame.final_faces}\n"
                    )

                    # Every iteration
                    file.write("\nITERATIONS\n")
                    file.write("-" * 70 + "\n")

                    for iteration in frame.iterations:

                        file.write(
                            f"Iteration: {iteration.iteration}\n"
                        )
                        file.write(
                            f"Collision: "
                            f"{iteration.collision}\n"
                        )
                        file.write(
                            f"Collision Time: "
                            f"{iteration.collision_time}\n"
                        )
                        file.write(
                            f"CCD Time: "
                            f"{iteration.ccd_time}\n"
                        )
                        file.write(
                            f"Alpha: {iteration.alpha}\n"
                        )
                        file.write(
                            f"Vertices: {iteration.vertices}\n"
                        )
                        file.write(
                            f"Faces: {iteration.faces}\n"
                        )
                        file.write("\n")

            # ---------------------------------------------------------
            # 2. Export every iteration metric as plots
            # ---------------------------------------------------------

            iteration_metrics = [
                "alpha",
                "collision_time",
                "ccd_time",
                "vertices",
                "faces"
            ]

            chart_types = [
                "line",
                "bar"
            ]

            for frame_index, frame in enumerate(self.frames):

                for metric in iteration_metrics:

                    for chart_type in chart_types:

                        # Draw the requested chart
                        self.chart.plot_iteration_data(
                            frame,
                            metric,
                            chart_type
                        )

                        filename = (
                            f"frame_{frame_index:04d}_"
                            f"iteration_{metric}_{chart_type}.png"
                        )

                        filepath = os.path.join(
                            plots_folder,
                            filename
                        )

                        self.chart.figure.savefig(
                            filepath,
                            dpi=150,
                            bbox_inches="tight"
                        )

            # ---------------------------------------------------------
            # 3. Export every frame metric as plots
            # ---------------------------------------------------------

            frame_metrics = [
                "smooth_time",
                "filter_time",
                "total_time",
                "iterations",
                "filtered_faces"
            ]

            for metric in frame_metrics:

                for chart_type in chart_types:

                    self.chart.plot_frame_data(
                        self.frames,
                        metric,
                        chart_type
                    )

                    filename = (
                        f"all_frames_{metric}_{chart_type}.png"
                    )

                    filepath = os.path.join(
                        plots_folder,
                        filename
                    )

                    self.chart.figure.savefig(
                        filepath,
                        dpi=150,
                        bbox_inches="tight"
                    )

            # ---------------------------------------------------------
            # 4. Export the currently displayed summary
            # ---------------------------------------------------------

            summary_file = os.path.join(
                export_folder,
                "current_summary.txt"
            )

            with open(
                summary_file,
                "w",
                encoding="utf-8"
            ) as file:

                file.write(
                    self.summaryText.toPlainText()
                )

            # ---------------------------------------------------------
            # 5. Export information about the current visualization
            # ---------------------------------------------------------

            settings_file = os.path.join(
                export_folder,
                "export_settings.txt"
            )

            with open(
                settings_file,
                "w",
                encoding="utf-8"
            ) as file:

                file.write("VISUALIZATION SETTINGS\n")
                file.write("=" * 50 + "\n")
                file.write(
                    f"Plot Mode: "
                    f"{self.plotTypeCombo.currentText()}\n"
                )
                file.write(
                    f"Iteration Metric: "
                    f"{self.iterationMetricCombo.currentText()}\n"
                )
                file.write(
                    f"Frame Metric: "
                    f"{self.frameMetricCombo.currentText()}\n"
                )
                file.write(
                    f"Chart Type: "
                    f"{self.chartTypeCombo.currentText()}\n"
                )
                file.write(
                    f"Selected Frame: "
                    f"{self.frameSpinBox.value()}\n"
                )
                file.write(
                    f"Total Frames: "
                    f"{len(self.frames)}\n"
                )

            # ---------------------------------------------------------
            # Finished
            # ---------------------------------------------------------

            QtWidgets.QMessageBox.information(
                self,
                "Export Complete",
                "All data and plot images were exported successfully.\n\n"
                f"Location:\n{export_folder}"
            )

        except Exception as error:

            QtWidgets.QMessageBox.critical(
                self,
                "Export Error",
                f"Failed to export data:\n\n{error}"
            )


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
    
