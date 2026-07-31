import re


class IterationData:
    def __init__(self):
        self.iteration = 0
        self.collision = False
        self.collision_time = 0
        self.ccd_time = 0
        self.alpha = 0.0
        self.vertices = 0
        self.faces = 0


class FrameData:
    def __init__(self):
        self.frame = 0

        self.smooth_time = 0
        self.filter_time = 0
        self.filtered_faces = 0
        self.intersected_objects = 0

        self.final_smooth_time = 0
        self.total_execution_time = 0
        self.total_iterations = 0
        self.final_vertices = 0
        self.final_faces = 0

        self.iterations = []



class LogParser:

    def __init__(self):
        self.frames = []

    def parse(self, filename) -> list[FrameData]:

        self.frames.clear()

        current_frame = None
        current_iteration = None
        frame_count = 0
        with open(filename, "r") as file:

            for line in file:

                line = line.strip()

                if not line:
                    continue

                # Skip separators
                if line.startswith("=") or line.startswith("-"):
                    continue

                # ----------------------------------------------------------
                # Frame
                # ----------------------------------------------------------
                frame = re.match(r"Frame:\s*(\d+)", line)
                if frame:

                    current_frame = FrameData()
                    current_frame.frame = int(frame_count)
                    frame_count += 1
                    self.frames.append(current_frame)

                    current_iteration = None
                    continue

                if current_frame is None:
                    continue

                # ----------------------------------------------------------
                # Frame statistics
                # ----------------------------------------------------------

                smooth = re.match(
                    r"Smooth Mesh Time \(ms\):\s*([\d.]+)", line)
                if smooth:
                    current_frame.smooth_time = float(smooth.group(1))
                    continue

                filter_time = re.match(
                    r"Intersection Filter Time \(ms\):\s*([\d.]+)", line)
                if filter_time:
                    current_frame.filter_time = float(filter_time.group(1))
                    continue

                filtered_faces = re.match(
                    r"Filtered Faces:\s*(\d+)", line)
                if filtered_faces:
                    current_frame.filtered_faces = int(filtered_faces.group(1))
                    continue

                intersected = re.match(
                    r"Intersected Objects:\s*(\d+)", line)
                if intersected:
                    current_frame.intersected_objects = int(intersected.group(1))
                    continue

                # ----------------------------------------------------------
                # Iteration
                # ----------------------------------------------------------

                iteration = re.match(
                    r"Iteration:\s*(\d+)", line)
                if iteration:

                    current_iteration = IterationData()
                    current_iteration.iteration = int(iteration.group(1))

                    current_frame.iterations.append(current_iteration)
                    continue

                # ----------------------------------------------------------
                # Iteration statistics
                # ----------------------------------------------------------

                if current_iteration is not None:

                    collision_time = re.match(
                        r"Collision Detection Time \(ms\):\s*([\d.]+)", line)
                    if collision_time:
                        current_iteration.collision_time = float(
                            collision_time.group(1))
                        continue

                    collision = re.match(
                        r"Collision Detected:\s*(YES|NO)", line)
                    if collision:
                        current_iteration.collision = (
                            collision.group(1) == "YES")
                        continue

                    ccd_time = re.match(
                        r"CCD Deformation Time \(ms\):\s*([\d.]+)", line)
                    if ccd_time:
                        current_iteration.ccd_time = float(
                            ccd_time.group(1))
                        continue

                    alpha = re.match(
                        r"Alpha:\s*([\d.]+)", line)
                    if alpha:
                        current_iteration.alpha = float(alpha.group(1))
                        continue

                    vertices = re.match(
                        r"Collided Vertices:\s*(\d+)", line)
                    if vertices:
                        current_iteration.vertices = int(vertices.group(1))
                        continue

                    faces = re.match(
                        r"Collided Faces:\s*(\d+)", line)
                    if faces:
                        current_iteration.faces = int(faces.group(1))
                        continue

                # ----------------------------------------------------------
                # Summary
                # ----------------------------------------------------------

                final_smooth = re.match(
                    r"Final Smoothing Time \(ms\):\s*([\d.]+)", line)
                if final_smooth:
                    current_frame.final_smooth_time = float(
                        final_smooth.group(1))
                    continue

                improved_dm = re.match(
                    r"Improved Delta Mush Time \(ms\):\s*([\d.]+)", line)
                if improved_dm:
                    current_frame.improved_dm_time = float(
                        improved_dm.group(1))
                    continue

                total_iterations = re.match(
                    r"Iterations:\s*(\d+)", line)
                if total_iterations:
                    current_frame.total_iterations = int(
                        total_iterations.group(1))
                    continue

                final_vertices = re.match(
                    r"Final Collided Vertices:\s*(\d+)", line)
                if final_vertices:
                    current_frame.final_vertices = int(
                        final_vertices.group(1))
                    continue

                final_faces = re.match(
                    r"Final Collided Faces:\s*(\d+)", line)
                if final_faces:
                    current_frame.final_faces = int(
                        final_faces.group(1))
                    continue

                total_time = re.match(
                    r"Total Execution Time \(ms\):\s*([\d.]+)", line)
                if total_time:
                    current_frame.total_execution_time = float(
                        total_time.group(1))
                    continue

        return self.frames
    

    def get_frames(self) -> list[FrameData]:
        return self.frames


