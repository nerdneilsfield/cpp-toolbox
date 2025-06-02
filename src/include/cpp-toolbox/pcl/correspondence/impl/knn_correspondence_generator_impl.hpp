#pragma once

#include <algorithm>
#include <unordered_map>

namespace toolbox::pcl
{

template<typename DataType, typename Signature, typename KNN>
void knn_correspondence_generator_t<DataType, Signature, KNN>::compute_impl(
    std::vector<correspondence_t>& correspondences)
{
  // 检查KNN算法 / Check KNN algorithm
  if (!m_knn) {
    std::cerr << "错误：未设置KNN算法 / Error: KNN algorithm not set" << std::endl;
    return;
  }

  // 构建KNN数据集 / Build KNN dataset
  build_knn_dataset();

  // 第一步：为每个源描述子查找候选匹配 / Step 1: Find candidate matches for each source descriptor
  std::vector<correspondence_t> all_candidates;
  all_candidates.reserve(this->m_src_descriptors->size());

  for (std::size_t i = 0; i < this->m_src_descriptors->size(); ++i) {
    std::vector<correspondence_t> candidates;
    find_candidates_for_descriptor(i, candidates);
    
    this->m_total_candidates += candidates.size();
    
    // 应用比率测试 / Apply ratio test
    if (!candidates.empty() && this->apply_ratio_test(candidates)) {
      this->m_ratio_test_passed++;
      all_candidates.push_back(candidates[0]);  // 只保留最佳匹配 / Keep only best match
    }
  }

  // 第二步：双向验证（如果启用） / Step 2: Mutual verification (if enabled)
  std::vector<correspondence_t> verified_corrs;
  if (this->m_mutual_verification) {
    perform_mutual_verification(all_candidates, verified_corrs);
    this->m_mutual_test_passed = verified_corrs.size();
  } else {
    verified_corrs = std::move(all_candidates);
  }

  // 第三步：应用距离阈值 / Step 3: Apply distance threshold
  correspondences = std::move(verified_corrs);
  this->apply_distance_threshold(correspondences);
  this->m_distance_test_passed = correspondences.size();

  // 第四步：计算简单的几何一致性分数（仅供参考） / Step 4: Compute simple geometric consistency score (for reference)
  if (correspondences.size() >= 3) {
    float consistency_score = this->compute_geometric_consistency(correspondences);
    // 可以在统计信息中报告这个分数 / Can report this score in statistics
  }
}

template<typename DataType, typename Signature, typename KNN>
void knn_correspondence_generator_t<DataType, Signature, KNN>::build_knn_dataset()
{
  // 将目标描述子设置为KNN的数据集 / Set target descriptors as KNN dataset
  m_knn->set_input(*this->m_dst_descriptors);
}

template<typename DataType, typename Signature, typename KNN>
void knn_correspondence_generator_t<DataType, Signature, KNN>::find_candidates_for_descriptor(
    std::size_t src_idx, std::vector<correspondence_t>& candidates) const
{
  candidates.clear();
  
  // 查找K个最近邻（K=2用于比率测试） / Find K nearest neighbors (K=2 for ratio test)
  const std::size_t k = 2;
  std::vector<std::size_t> indices;
  std::vector<typename Signature::data_type> distances;
  
  // 执行KNN搜索 / Perform KNN search
  m_knn->kneighbors((*this->m_src_descriptors)[src_idx], k, indices, distances);
  
  // 转换为correspondence_t格式 / Convert to correspondence_t format
  for (std::size_t i = 0; i < indices.size(); ++i) {
    correspondence_t corr;
    corr.src_idx = (*this->m_src_keypoint_indices)[src_idx];
    corr.dst_idx = (*this->m_dst_keypoint_indices)[indices[i]];
    corr.distance = static_cast<float>(distances[i]);
    candidates.push_back(corr);
  }
}

template<typename DataType, typename Signature, typename KNN>
void knn_correspondence_generator_t<DataType, Signature, KNN>::perform_mutual_verification(
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
    for (std::size_t i = 0; i < this->m_dst_keypoint_indices->size(); ++i) {
      if ((*this->m_dst_keypoint_indices)[i] == forward_corr.dst_idx) {
        dst_desc_idx = i;
        break;
      }
    }
    
    // 临时创建一个只包含源描述子的KNN / Temporarily create a KNN with only source descriptors
    auto src_knn = std::make_shared<KNN>();
    src_knn->set_input(*this->m_src_descriptors);
    
    // 查找最近的源描述子 / Find nearest source descriptor
    std::vector<std::size_t> indices;
    std::vector<typename Signature::data_type> distances;
    src_knn->kneighbors((*this->m_dst_descriptors)[dst_desc_idx], 1, indices, distances);
    
    if (!indices.empty()) {
      std::size_t reverse_src_idx = (*this->m_src_keypoint_indices)[indices[0]];
      
      // 检查是否形成双向匹配 / Check if bidirectional match
      if (reverse_src_idx == forward_corr.src_idx) {
        verified_corrs.push_back(forward_corr);
      }
    }
  }
}

}  // namespace toolbox::pcl