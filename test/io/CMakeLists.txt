set(BASE_TEST_FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/pcd_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kitti_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dataloader_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kitti_dataset_test.cpp
)

list(APPEND TEST_FILES ${BASE_TEST_FILES})
set(TEST_FILES ${TEST_FILES} PARENT_SCOPE)