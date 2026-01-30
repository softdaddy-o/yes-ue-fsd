"""
AutoDriver Python Helper Module
Provides high-level Python interface for AutoDriver automation
"""

import unreal
import time
from typing import Optional, List, Tuple


class AutoDriver:
    """Main interface for AutoDriver automation in Python"""

    def __init__(self, player_index: int = 0):
        """Initialize AutoDriver for specific player

        Args:
            player_index: Player index (default: 0)
        """
        self.player_index = player_index
        self.bridge = unreal.AutoDriverPythonBridge

    def move_to(self, location: unreal.Vector,
                acceptance_radius: float = 50.0,
                speed: float = 1.0,
                wait: bool = True,
                timeout: float = 10.0) -> bool:
        """Move to a location

        Args:
            location: Target location (Vector)
            acceptance_radius: How close to get (default: 50 units)
            speed: Speed multiplier (default: 1.0)
            wait: Wait for completion (default: True)
            timeout: Max wait time in seconds (default: 10.0)

        Returns:
            True if movement started (and completed if wait=True)
        """
        success = self.bridge.move_to_location(location, acceptance_radius, speed, self.player_index)

        if success and wait:
            self.wait_for_completion(timeout)

        return success

    def move_to_actor(self, actor: unreal.Actor,
                     acceptance_radius: float = 50.0,
                     speed: float = 1.0,
                     wait: bool = True,
                     timeout: float = 10.0) -> bool:
        """Move to an actor

        Args:
            actor: Target actor
            acceptance_radius: How close to get
            speed: Speed multiplier
            wait: Wait for completion
            timeout: Max wait time

        Returns:
            True if movement started
        """
        success = self.bridge.move_to_actor(actor, acceptance_radius, speed, self.player_index)

        if success and wait:
            self.wait_for_completion(timeout)

        return success

    def rotate_to(self, rotation: unreal.Rotator,
                  wait: bool = True,
                  timeout: float = 5.0) -> bool:
        """Rotate to a specific rotation

        Args:
            rotation: Target rotation
            wait: Wait for completion
            timeout: Max wait time

        Returns:
            True if rotation started
        """
        success = self.bridge.rotate_to_rotation(rotation, self.player_index)

        if success and wait:
            self.wait_for_completion(timeout)

        return success

    def look_at(self, location: unreal.Vector,
                wait: bool = True,
                timeout: float = 5.0) -> bool:
        """Look at a location

        Args:
            location: Target location to look at
            wait: Wait for completion
            timeout: Max wait time

        Returns:
            True if rotation started
        """
        success = self.bridge.look_at_location(location, self.player_index)

        if success and wait:
            self.wait_for_completion(timeout)

        return success

    def look_at_actor(self, actor: unreal.Actor,
                     wait: bool = True,
                     timeout: float = 5.0) -> bool:
        """Look at an actor

        Args:
            actor: Target actor
            wait: Wait for completion
            timeout: Max wait time

        Returns:
            True if rotation started
        """
        success = self.bridge.look_at_actor(actor, self.player_index)

        if success and wait:
            self.wait_for_completion(timeout)

        return success

    def press_button(self, action_name: str) -> bool:
        """Press a button/action

        Args:
            action_name: Name of the action (e.g., "Jump", "Fire")

        Returns:
            True if button press succeeded
        """
        return self.bridge.press_button(action_name, self.player_index)

    def set_axis(self, axis_name: str, value: float) -> bool:
        """Set an axis value

        Args:
            axis_name: Name of the axis
            value: Value to set (-1.0 to 1.0)

        Returns:
            True if successful
        """
        return self.bridge.set_axis_value(axis_name, value, self.player_index)

    def is_executing(self) -> bool:
        """Check if currently executing a command

        Returns:
            True if executing
        """
        return self.bridge.is_executing_command(self.player_index)

    def stop(self):
        """Stop current command"""
        self.bridge.stop_current_command(self.player_index)

    def wait_for_completion(self, timeout: float = 10.0):
        """Wait for current command to complete

        Args:
            timeout: Max wait time in seconds
        """
        self.bridge.wait_for_command_completion(timeout, self.player_index)

    def is_reachable(self, location: unreal.Vector) -> bool:
        """Check if a location is reachable

        Args:
            location: Location to check

        Returns:
            True if reachable
        """
        return self.bridge.is_location_reachable(location, self.player_index)

    def get_path_length(self, from_loc: unreal.Vector, to_loc: unreal.Vector) -> float:
        """Get path length between two locations

        Args:
            from_loc: Start location
            to_loc: End location

        Returns:
            Path length in units (-1 if unreachable)
        """
        return self.bridge.get_path_length(from_loc, to_loc, self.player_index)

    def get_random_location(self, origin: unreal.Vector, radius: float = 1000.0) -> unreal.Vector:
        """Get random reachable location

        Args:
            origin: Center point
            radius: Search radius

        Returns:
            Random reachable location
        """
        return self.bridge.get_random_reachable_location(origin, radius, self.player_index)

    @property
    def location(self) -> unreal.Vector:
        """Get current player location"""
        return self.bridge.get_player_location(self.player_index)

    @property
    def rotation(self) -> unreal.Rotator:
        """Get current player rotation"""
        return self.bridge.get_player_rotation(self.player_index)


class ActionRecorder:
    """Record player actions to timeline"""

    def __init__(self, player_index: int = 0):
        self.player_index = player_index
        self.bridge = unreal.AutoDriverPythonBridge
        self.recorder = self.bridge.create_recorder(player_index)

    def start(self, name: str = "Python Recording"):
        """Start recording

        Args:
            name: Recording name
        """
        if self.recorder:
            self.recorder.start_recording(name)

    def stop(self) -> Optional[unreal.ActionTimeline]:
        """Stop recording

        Returns:
            Recorded timeline
        """
        if self.recorder:
            self.recorder.stop_recording()
            return self.recorder.get_current_timeline()
        return None

    def pause(self):
        """Pause recording"""
        if self.recorder:
            self.recorder.pause_recording()

    def resume(self):
        """Resume recording"""
        if self.recorder:
            self.recorder.resume_recording()

    def save(self, file_path: str) -> bool:
        """Save recording to file

        Args:
            file_path: Path to save file

        Returns:
            True if successful
        """
        if self.recorder:
            return self.recorder.save_recording(file_path)
        return False


class ActionPlayer:
    """Playback recorded actions"""

    def __init__(self, player_index: int = 0):
        self.player_index = player_index
        self.bridge = unreal.AutoDriverPythonBridge
        self.playback = self.bridge.create_playback(player_index)

    def play(self, timeline: unreal.ActionTimeline):
        """Play a timeline

        Args:
            timeline: Timeline to play
        """
        if self.playback:
            self.playback.play(timeline)

    def load_and_play(self, file_path: str):
        """Load and play timeline from file

        Args:
            file_path: Path to timeline file
        """
        if self.playback:
            self.playback.load_and_play_timeline(file_path)

    def stop(self):
        """Stop playback"""
        if self.playback:
            self.playback.stop()

    def pause(self):
        """Pause playback"""
        if self.playback:
            self.playback.pause()

    def resume(self):
        """Resume playback"""
        if self.playback:
            self.playback.resume()

    def set_speed(self, speed: float):
        """Set playback speed

        Args:
            speed: Speed multiplier (1.0 = normal)
        """
        if self.playback:
            self.playback.set_playback_speed(speed)

    @property
    def is_playing(self) -> bool:
        """Check if currently playing"""
        return self.playback.is_playing() if self.playback else False


# Utility functions

def find_actor(name: str) -> Optional[unreal.Actor]:
    """Find actor by name

    Args:
        name: Actor name

    Returns:
        Actor if found, None otherwise
    """
    return unreal.AutoDriverPythonBridge.find_actor_by_name(name)


def get_actors_of_class(actor_class: type) -> List[unreal.Actor]:
    """Get all actors of a specific class

    Args:
        actor_class: Actor class type

    Returns:
        List of actors
    """
    return unreal.AutoDriverPythonBridge.get_all_actors_of_class(actor_class)


def execute_command(command: str):
    """Execute console command

    Args:
        command: Console command to execute
    """
    unreal.AutoDriverPythonBridge.execute_console_command(command)


def wait(seconds: float):
    """Wait for specified duration

    Args:
        seconds: Time to wait
    """
    time.sleep(seconds)
