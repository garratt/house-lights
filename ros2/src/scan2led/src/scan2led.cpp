// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include "laser_geometry/laser_geometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include <Eigen/Core>
#define POINT_FIELD sensor_msgs::msg::PointField
using std::placeholders::_1;

int SetSerialParams(int serial_port) {
  // Create new termios struct, we call it 'tty' for convention
  struct termios tty;

  // Read in existing settings, and handle any error
  if(tcgetattr(serial_port, &tty) != 0) {
      printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
  tty.c_cflag |= CS8; // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO; // Disable echo
  tty.c_lflag &= ~ECHOE; // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 9600
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);


  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }
  return 0;
}




class Scan2Led : public rclcpp::Node
{
public:
  Scan2Led()
  : Node("scan2led_node")
  {
    // Spherical->Cartesian projection
    for (size_t i = 0; i < n_pts; ++i) {
      co_sine_map_(i, 0) = cos(angle_min_ + static_cast<double>(i) * angle_increment_);
      co_sine_map_(i, 1) = sin(angle_min_ + static_cast<double>(i) * angle_increment_);
    }
    SetupCloutOut();
    subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
      "scan", 10, std::bind(&Scan2Led::topic_callback, this, _1));
    publisher_ = this->create_publisher<sensor_msgs::msg::PointCloud2>("outcloud", 10);
  
    serial_port_ = open("/dev/ttyACM0", O_RDWR);
    if(serial_port_ <=  0) {
      printf("Error %i opening serial port: %s\n", errno, strerror(errno));
    }
  }

private:
  void topic_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
  {
   // projector_.projectLaser(*msg, cloud_out);
   ProjectLaser(*msg);
   publisher_->publish(cloud_out);
    // RCLCPP_INFO(this->get_logger(), "I heard: %lu: %f, %f, %f", msg->ranges.size(), msg->angle_min, msg->angle_max, msg->angle_increment);

   // convert range of -1->5 to 1-100
   if (cloud_out.width && last_y < 5.0 && last_y > -1.0) {
     uint8_t msg[2];
     msg[0]='D';
     msg[1] = static_cast<uint8_t>((last_y + 1.0) * 16.6);
     RCLCPP_INFO(this->get_logger(), "last_y: %f  %u", last_y, msg[1]);
       if (serial_port_ > 0) {
       write(serial_port_, msg, sizeof(msg));
     }
   }
  }

  void SetupCloutOut() {
     cloud_out.width = 417;
     cloud_out.height = 1;
     cloud_out.fields.resize(3);
     cloud_out.fields[0].name = "x";
     cloud_out.fields[0].offset = 0;
     cloud_out.fields[0].datatype = POINT_FIELD::FLOAT32;
     cloud_out.fields[0].count = 1;
     cloud_out.fields[1].name = "y";
     cloud_out.fields[1].offset = 4;
     cloud_out.fields[1].datatype = POINT_FIELD::FLOAT32;
     cloud_out.fields[1].count = 1;
     cloud_out.fields[2].name = "z";
     cloud_out.fields[2].offset = 8;
     cloud_out.fields[2].datatype = POINT_FIELD::FLOAT32;
     cloud_out.fields[2].count = 1;
   
     cloud_out.point_step = 12;
     cloud_out.row_step = cloud_out.point_step * cloud_out.width;
     cloud_out.data.resize(cloud_out.row_step * cloud_out.height);
     cloud_out.is_dense = false;
  }

  void ProjectLaser(const sensor_msgs::msg::LaserScan & scan_in) {
     for (size_t i = 0; i < n_pts; ++i) {
        ranges(i, 0) = static_cast<double>(scan_in.ranges[i]);
        ranges(i, 1) = static_cast<double>(scan_in.ranges[i]);
     }
     output = ranges * co_sine_map_;

  cloud_out.header = scan_in.header;
  cloud_out.header.frame_id = "map";
  cloud_out.width = static_cast<uint32_t>(scan_in.ranges.size());
  cloud_out.row_step = cloud_out.point_step * cloud_out.width;
  cloud_out.data.resize(cloud_out.row_step * cloud_out.height);

  unsigned int count = 0;
  for (size_t i = 100; i < n_pts-130; ++i) {
    // check to see if we want to keep the point
    const float range = scan_in.ranges[i];
    if (range < scan_in.range_max && range >= 3.0) {

      auto pstep = reinterpret_cast<float *>(&cloud_out.data[count * cloud_out.point_step]);
      float x = range * co_sine_map_(i-12, 0);
      float y = range * co_sine_map_(i-12, 1);
      if (x > 8 && x <= 9.5) {
          last_y = y;
      // Copy XYZ
      // pstep[0] = static_cast<float>(output(i, 0));
      // pstep[1] = static_cast<float>(output(i, 1));
      pstep[0] = x;
      // pstep[0] = range * co_sine_map_(i-12, 0);
      pstep[1] = y;
      // pstep[1] = y;
      pstep[2] = 0;

      // make sure to increment count
      ++count;
      }
    }

  }

  // resize if necessary
  cloud_out.width = count;
  cloud_out.row_step = cloud_out.point_step * cloud_out.width;
  cloud_out.data.resize(cloud_out.row_step * cloud_out.height);
  }




  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr publisher_;
  laser_geometry::LaserProjection projector_;

  int serial_port_;
  float last_y = 0;
  size_t n_pts = 417;
  float angle_min_ = -2.359030;
  float angle_max_ = 2.359030;
  float angle_increment_ = 0.011341;
  sensor_msgs::msg::PointCloud2 cloud_out;
  Eigen::ArrayXXd co_sine_map_ = Eigen::ArrayXXd(n_pts, 2);
  Eigen::ArrayXXd ranges = Eigen::ArrayXXd(n_pts, 2);
  Eigen::ArrayXXd output = Eigen::ArrayXXd(n_pts, 2);
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Scan2Led>());
  rclcpp::shutdown();
  return 0;
}
