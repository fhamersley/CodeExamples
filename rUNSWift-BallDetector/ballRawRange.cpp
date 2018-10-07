// Function for calculating the range of the Y values in the given raw image (YUV422 format)

// Author: Fraser Hamersley for rUNSWift

void BallDetector::ballRawRange(BallDetectorVisionBundle &bdvb) {
    
        RegionI::iterator_raw cur_point = bdvb.region->begin_raw();
    
        // Track the literal location of the iterators.
        int x = 0;
        int y = 0;
    
        // Min and max raw Y values
        uint8_t minY = 255;
        uint8_t maxY = 0;
    
        // Current raw value
        uint8_t currY = 0;
    
        // The number of rows and columns in the region.
        int rows = bdvb.region->getRows();
        int cols = bdvb.region->getCols();
    
        // Circle centre
        float circ_x = bdvb.circle_fit.result_circle.centre.x();
        float circ_y = bdvb.circle_fit.result_circle.centre.y();
        float circ_r2 = bdvb.circle_fit.result_circle.radius * bdvb.circle_fit.result_circle.radius;
    
        // Loop
        for(int pixel = 0; pixel < cols*rows; ++pixel)
        {
            if (DISTANCE_SQR((float)x, (float)y, circ_x, circ_y) < circ_r2) {       // If inside circle
                currY = *cur_point.raw();
                minY = std::min(minY, currY);
                maxY = std::max(maxY, currY);
            }
            ++cur_point;
            ++x;
            if(x == cols)
            {
                x = 0;
                ++y;
            }
        }
    
        // and store
        bdvb.max_y_value = int(maxY);
        bdvb.min_y_value = int(minY);
    
    #ifdef BALL_DEBUG
        std::cout << "Max ball Y: "  << bdvb.max_y_value << " Min ball Y: " << bdvb.min_y_value
                  << "\nBall Raw Variance: " << (bdvb.max_y_value - bdvb.min_y_value) << "\n";
    #endif // BALL_DEBUG
}