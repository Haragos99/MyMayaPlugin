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

    def parse(self, filename):

        current_frame = None
        current_iteration = None

        with open(filename, "r") as file:

            for line in file:

                line = line.strip()

                if not line:
                    continue

                # Frame
                frame = re.match(r"Frame:\s*(\d+)", line)
                if frame:

                    current_frame = FrameData()
                    current_frame.frame = int(frame.group(1))

                    self.frames.append(current_frame)
                    continue

                if current_frame is None:
                    continue

                # Frame statistics
                frame_statistics = re.match(r"Smooth time:\s*(\d+)", line)
                if frame_statistics:
                    current_frame.smooth_time = int(frame_statistics.group(1))
                    continue

                filter_tieme = re.match(r"Filter time:\s*(\d+)", line)
                if filter_tieme:
                    current_frame.filter_time = int(filter_tieme.group(1))
                    continue

                filtered_faces = re.match(r"Filtered Faces:\s*(\d+)", line)
                if filtered_faces:
                    current_frame.filtered_faces = int(filtered_faces.group(1))
                    continue

                intersected = re.match(r"Intersected Objects:\s*(\d+)", line)
                if intersected:
                    current_frame.intersected_objects = int(intersected.group(1))
                    continue

                # Iteration
                iter = re.match(r"Iteration\s*(\d+)", line)
                if iter:

                    current_iteration = IterationData()
                    current_iteration.iteration = int(iter.group(1))

                    current_frame.iterations.append(current_iteration)

                    continue

                if current_iteration:

                    iscollided = re.match(r"Collision detected:\s*(YES|NO)", line)
                    if iscollided:
                        current_iteration.collision = (iscollided.group(1) == "YES")
                        continue

                    CD_time = re.match(r"Collision Detection Time:\s*(\d+)", line)
                    if CD_time:
                        current_iteration.collision_time = int(CD_time.group(1))
                        continue

                    CCD_time = re.match(r"CCD Deformation Time:\s*(\d+)", line)
                    if CCD_time:
                        current_iteration.ccd_time = int(CCD_time.group(1))
                        continue

                    alpha = re.match(r"Alpha:\s*([0-9.]+)", line)
                    if alpha:
                        current_iteration.alpha = float(alpha.group(1))
                        continue

                    collided_vertices = re.match(r"Collided Vertices:\s*(\d+)", line)
                    if collided_vertices:
                        current_iteration.vertices = int(collided_vertices.group(1))
                        continue

                    collided_faces = re.match(r"Collided Faces:\s*(\d+)", line)
                    if collided_faces:
                        current_iteration.faces = int(collided_faces.group(1))
                        continue


                # Summary
                all_iter = re.match(r"Iterations:\s*(\d+)", line)
                if all_iter:
                    current_frame.total_iterations = int(all_iter.group(1))
                    continue

                all_collided_vertices = re.match(r"Final Collided Vertices:\s*(\d+)", line)
                if all_collided_vertices:
                    current_frame.final_vertices = int(all_collided_vertices.group(1))
                    continue

                all_collided_faces = re.match(r"Final Collided Faces:\s*(\d+)", line)
                if all_collided_faces:
                    current_frame.final_faces = int(all_collided_faces.group(1))
                    continue

                smooth_time = re.match(r"Final Smooth Time:\s*(\d+)", line)
                if smooth_time:
                    current_frame.final_smooth_time = int(smooth_time.group(1))
                    continue

                total_time = re.match(r"Total Execution Time:\s*(\d+)", line)
                if total_time:
                    current_frame.total_execution_time = int(total_time.group(1))
                    continue

        return self.frames
    


log = LogParser()
log.parse(r"C:\Users\Geri\Documents\Projects\CG\MyMayaPlugin\resources\ImprovedDM_Log.txt")

frames = log.frames

for frame in frames:
    print(f"  Total Iterations:     {frame.total_iterations}")
