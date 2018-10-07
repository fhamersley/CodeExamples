// Function forms a vector of candidate points for circle fitting by finding the boundary from black to white from each direction.
// Requires two sequential white pixels in the given direction to form a canditiate point to remove noise.

// Author: Fraser Hamersley for rUNSWift

void BallDetector::getCircleCandidatePoints(const RegionI& region, BallDetectorVisionBundle &bdvb, bool semiCircle) {

    RegionI::iterator_fovea cur_point = region.begin_fovea();
    int rows = region.getRows();
    int cols = region.getCols();

    bool top_cam = bdvb.region->isTopCamera();

    // Reserve max number of circle_fit_points
    bdvb.circle_fit_points.reserve(2*rows + 2*cols);

    std::vector<int> tops(cols, -1);
    std::vector<int> bots(cols, -1);

    for (int y = 0; y < rows; ++y){
        int left = -1;
        int right = -1;

        for (int x = 0; x < cols; ++x){

            if (cur_point.colour() == cWHITE){

                // Left scan
                if ((x < cols - 1) && (left == -1)) {
                    if (cur_point.colourRight() == cWHITE){
                        left = x;
                    }
                }
                // Right scan
                if (x > 0) {
                    if (cur_point.colourLeft() == cWHITE) {
                        right = x;
                    }
                }
                // Top scan
                if ((y < rows - 1) && (tops[x] == -1)) {
                    if (cur_point.colourBelow() == cWHITE){
                        tops[x] = y;
                    }
                }
                // Bottom scan (only run in bottom camera)
                if ((y > 0) && !top_cam) {
                    if (cur_point.colourAbove() == cWHITE) {
                        bots[x] = y;
                    }
                }
            }
            cur_point++;
        }

        // Add left point
        if (left > 0 && left < cols - 1){
            bdvb.circle_fit_points.push_back(Point(left,y));
        }

        // Add right point
        if (right > 0 && right < rows - 1){
            bdvb.circle_fit_points.push_back(Point(right,y));
        }
    }

    // Add top points
    for (int x = 0; x < cols; ++x){
        int val = tops[x];

        if (val > 0 && val < rows - 1){
            bdvb.circle_fit_points.push_back(Point(x, val));
        }
    }

    // Add bottom points (only run in bottom camera)
    if (!top_cam){
        for (int x = 0; x < cols; ++x){
            int val = bots[x];

            if (val > 0 && val < rows - 1){
                bdvb.circle_fit_points.push_back(Point(x, val));
            }
        }
    }

#ifdef BALL_DEBUG
    std::cout << "GetCircleCandidatePoints: " << bdvb.circle_fit_points.size() << "\n";
#endif // BALL_DEBUG
}