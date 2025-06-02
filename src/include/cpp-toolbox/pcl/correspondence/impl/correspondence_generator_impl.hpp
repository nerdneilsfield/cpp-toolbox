#pragma once

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cmath>

namespace toolbox::pcl
{

template<typename DataType, typename Signature, typename KNN>
void correspondence_generator_t<DataType, Signature, KNN>::compute(
    std::vector<correspondence_t>& correspondences)
{
  // 清空输出和统计信息 / Clear output and statistics
  correspondences.clear();
  m_total_candidates = 0;
  m_ratio_test_passed = 0;
  m_mutual_test_passed = 0;
  m_distance_test_passed = 0;

  // 验证输入 / Validate input
  if (!validate_input()) {
    return;
  }

  // 构建KNN数据集 / Build KNN dataset
  build_knn_dataset();

  // 第一步：为每个源描述子查找候选匹配 / Step 1: Find candidate matches for each source descriptor
  std::vector<correspondence_t> all_candidates;
  all_candidates.reserve(m_src_descriptors->size());

  for (std::size_t i = 0; i < m_src_descriptors->size(); ++i) {
    std::vector<correspondence_t> candidates;
    find_candidates_for_descriptor(i, candidates);
    
    m_total_candidates += candidates.size();
    
    // 应用比率测试 / Apply ratio test
    if (!candidates.empty() && apply_ratio_test(candidates)) {
      m_ratio_test_passed++;
      all_candidates.push_back(candidates[0]);  // 只保留最佳匹配 / Keep only best match
    }
  }

  // 第二步：双向验证（如果启用） / Step 2: Mutual verification (if enabled)
  std::vector<correspondence_t> verified_corrs;
  if (m_mutual_verification) {
    perform_mutual_verification(all_candidates, verified_corrs);
    m_mutual_test_passed = verified_corrs.size();
  } else {
    verified_corrs = std::move(all_candidates);
  }

  // 第三步：应用距离阈值 / Step 3: Apply distance threshold
  correspondences = std::move(verified_corrs);
  apply_distance_threshold(correspondences);
  m_distance_test_passed = correspondences.size();

  // 第四步：计算简单的几何一致性分数（仅供参考） / Step 4: Compute simple geometric consistency score (for reference)
  if (correspondences.size() >= 3) {
    float consistency_score = compute_geometric_consistency(correspondences);
    // 可以在统计信息中报告这个分数 / Can report this score in statistics
  }
}

template<typename DataType, typename Signature, typename KNN>
bool correspondence_generator_t<DataType, Signature, KNN>::validate_input() const
{
  // 检查KNN算法 / Check KNN algorithm
  if (!m_knn) {
    std::cerr << "错误：未设置KNN算法 / Error: KNN algorithm not set" << std::endl;
    return false;
  }

  // 检查源数据 / Check source data
  if (!m_src_cloud || !m_src_descriptors || !m_src_keypoint_indices) {
    std::cerr << "错误：源数据不完整 / Error: Source data incomplete" << std::endl;
    return false;
  }

  // 检查目标数据 / Check target data
  if (!m_dst_cloud || !m_dst_descriptors || !m_dst_keypoint_indices) {
    std::cerr << "错误：目标数据不完整 / Error: Target data incomplete" << std::endl;
    return false;
  }

  // 检查描述子和关键点数量是否匹配 / Check if descriptors and keypoints match
  if (m_src_descriptors->size() != m_src_keypoint_indices->size()) {
    std::cerr << "错误：源描述子数量与关键点数量不匹配 / Error: Source descriptor count doesn't match keypoint count" << std::endl;
    return false;
  }

  if (m_dst_descriptors->size() != m_dst_keypoint_indices->size()) {
    std::cerr << "错误：目标描述子数量与关键点数量不匹配 / Error: Target descriptor count doesn't match keypoint count" << std::endl;
    return false;
  }

  // 检查参数有效性 / Check parameter validity
  if (m_ratio <= 0.0f || m_ratio >= 1.0f) {
    std::cerr << "警告：比率测试阈值应在(0,1)之间 / Warning: Ratio test threshold should be in (0,1)" << std::endl;
  }

  return true;
}

template<typename DataType, typename Signature, typename KNN>
void correspondence_generator_t<DataType, Signature, KNN>::build_knn_dataset()
{
  // 将目标描述子设置为KNN的数据集 / Set target descriptors as KNN dataset
  // 注意：这里需要根据KNN的具体接口来调整 / Note: This needs to be adjusted based on KNN's specific interface
  m_knn->set_input(*m_dst_descriptors);
}

template<typename DataType, typename Signature, typename KNN>
void correspondence_generator_t<DataType, Signature, KNN>::find_candidates_for_descriptor(
    std::size_t src_idx, std::vector<correspondence_t>& candidates) const
{
  candidates.clear();
  
  // 查找K个最近邻（K=2用于比率测试） / Find K nearest neighbors (K=2 for ratio test)
  const std::size_t k = 2;
  std::vector<std::size_t> indices;
  std::vector<typename Signature::data_type> distances;
  
  // 执行KNN搜索 / Perform KNN search
  m_knn->kneighbors((*m_src_descriptors)[src_idx], k, indices, distances);
  
  // 转换为correspondence_t格式 / Convert to correspondence_t format
  for (std::size_t i = 0; i < indices.size(); ++i) {
    correspondence_t corr;
    corr.src_idx = (*m_src_keypoint_indices)[src_idx];
    corr.dst_idx = (*m_dst_keypoint_indices)[indices[i]];
    corr.distance = static_cast<float>(distances[i]);
    candidates.push_back(corr);
  }
}

template<typename DataType, typename Signature, typename KNN>
bool correspondence_generator_t<DataType, Signature, KNN>::apply_ratio_test(
    const std::vector<correspondence_t>& candidates) const
{
  // Lowe's ratio test: 检查最佳匹配是否显著优于次佳匹配 / Check if best match is significantly better than second best
  if (candidates.size() < 2) {
    return true;  // 如果只有一个匹配，默认通过 / If only one match, pass by default
  }
  
  return (candidates[0].distance < m_ratio * candidates[1].distance);
}

template<typename DataType, typename Signature, typename KNN>
void correspondence_generator_t<DataType, Signature, KNN>::perform_mutual_verification(
    const std::vector<correspondence_t>& forward_corrs,
    std::vector<correspondence_t>& verified_corrs)
{
  verified_corrs.clear();
  
  // 构建反向查找表：目标索引 -> 源索引 / Build reverse lookup: target index -> source index
  std::unordered_map<std::size_t, std::size_t> dst_to_src;
  for (const auto& corr : forward_corrs) {
    dst_to_src[corr.dst_idx] = corr.src_idx;
  }
  
  // 对每个目标描述子执行反向搜索 / Perform reverse search for each target descriptor
  for (const auto& forward_corr : forward_corrs) {
    // 获取目标描述子在源描述子集中的索引 / Get target descriptor's index in source descriptor set
    std::size_t dst_desc_idx = 0;
    for (std::size_t i = 0; i < m_dst_keypoint_indices->size(); ++i) {
      if ((*m_dst_keypoint_indices)[i] == forward_corr.dst_idx) {
        dst_desc_idx = i;
        break;
      }
    }
    
    // 临时创建一个只包含源描述子的KNN / Temporarily create a KNN with only source descriptors
    auto src_knn = std::make_shared<KNN>();
    src_knn->set_input(*m_src_descriptors);
    
    // 查找最近的源描述子 / Find nearest source descriptor
    std::vector<std::size_t> indices;
    std::vector<typename Signature::data_type> distances;
    src_knn->kneighbors((*m_dst_descriptors)[dst_desc_idx], 1, indices, distances);
    
    if (!indices.empty()) {
      std::size_t reverse_src_idx = (*m_src_keypoint_indices)[indices[0]];
      
      // 检查是否形成双向匹配 / Check if bidirectional match
      if (reverse_src_idx == forward_corr.src_idx) {
        verified_corrs.push_back(forward_corr);
      }
    }
  }
}

template<typename DataType, typename Signature, typename KNN>
void correspondence_generator_t<DataType, Signature, KNN>::apply_distance_threshold(
    std::vector<correspondence_t>& correspondences) const
{
  // 移除距离超过阈值的对应关系 / Remove correspondences exceeding distance threshold
  correspondences.erase(
      std::remove_if(correspondences.begin(), correspondences.end(),
                     [this](const correspondence_t& corr) {
                       return corr.distance > m_distance_threshold;
                     }),
      correspondences.end());
}

template<typename DataType, typename Signature, typename KNN>
float correspondence_generator_t<DataType, Signature, KNN>::compute_geometric_consistency(
    const std::vector<correspondence_t>& correspondences) const
{
  if (correspondences.size() < 3) {
    return 0.0f;
  }
  
  // 简单的几何一致性检查：比较点对之间的距离比率 / Simple geometric consistency: compare distance ratios between point pairs
  float consistency_score = 0.0f;
  std::size_t valid_pairs = 0;
  
  for (std::size_t i = 0; i < correspondences.size() - 1; ++i) {
    for (std::size_t j = i + 1; j < correspondences.size(); ++j) {
      // 计算源点对之间的距离 / Compute distance between source point pair
      const auto& src_p1 = m_src_cloud->points[correspondences[i].src_idx];
      const auto& src_p2 = m_src_cloud->points[correspondences[j].src_idx];
      DataType src_dist = std::sqrt(
          (src_p1.x - src_p2.x) * (src_p1.x - src_p2.x) +
          (src_p1.y - src_p2.y) * (src_p1.y - src_p2.y) +
          (src_p1.z - src_p2.z) * (src_p1.z - src_p2.z));
      
      // 计算目标点对之间的距离 / Compute distance between target point pair
      const auto& dst_p1 = m_dst_cloud->points[correspondences[i].dst_idx];
      const auto& dst_p2 = m_dst_cloud->points[correspondences[j].dst_idx];
      DataType dst_dist = std::sqrt(
          (dst_p1.x - dst_p2.x) * (dst_p1.x - dst_p2.x) +
          (dst_p1.y - dst_p2.y) * (dst_p1.y - dst_p2.y) +
          (dst_p1.z - dst_p2.z) * (dst_p1.z - dst_p2.z));
      
      // 避免除零 / Avoid division by zero
      if (src_dist > 0.001 && dst_dist > 0.001) {
        DataType ratio = src_dist / dst_dist;
        // 如果比率接近1，说明几何关系保持良好 / If ratio is close to 1, geometry is well preserved
        if (ratio > 0.8 && ratio < 1.2) {
          consistency_score += 1.0f;
        }
        valid_pairs++;
      }
    }
  }
  
  return valid_pairs > 0 ? consistency_score / valid_pairs : 0.0f;
}

template<typename DataType, typename Signature, typename KNN>
std::string correspondence_generator_t<DataType, Signature, KNN>::get_statistics() const
{
  std::stringstream ss;
  ss << "对应点生成统计 / Correspondence Generation Statistics:\n";
  ss << "  候选对应点总数 / Total candidates: " << m_total_candidates << "\n";
  ss << "  通过比率测试 / Passed ratio test: " << m_ratio_test_passed << "\n";
  if (m_mutual_verification) {
    ss << "  通过双向验证 / Passed mutual verification: " << m_mutual_test_passed << "\n";
  }
  ss << "  通过距离阈值 / Passed distance threshold: " << m_distance_test_passed << "\n";
  
  if (m_src_descriptors) {
    float match_rate = static_cast<float>(m_distance_test_passed) / m_src_descriptors->size();
    ss << "  匹配率 / Match rate: " << (match_rate * 100) << "%\n";
  }
  
  return ss.str();
}

}  // namespace toolbox::pcl