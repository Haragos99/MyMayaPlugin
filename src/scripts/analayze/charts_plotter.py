from matplotlib.figure import Figure
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas


class ChartPlotter:

    def __init__(self, parent_layout):
        """
        parent_layout = Qt layout where the chart should be inserted
        """

        self.figure = Figure()
        self.canvas = FigureCanvas(self.figure)

        self.ax = self.figure.add_subplot(111)

        self.parent_layout = parent_layout
        self.parent_layout.addWidget(self.canvas)

    # --------------------------------------------------
    # Clear chart
    # --------------------------------------------------

    def clear(self):
        self.ax.clear()

    # --------------------------------------------------
    # Generic plot function
    # --------------------------------------------------

    def plot(self,
             x_data,
             y_data,
             title="Chart",
             xlabel="X",
             ylabel="Y",
             chart_type="line"):
        """
        chart_type:
            "line" | "bar"
        """

        self.ax.clear()

        # -----------------------
        # Line chart
        # -----------------------
        if chart_type == "line":
            self.ax.plot(x_data, y_data, marker='o')

        # -----------------------
        # Bar chart
        # -----------------------
        elif chart_type == "bar":
            self.ax.bar(x_data, y_data)

        else:
            raise ValueError("chart_type must be 'line' or 'bar'")

        self.ax.set_title(title)
        self.ax.set_xlabel(xlabel)
        self.ax.set_ylabel(ylabel)

        self.ax.grid(True)

        self.canvas.draw()

    # --------------------------------------------------
    # Specialized helpers for your log data
    # --------------------------------------------------

    def plot_iteration_data(self, frame, key, chart_type="line"):
        """
        Convenience wrapper for iteration-based plots
        """

        x = []
        y = []

        for it in frame.iterations:
            x.append(it.iteration)

            if key == "alpha":
                y.append(it.alpha)

            elif key == "collision_time":
                y.append(it.collision_time)

            elif key == "ccd_time":
                y.append(it.ccd_time)

            elif key == "vertices":
                y.append(it.vertices)

            elif key == "faces":
                y.append(it.faces)

            else:
                raise ValueError(f"Unknown key: {key}")

        self.plot(
            x,
            y,
            title=f"Iteration - {key}",
            xlabel="Iteration",
            ylabel=key,
            chart_type=chart_type
        )

    # --------------------------------------------------
    # Frame-level plotting
    # --------------------------------------------------

    def plot_frame_data(self, frames, key, chart_type="line"):
        """
        Frame-based charts (X = frame index)
        """

        x = []
        y = []

        for f in frames:
            x.append(f.frame)

            if key == "smooth_time":
                y.append(f.smooth_time)

            elif key == "filter_time":
                y.append(f.filter_time)

            elif key == "total_time":
                y.append(f.total_execution_time)

            elif key == "iterations":
                y.append(f.total_iterations)

            elif key == "filtered_faces":
                y.append(f.filtered_faces)

            else:
                raise ValueError(f"Unknown key: {key}")

        self.plot(
            x,
            y,
            title=f"Frame - {key}",
            xlabel="Frame",
            ylabel=key,
            chart_type=chart_type
        )