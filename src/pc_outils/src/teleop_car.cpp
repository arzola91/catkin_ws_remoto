#include <ros/ros.h>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include "std_msgs/String.h"
#include "std_msgs/Int16.h"
#include "std_msgs/Int32.h"
#include <sstream>

#define KEYCODE_R 0x43 
#define KEYCODE_L 0x44
#define KEYCODE_U 0x41
#define KEYCODE_D 0x42
#define KEYCODE_Q 0x71

class TeleopTurtle
{
public:
  TeleopTurtle();
  void keyLoop();

private:


  ros::NodeHandle n;
  std_msgs::Int16 steer_msg;
  std_msgs::Int32 speed_msg;
  std::stringstream ss;
  short int offset = 0;
  ros::Publisher steer_pub_;
  ros::Publisher speed_pub_;

};

TeleopTurtle::TeleopTurtle()
{
  steer_pub_ = n.advertise<std_msgs::Int16>("manual_control/steering", 1000);
  speed_pub_ = n.advertise<std_msgs::Int32>("manual_control/speed", 1000);
}


int kfd = 0;
struct termios cooked, raw;

void quit(int sig)
{
  tcsetattr(kfd, TCSANOW, &cooked);
  ros::shutdown();
  exit(0);
}


int main(int argc, char** argv)
{
  ros::init(argc, argv, "teleoperation");
  TeleopTurtle teleop_turtle;

  signal(SIGINT,quit);

  teleop_turtle.keyLoop();

  return(0);
}


void TeleopTurtle::keyLoop()
{
  char c;
  bool dirty_st=false;
  bool dirty_sp=false;


  // get the console in raw mode                                                              
  tcgetattr(kfd, &cooked);
  memcpy(&raw, &cooked, sizeof(struct termios));
  raw.c_lflag &=~ (ICANON | ECHO);
  // Setting a new line, then end of file                         
  raw.c_cc[VEOL] = 1;
  raw.c_cc[VEOF] = 2;
  tcsetattr(kfd, TCSANOW, &raw);

  puts("Reading from keyboard");
  puts("---------------------------");
  puts("Use arrow keys to move the car.");


  for(;;)
  {
    // get the next event from the keyboard  
    if(read(kfd, &c, 1) < 0)
    {
      perror("read():");
      exit(-1);
    }

    ss.str("");
    ROS_DEBUG("value: 0x%02X\n", c);

    switch(c)
    {
      case KEYCODE_L:
        ROS_DEBUG("LEFT");
        offset = offset-100;
        steer_msg.data = 1488+offset;
        dirty_st = true;
        break;
      case KEYCODE_R:
        ROS_DEBUG("RIGHT");
        offset = offset+100;
        steer_msg.data = 1488+offset;
        dirty_st = true;
        break;
      case KEYCODE_U:
        ROS_DEBUG("UP");
        speed_msg.data = speed_msg.data+50;
        dirty_sp = true;
        break;
      case KEYCODE_D:
        ROS_DEBUG("DOWN");
        speed_msg.data = speed_msg.data-50;
        dirty_sp = true;
        break;
    }

     
    ROS_INFO("%d", speed_msg.data);
    if(dirty_st ==true)
    {
      steer_pub_.publish(steer_msg);
      dirty_st=false;
    }
    if(dirty_sp ==true)
    {
      speed_pub_.publish(speed_msg);
      dirty_sp=false;
    }
  }


  return;
}



