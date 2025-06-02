#pragma once

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <cpp-toolbox/base/thread_pool_singleton.hpp>

namespace toolbox::pcl
{

template<typename DataType, typename Signature>
void brute_force_correspondence_generator_t<DataType, Signature>::compute_impl(
    std::vector<correspondence_t>& correspondences)
{
  if (m_enable_parallel) {
    // 并行版本 / Parallel version
    std::vector<correspondence_t> all_candidates;
    compute_all_candidates_parallel(all_candidates);
    
    // 双向验证（如果启用） / Mutual verification (if enabled)
    if (this->m_mutual_verification) {
      std::vector<correspondence_t> verified_corrs;
      perform_mutual_verification(all_candidates, verified_corrs);
      this->m_mutual_test_passed = verified_corrs.size();
      correspondences = std::move(verified_corrs);
    } else {
      correspondences = std::move(all_candidates);
    }
  } else {
    // 串行版本 / Serial version
    std::vector<correspondence_t> all_candidates;
    all_candidates.reserve(this->m_src_descriptors->size());

    for (std::size_t i = 0; i < this->m_src_descriptors->size(); ++i) {
      std::vector<correspondence_t> candidates;
      find_candidates_for_descriptor_serial(i, candidates);
      
      this->m_total_candidates += candidates.size();
      
      // 应用比率测试 / Apply ratio test
      if (!candidates.empty() && this->apply_ratio_test(candidates)) {
        this->m_ratio_test_passed++;
        all_candidates.push_back(candidates[0]);  // 只保留最佳匹配 / Keep only best match
      }
    }

    // 双向验证（如果启用） / Mutual verification (if enabled)
    if (this->m_mutual_verification) {
      std::vector<correspondence_t> verified_corrs;
      perform_mutual_verification(all_candidates, verified_corrs);
      this->m_mutual_test_passed = verified_corrs.size();
      correspondences = std::move(verified_corrs);
    } else {
      correspondences = std::move(all_candidates);
    }
  }

  // 应用距离阈值 / Apply distance threshold
  this->apply_distance_threshold(correspondences);
  this->m_distance_test_passed = correspondences.size();

  // 计算简单的几何一致性分数（仅供参考） / Compute simple geometric consistency score (for reference)
  if (correspondences.size() >= 3) {
    float consistency_score = this->compute_geometric_consistency(correspondences);
    // 可以在统计信息中报告这个分数 / Can report this score in statistics
  }
}

template<typename DataType, typename Signature>
void brute_force_correspondence_generator_t<DataType, Signature>::find_candidates_for_descriptor_serial(
    std::size_t src_idx, std::vector<correspondence_t>& candidates) const
{
  candidates.clear();
  
  const auto& src_desc = (*this->m_src_descriptors)[src_idx];
  
  // 存储所有距离和索引 / Store all distances and indices
  std::vector<std::pair<float, std::size_t>> distance_index_pairs;
  distance_index_pairs.reserve(this->m_dst_descriptors->size());
  
  // 计算到所有目标描述子的距离 / Compute distances to all target descriptors
  for (std::size_t i = 0; i < this->m_dst_descriptors->size(); ++i) {
    float dist = src_desc.distance((*this->m_dst_descriptors)[i]);
    distance_index_pairs.emplace_back(dist, i);
  }
  
  // 部分排序找到最近的两个（用于比率测试） / Partial sort to find nearest two (for ratio test)
  std::partial_sort(distance_index_pairs.begin(),
                    distance_index_pairs.begin() + std::min(size_t(2), distance_index_pairs.size()),
                    distance_index_pairs.end());
  
  // 转换为correspondence_t格式 / Convert to correspondence_t format
  for (std::size_t i = 0; i < std::min(size_t(2), distance_index_pairs.size()); ++i) {
    correspondence_t corr;
    corr.src_idx = (*this->m_src_keypoint_indices)[src_idx];
    corr.dst_idx = (*this->m_dst_keypoint_indices)[distance_index_pairs[i].second];
    corr.distance = distance_index_pairs[i].first;
    candidates.push_back(corr);
  }
}

template<typename DataType, typename Signature>
void brute_force_correspondence_generator_t<DataType, Signature>::compute_all_candidates_parallel(
    std::vector<correspondence_t>& all_candidates)
{
  const std::size_t num_src = this->m_src_descriptors->size();
  
  // 预分配结果向量 / Pre-allocate result vectors
  std::vector<std::vector<correspondence_t>> thread_results(num_src);
  
  // 获取线程池单例 / Get thread pool singleton
  auto& pool_singleton = toolbox::base::thread_pool_singleton_t::instance();
  
  // 并行计算每个源描述子的候选匹配 / Compute candidate matches for each source descriptor in parallel
  std::vector<std::future<void>> futures;
  futures.reserve(num_src);
  
  for (std::size_t i = 0; i < num_src; ++i) {
    futures.push_back(pool_singleton.submit([this, i, &thread_results]() {
      find_candidates_for_descriptor_serial(i, thread_results[i]);
    }));
  }
  
  // 等待所有任务完成 / Wait for all tasks to complete
  for (auto& future : futures) {
    future.get();
  }
  
  // 收集结果并应用比率测试 / Collect results and apply ratio test
  all_candidates.clear();
  all_candidates.reserve(num_src);
  
  for (std::size_t i = 0; i < num_src; ++i) {
    const auto& candidates = thread_results[i];
    this->m_total_candidates += candidates.size();
    
    if (!candidates.empty() && this->apply_ratio_test(candidates)) {
      this->m_ratio_test_passed++;
      all_candidates.push_back(candidates[0]);  // 只保留最佳匹配 / Keep only best match
    }
  }
}

template<typename DataType, typename Signature>
void brute_force_correspondence_generator_t<DataType, Signature>::perform_mutual_verification(
    const std::vector<correspondence_t>& forward_corrs,
    std::vector<correspondence_t>& verified_corrs)
{
  verified_corrs.clear();
  
  // 构建目标到源的映射 / Build target to source mapping
  std::unordered_map<std::size_t, std::size_t> dst_to_src;
  for (const auto& corr : forward_corrs) {
    dst_to_src[corr.dst_idx] = corr.src_idx;
  }
  
  // 对每个唯一的目标点执行反向搜索 / Perform reverse search for each unique target point
  std::unordered_map<std::size_t, std::pair<std::size_t, float>> reverse_matches;
  
  for (const auto& [dst_idx, src_idx] : dst_to_src) {
    // 找到目标描述子的索引 / Find target descriptor index
    std::size_t dst_desc_idx = 0;
    for (std::size_t i = 0; i < this->m_dst_keypoint_indices->size(); ++i) {
      if ((*this->m_dst_keypoint_indices)[i] == dst_idx) {
        dst_desc_idx = i;
        break;
      }
    }
    
    // 找到最近的源描述子 / Find nearest source descriptor
    float min_dist = std::numeric_limits<float>::max();
    std::size_t best_src_idx = 0;
    
    for (std::size_t i = 0; i < this->m_src_descriptors->size(); ++i) {
      float dist = (*this->m_dst_descriptors)[dst_desc_idx].distance((*this->m_src_descriptors)[i]);
      if (dist < min_dist) {
        min_dist = dist;
        best_src_idx = (*this->m_src_keypoint_indices)[i];
      }
    }
    
    reverse_matches[dst_idx] = {best_src_idx, min_dist};
  }
  
  // 检查双向匹配 / Check bidirectional matches
  for (const auto& forward_corr : forward_corrs) {
    auto it = reverse_matches.find(forward_corr.dst_idx);
    if (it != reverse_matches.end() && it->second.first == forward_corr.src_idx) {
      verified_corrs.push_back(forward_corr);
    }
  }
}

}  // namespace toolbox::pcl