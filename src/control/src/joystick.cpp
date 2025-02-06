#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "mavros_msgs/msg/override_rc_in.hpp"
#include <vector>

// Function to map a value from one range to another
double map_value(double input_val, double input_min, double input_max, double output_min, double output_max) {
    return (input_val - input_min) * (output_max - output_min) / (input_max - input_min) + output_min;
}

class JoyMapperNode : public rclcpp::Node {
public:
    JoyMapperNode() : Node("joystick_node") {
        // Create publisher for MAVROS RC override
        rc_pub_ = this->create_publisher<mavros_msgs::msg::OverrideRCIn>("/mavros/rc/override", 10);

        // Create subscriber for joystick input
        joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "/joy", 10, std::bind(&JoyMapperNode::joy_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "JoystickNode initialized.");
    }

private:
    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) {
        // Joystick axes indices
        std::vector<int> axes_indices = {0, 1, 3, 4};  // Indices for mapping joystick axes

        // Create OverrideRCIn message
        auto rc_msg = mavros_msgs::msg::OverrideRCIn();
        
        // Set default value to 1500 (neutral position)
        for (int i = 0; i < 18; i++) {
            rc_msg.channels[i] = 1500;
        }

        // Map joystick values to RC channels
        rc_msg.channels[3] = static_cast<uint16_t>(map_value(msg->axes[axes_indices[0]], -1.0, 1.0, 1100.0, 1900.0)); // Roll
        rc_msg.channels[4] = static_cast<uint16_t>(map_value(msg->axes[axes_indices[1]], -1.0, 1.0, 1100.0, 1900.0)); // Pitch
        rc_msg.channels[1] = static_cast<uint16_t>(map_value(msg->axes[axes_indices[2]], -1.0, 1.0, 1100.0, 1900.0)); // Throttle
        rc_msg.channels[2] = static_cast<uint16_t>(map_value(msg->axes[axes_indices[3]], -1.0, 1.0, 1100.0, 1900.0)); // Yaw

        // Publish to MAVROS
        rc_pub_->publish(rc_msg);

        // Logging for debugging
        // RCLCPP_INFO(this->get_logger(), "Joystick values mapped: [ch4: %d, ch5: %d, ch2: %d, ch3: %d]",
        //             rc_msg.channels[3], rc_msg.channels[4], rc_msg.channels[1], rc_msg.channels[2]);
    }

    rclcpp::Publisher<mavros_msgs::msg::OverrideRCIn>::SharedPtr rc_pub_;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyMapperNode>());
    rclcpp::shutdown();
    return 0;
}
