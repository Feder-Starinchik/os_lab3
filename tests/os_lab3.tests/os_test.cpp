#include <gtest/gtest.h>
#include <vector>
#include <algorithm>

void SimulateMarker(int id, int n, std::vector<int>& arr, int& markedCount) {
    srand(id);
    markedCount = 0;
    for (int i = 0; i < 1000; ++i) { 
        int r = rand();
        int idx = r % n;
        if (arr[idx] == 0) {
            arr[idx] = id;
            markedCount++;
        }
        else {
            break; 
        }
    }
}

void ClearMarkerMarks(int id, std::vector<int>& arr) {
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] == id) arr[i] = 0;
    }
}

TEST(MarkerLogic, ArrayInitialization) {
    const int n = 10;
    std::vector<int> arr(n, 0);
    EXPECT_TRUE(std::all_of(arr.begin(), arr.end(), [](int x) { return x == 0; }));
}

TEST(MarkerLogic, SmallArrayManyMarkers) {
    const int n = 3;
    const int k = 10; 
    std::vector<int> arr(n, 0);

    int totalMarked = 0;
    for (int id = 1; id <= k; ++id) {
        int markedCount = 0;
        SimulateMarker(id, n, arr, markedCount);
        totalMarked += markedCount;
    }

    
    EXPECT_LE(totalMarked, n);
  
    EXPECT_LT(totalMarked, k);
}

TEST(MarkerLogic, ClearMarks) {
    const int n = 5;
    std::vector<int> arr(n, 0);
    int id = 7;
    int markedCount = 0;

    SimulateMarker(id, n, arr, markedCount);
    EXPECT_GT(markedCount, 0);

    ClearMarkerMarks(id, arr);
    EXPECT_TRUE(std::all_of(arr.begin(), arr.end(), [](int x) { return x == 0; }));
}
