#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "mavros_msgs/msg/override_rc_in.hpp"

class JoyController : public rclcpp::Node {
public:
    JoyController() : Node("joystick_node") {
        // Declare and get the mode parameter
        this->declare_parameter<bool>("use_simulator", false);
        use_simulator_ = this->get_parameter("use_simulator").as_bool();
    
        if (!use_simulator_) {
            rc_pub_ = this->create_publisher<mavros_msgs::msg::OverrideRCIn>("/mavros/rc/override", 10);
            RCLCPP_INFO(this->get_logger(), "Joystick in MAVROS mode.");
        } else {
            vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/orca/cmd_vel", 10);
            RCLCPP_INFO(this->get_logger(), "Joystick in SIMULATION mode.");
        }

        // Create subscriber for joystick input
        joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>("/joy", 10, std::bind(&JoyController::joy_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "JoystickNode initialized.");
    }

private:
    bool use_simulator_;
    rclcpp::Publisher<mavros_msgs::msg::OverrideRCIn>::SharedPtr rc_pub_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr vel_pub_;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;

    // Function to map a value from one range to another
    double map_value(double input_val, double input_min, double input_max, double output_min, double output_max) {
        return (input_val - input_min) * (output_max - output_min) / (input_max - input_min) + output_min;
    }

    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) {
        if (!use_simulator_) {
            auto rc_msg = mavros_msgs::msg::OverrideRCIn();
            for (int i = 0; i < 4; i++) rc_msg.channels[i] = 1500;

            rc_msg.channels[3] = static_cast<uint16_t>(map_value(msg->axes[0], -1.0, 1.0, 1100.0, 1900.0)); // Roll
            rc_msg.channels[4] = static_cast<uint16_t>(map_value(msg->axes[1], -1.0, 1.0, 1100.0, 1900.0)); // Pitch
            rc_msg.channels[1] = static_cast<uint16_t>(map_value(msg->axes[3], -1.0, 1.0, 1100.0, 1900.0)); // Throttle
            rc_msg.channels[2] = static_cast<uint16_t>(map_value(msg->axes[4], -1.0, 1.0, 1100.0, 1900.0)); // Yaw

            rc_pub_->publish(rc_msg);
        } else {
            auto twist_msg = geometry_msgs::msg::Twist();
            twist_msg.linear.x = msg->axes[1] * 1.0; // Forward/Backward (left stick Y)
            twist_msg.linear.y = msg->axes[0] * 1.0; // Left-Right (left stick X)
            twist_msg.linear.z = msg->axes[3] * 1.0; // Up/Down (right stick Y)
            twist_msg.angular.z = msg->axes[2] * 1.0; // Up/Down (right stick X)

            vel_pub_->publish(twist_msg);
        }
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyController>());
    rclcpp::shutdown();
    return 0;
}
