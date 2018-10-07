// Functions for finding appropriately sized equilateral triangles between the black patches found on the ball.

// Author: Fraser Hamersley for rUNSWift

void BallDetector::findRegionTriangles(BallDetectorVisionBundle &bdvb, InternalRegionFeatures &internal_regions, RegionTriangleFeatures &region_triangle_features)
{
    /*
     * --- Finding equilateral triangles in ball regions ---
     * A key region is a fully internal region that is closest to the centre of the circle.
     * Equilateral triangles can only be formed including this key region as a vertice point
     * When constructing all posible combos, vertice sets not including this key region centre will thrown out
     * These combo sets are then passed to a function to check if it is equilateral within a defined error threshold
     */

    // Number of regions
    int region_num = int(internal_regions.groups.size());

    // Internal data structures
    Triangle tri;
    RegionTriangleFeatures tri_features;
    std::vector<Point> region_centres;
    region_centres.reserve(region_num);

    // Circle features
    int cir_x = bdvb.circle_fit.result_circle.centre.x();
    int cir_y = bdvb.circle_fit.result_circle.centre.y();
    int cir_rad = bdvb.circle_fit.result_circle.radius;

    // Key Region
    int key_region_dist = cir_rad;
    int key_region_num = 0;

    // Find the centre of each region
    for (int i = 0; i < region_num; i++) {
        Point centre(((internal_regions.groups[i].max_x - internal_regions.groups[i].min_x) / 2) + internal_regions.groups[i].min_x,
                     ((internal_regions.groups[i].max_y - internal_regions.groups[i].min_y) / 2) + internal_regions.groups[i].min_y);
        // Finding the key region
        if (internal_regions.groups[i].completely_internal) {
            int x_d2 = pow(centre.x() - cir_x, 2);
            int y_d2 = pow(centre.y() - cir_y, 2);
            int dist = sqrt(x_d2 + y_d2);
            if (dist < key_region_dist) {
                key_region_dist = dist;
                key_region_num = i;
            }
        }
        region_centres.push_back(centre);       // Store the centre
    }

#ifdef BALL_DEBUG
    std::cout << "Key region number: " << key_region_num << "\nKey region -" <<
                 " x: " << region_centres[key_region_num].x() <<
                 " y: " << region_centres[key_region_num].y() << "\n";
#endif //BALL_DEBUG

    // For Vatnao
    tri_features.region_centres = region_centres;

    // Clear from memory
    combo_.clear();
    tri_combos_.clear();

    // Reserve memory
    combo_.reserve(4);

    /// Form combinations and return suitable (including key region) combos in member data tri_combos
    int k = 3;                          // Triangles have 3 vertices -- Would you like to know more?
    key_reg_combo_ = false;              // Initalise outside of function
    combinations(0, k, region_centres, key_region_num);

    // Test each combo for equilateral-ness
    for (std::vector<Triangle>::iterator it = tri_combos_.begin(); it != tri_combos_.end(); it++) {
        // If equilateral store its vertices in local struct
        if (equilateralTriangle(bdvb, it->vertices)) {
            tri.vertices = it->vertices;
            tri_features.region_triangles.push_back(tri);
        }
    }
#ifdef BALL_DEBUG
    std::cout << "Triangles found: " << tri_features.region_triangles.size() << "\n";
#endif //BALL_DEBUG

    // Store all data in bdvb structs
    region_triangle_features = tri_features;
}

bool BallDetector::equilateralTriangle(BallDetectorVisionBundle &bdvb, std::vector<Point> triangle_points) {

    int radius2 = bdvb.circle_fit.result_circle.radius * bdvb.circle_fit.result_circle.radius;
    int dist_min = radius2 - radius2 * TRIANGLE_FIT_DIST_RAD_RATIO_MIN;
    int dist_max = radius2 + radius2 * TRIANGLE_FIT_DIST_RAD_RATIO_MAX;

    std::vector<int> distance;
    distance.reserve(3);

    // Find the distance between each region centre with pythagorean theorem
    std::vector<Point>::iterator next_it = triangle_points.begin() + 1;      // Next iterator point

    for (std::vector<Point>::iterator it = triangle_points.begin(); it != triangle_points.end(); it++, next_it++) {
        if (next_it == triangle_points.end()) {  // If the next point is the end
            next_it = triangle_points.begin();   // Reset next point to the beginning
        }
        int x_d2 = pow((*it).x() - (*next_it).x(), 2);
        int y_d2 = pow((*it).y() - (*next_it).y(), 2);
        int dist2 = x_d2 + y_d2;
        if (dist2 < dist_min|| dist2 > dist_max) {

#ifdef BALL_DEBUG
            std::cout << "Dist between reg failed - Dist: " << dist2 << " Dist Min: " << dist_min << " Dist Max: " << dist_max << "\n";
#endif //BALL_DEBUG

            return false;   //Throw out lines that are too long to form the kind of equilateral triangle we are looking for ,m
        }
        distance.push_back(dist2);
    }
    int avg_error = (abs(distance[0] - distance[1]) + abs(distance[1] - distance[2]) + abs(distance[2] - distance[0])) / 3;
    if (avg_error < TRIANGLE_FIT_ERROR_THRESHOLD) {

#ifdef BALL_DEBUG
        std::cout << "Equilateral triangle found\n";
        std::cout << "Vertice 1: " << triangle_points[0].x() << ", " << triangle_points[0].y() << "\n"
                  << "Vertice 2: " << triangle_points[1].x() << ", " << triangle_points[1].y() << "\n"
                  << "Vertice 3: " << triangle_points[2].x() << ", " << triangle_points[2].y() << "\n";
#endif //BALL_DEBUG

        return true;
    }
    else {

#ifdef BALL_DEBUG
        std::cout << "Not Equilateral triangle. Avg error: " << avg_error << "\n";
        std::cout << "Vertice 1: " << triangle_points[0].x() << ", " << triangle_points[0].y() << "\n"
                  << "Vertice 2: " << triangle_points[1].x() << ", " << triangle_points[1].y() << "\n"
                  << "Vertice 3: " << triangle_points[2].x() << ", " << triangle_points[3].y() << "\n";
#endif //BALL_DEBUG
        return false;
    }
}

void BallDetector::combinations(int offset, int k, std::vector <Point> region_centres, int key_region_num) {
    // Create a list of possible combinations of vertice points that include the key region vertice
    Triangle tri;
    int k_x = region_centres[key_region_num].x();       // Key region centre x
    int k_y = region_centres[key_region_num].y();       // Key region centre y

    if (k == 0) {
        // Check if combo includes key region
        for (std::vector<Point>::iterator it = combo_.begin(); it != combo_.end(); it++) {
            if (((it->x() - k_x) + (it->y() - k_y)) == 0) {
                key_reg_combo_ = true;
            }
        }
        // Save combo if includes key region
        if (key_reg_combo_) {
            tri.vertices = combo_;
            tri_combos_.push_back(tri);
            key_reg_combo_ = false;      // Reset
            return;
        }
        // Throw out combos not including the key region
        else {
            return;
        }
    }
    for (int i = offset; i <= int(region_centres.size()) - k; ++i) {
        combo_.push_back(region_centres[i]);
        combinations(i+1, k-1, region_centres, key_region_num);
        combo_.pop_back();
    }
}
