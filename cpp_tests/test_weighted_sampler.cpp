#include<gtest/gtest.h>
#include <iostream>
#include <vector>
#include "../Weighted_Sampler.h"
#include "../helpers.h"

TEST(testSampler, vector_normalization){
  double tol = 0.01;
  // Setup generator
  Weighted_Sampler my_sampler;
  
  // Sample vector
  std::vector<double> vec_raw = {1.0, 2.0, 3.0, 4.0};
  
  // Normalize vector
  std::vector<double> vec_norm = normalize_vector(vec_raw);
  
  EXPECT_EQ(vec_norm.size(), vec_raw.size());
  ASSERT_NEAR(vec_norm[0], 0.1, tol);
  ASSERT_NEAR(vec_norm[1], 0.2, tol);
  ASSERT_NEAR(vec_norm[2], 0.3, tol);
  ASSERT_NEAR(vec_norm[3], 0.4, tol);
}

TEST(testSampler, basic){
  // Setup generator
  Weighted_Sampler my_sampler;

  double drawn_value = my_sampler.draw_unif();

  EXPECT_TRUE((drawn_value < 1.0) & (drawn_value > 0.0));
}

TEST(testSampler, set_seeds){
  
  // Setup multuple generators with same seed
  Weighted_Sampler sampler_1(42);
  Weighted_Sampler sampler_2(42);
  
  // The two samplers should give back the same result
  EXPECT_TRUE(sampler_1.draw_unif() == sampler_2.draw_unif());
}

TEST(testSampler, lots_of_samples){
  
  Weighted_Sampler my_sampler;
  
  int num_samples = 1000;
  double min_draw = 100;
  double max_draw = -1;

  // Draw and add a bunch of values to a vector
  for (int i = 0; i < num_samples; ++i)
  {
    double current_draw = my_sampler.draw_unif();
    
    if (current_draw < min_draw) min_draw = current_draw;
    if (current_draw > max_draw) max_draw = current_draw;
  }
  
  EXPECT_TRUE(min_draw > 0.0);
  EXPECT_TRUE(max_draw < 1.0);
}

TEST(testSampler, drawing_from_weights){

  Weighted_Sampler my_sampler;

  // Setup some weights
  std::vector<double> weights {0.1, 0.4, 0.3, 0.2};

  int chosen_index = my_sampler.sample(weights);

  // Hopefully chosen index is within range...
  EXPECT_TRUE(chosen_index < 4 & chosen_index > 0);

  int num_samples = 10000;
  int times_el_3_chosen = 0;
    // Sample index from function a bunch of times and record
  for (int i = 0; i < num_samples; ++i)
  {
    // Sample from weights and record if we chose the third element
    if (my_sampler.sample(weights) == 2) times_el_3_chosen++;
  }
   
  double prop_of_el_3 = double(times_el_3_chosen)/double(num_samples);

  // Make sure that the element was chosen roughly as much as it should have
  // been
  ASSERT_NEAR(
    prop_of_el_3,
    weights[2],
    0.01
  );
}

int main(int argc, char* argv[]){
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}