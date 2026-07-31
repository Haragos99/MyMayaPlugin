from scripts.analayze.log_parser import FrameData, IterationData


class StatisticsGenerator:
    """Generates formatted statistics for LogVisualizerWindow."""

    # OVERALL (ALL FRAMES)
    @staticmethod
    def overall_summary(frames: list[FrameData]):

        if not frames:
            return "No data loaded."

        total_frames = len(frames)

        total_time = [f.total_execution_time for f in frames]
        smooth_time = [f.final_smooth_time for f in frames]
        filter_time = [f.filter_time for f in frames]
        iterations = [f.total_iterations for f in frames]
        filtered_faces = [f.filtered_faces for f in frames]

        text = []
        text.append("====== OVERALL STATISTICS ======")
        text.append(f"Frames: {total_frames}")
        text.append("")

        text.append(f"Avg Total Time   : {sum(total_time)/total_frames:.2f} ms")
        text.append(f"Avg Smooth Time  : {sum(smooth_time)/total_frames:.2f} ms")
        text.append(f"Avg Filter Time  : {sum(filter_time)/total_frames:.2f} ms")
        text.append(f"Avg Iterations   : {sum(iterations)/total_frames:.2f}")
        text.append(f"Avg Faces        : {sum(filtered_faces)/total_frames:.2f}")
        text.append("")

        text.append(f"Min Total Time   : {min(total_time)} ms")
        text.append(f"Max Total Time   : {max(total_time)} ms")
        text.append(f"All Total Time   : {sum(total_time)} ms")

        return "\n".join(text)


    # SINGLE FRAME
    @staticmethod
    def frame_summary(frame: FrameData):

        if frame is None:
            return "No frame selected."

        text = []

        text.append(f"====== FRAME {frame.frame} ======")
        text.append(f"Delta Mush Deformation      : {frame.smooth_time} ms")
        text.append(f"Final Smooth Time      : {frame.final_smooth_time} ms")
        text.append(f"Filter Time      : {frame.filter_time} ms")
        text.append(f"Total Time       : {frame.total_execution_time} ms")
        text.append(f"Iterations       : {frame.total_iterations}")
        text.append(f"Filtered Faces   : {frame.filtered_faces}")

        text.append("")


        # ITERATION STATS (if exists)
        iterations = getattr(frame, "iterations", [])

        if iterations:

            alphas = [i.alpha for i in iterations if hasattr(i, "alpha")]
            ccd_times = [i.ccd_time for i in iterations if hasattr(i, "ccd_time")]
            collision_times = [i.collision_time for i in iterations if hasattr(i, "collision_time")]

            text.append("----- ITERATION STATISTICS -----")
            text.append(f"Iterations        : {len(iterations)}")

            if alphas:
                text.append(f"Avg Alpha         : {sum(alphas)/len(alphas):.4f}")
                text.append(f"Min Alpha         : {min(alphas):.4f}")
                text.append(f"Max Alpha         : {max(alphas):.4f}")

            if collision_times:
                text.append(f"Avg Collision Time: {sum(collision_times)/len(collision_times):.2f} ms")
                text.append(f"All Collision Time: {sum(collision_times):.2f} ms")

            if ccd_times:
                text.append(f"Avg CCD Def Time      : {sum(ccd_times)/len(ccd_times):.2f} ms")
                text.append(f"All CCD Def Time      : {sum(ccd_times):.2f} ms")

        return "\n".join(text)