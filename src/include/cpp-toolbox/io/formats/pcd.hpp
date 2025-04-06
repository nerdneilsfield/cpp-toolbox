#pragma once

#include <cpp-toolbox/io/formats/base.hpp>

namespace cpp_toolbox::io
{

class pcd_file_data_t : public base_file_data_t
{
public:
  std::vector<Eigen::Vector3d> points;
  std::vector<Eigen::Vector3d> normals;
  std::vector<Eigen::Vector3d> colors;
};

}  // namespace cpp_toolbox::io
