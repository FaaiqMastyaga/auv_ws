import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    return LaunchDescription([
        # Declare joystick device argument
        DeclareLaunchArgument(
            "joy_dev", default_value="/dev/input/js0",
            description="Joystick device (e.g., /dev/input/js0)"
        ),

        # Start the joystick node
        Node(
            package="joy",
            executable="joy_node",
            name="joystick",
            parameters=[{"device": LaunchConfiguration("joy_dev")}],
            output="screen"
        ),

        # Start the MAVROS node
        Node(
            package="mavros",
            executable="mavros_node",
            name="mavros",
            output="screen",
            parameters=[{
                "fcu_url": "/dev/ttyACM0:57600",
                "gcs_url": "",
                "tgt_system": 1,
                "tgt_component": 1
            }]
        ),

        # Start the joystick to MAVROS RC override node
        Node(
            package="control",  # Change to your package name
            executable="joystick_node",
            name="joystick",
            output="screen"
        )
    ])
